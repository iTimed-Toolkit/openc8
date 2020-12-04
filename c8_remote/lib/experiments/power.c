#include "util/experiments.h"
#include "tool/payload.h"
#include "exp_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>
#include <semaphore.h>

#define PORTNO          6636
#define TIMEOUT_SEC     10
#define PAYLOAD_TO_TEST PAYLOAD_AES_HW_BUILTIN

struct pwned_device *setup()
{
    struct pwned_device *dev = exploit_device(true);
    if(dev == NULL || dev->status == DEV_NORMAL)
        return NULL;

    if(payload_is_installed(dev, PAYLOAD_TO_TEST))
        uninstall_payload(dev, PAYLOAD_TO_TEST);

    install_payload(dev, PAYLOAD_TO_TEST);
    return dev;
}

struct hardware_aes_arg
{
    int cli_num;
    int cli_sockfd;
    struct pwned_device *dev;
    sem_t *lock;
};

void *hardware_aes_thread(void *arg)
{
    struct thread_tracker *tracker = (struct thread_tracker *) arg;
    struct hardware_aes_arg *params = get_arg(tracker);

    int i, read_len, write_len, recv, sent;
    long count;

    uint8_t res[16];
    struct hw_aes_args dev_args = {
            .dir = DIR_ENC,
            .mode = MODE_ECB,
            .type = KEY_USER,
            .size = SIZE_128,

            .msg = {0},
            .key = {0x00, 0x11, 0x22, 0x33,
                    0x44, 0x55, 0x66, 0x77,
                    0x88, 0x99, 0xAA, 0xBB,
                    0xCC, 0xDD, 0xEE, 0xFF}
    };

    count = 0;
    while(1)
    {
        read_len = 0;
        while(read_len < 16)
        {
            recv = read(params->cli_sockfd, &dev_args.msg[read_len], 16 - read_len);
            if(recv < 0)
            {
                printf("%i.%li socket read error at byte %i\n", params->cli_num, count, read_len);
                goto done;
            }

            read_len += recv;
        }

        printf("%i.%li > ", params->cli_num, count);
        for(i = 0; i < 16; i++)
            printf("%02X", dev_args.msg[i]);
        printf("\n");

        sem_wait(params->lock);
        execute_payload(params->dev, PAYLOAD_TO_TEST,
                        &dev_args, sizeof(struct hw_aes_args),
                        res, 16);
        sem_post(params->lock);

        printf("%i.%li < ", params->cli_num, count);
        for(i = 0; i < 16; i++)
            printf("%02X", res[i]);
        printf("\n");

        write_len = 0;
        while(write_len < 16)
        {
            sent = write(params->cli_sockfd, &res[write_len], 16 - write_len);
            if(sent < 0)
            {
                printf("%i.%li socket write error at byte %i\n", params->cli_num, count, write_len);
                goto done;
            }

            write_len += sent;
        }

        count++;
    }

done:
    printf("%i thread exiting\n", params->cli_num);

    close(params->cli_sockfd);
    free(params);
    set_finished(tracker);
    pthread_exit(NULL);
}

int main_hardware_aes()
{
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval timeout = {
            .tv_sec = TIMEOUT_SEC,
            .tv_usec = 0
    };

    int serv_sockfd, cli_sockfd, ret;
    unsigned int cli_len, num_clients;

    struct thread_tracker *tracker;
    struct hardware_aes_arg *arg;
    sem_t device_lock;
    if(sem_init(&device_lock, 0, 1) < 0)
    {
        printf("failed to init semaphore\n");
        return -1;
    }

    struct pwned_device *dev = setup();
    if(dev == NULL)
    {
        printf("failed to exploit device\n");
        return -1;
    }

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sockfd < 0)
    {
        printf("failed to open server socket\n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORTNO);

    ret = bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(ret < 0)
    {
        printf("failed to bind socket to port\n");
        return 0;
    }

    cli_len = sizeof(cli_addr);
    listen(serv_sockfd, 1);

    num_clients = 0;
    while(1)
    {
        printf("listening for a client\n");
        cli_sockfd = accept(serv_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if(cli_sockfd < 0)
        {
            printf("failed to accept a client\n");
            return 0;
        }

        if(setsockopt(cli_sockfd, SOL_SOCKET, SO_RCVTIMEO, (uint8_t *) &timeout, sizeof(struct timeval)) < 0)
        {
            printf("failed to set recv timeout\n");
            return 0;
        }

        if(setsockopt(cli_sockfd, SOL_SOCKET, SO_SNDTIMEO, (uint8_t *) &timeout, sizeof(struct timeval)) < 0)
        {
            printf("failed to set send timeout\n");
            return 0;
        }

        arg = malloc(sizeof(struct hardware_aes_arg));
        arg->cli_num = num_clients++;
        arg->cli_sockfd = cli_sockfd;
        arg->dev = dev;
        arg->lock = &device_lock;

        create_and_save(&tracker, hardware_aes_thread, arg);
    }
}
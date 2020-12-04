#include "exp_util.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct thread_tracker
{
    pthread_t handle;
    void *arg;
    enum
    {
        THREAD_RUNNING,
        THREAD_EXITED
    } state;

    struct thread_tracker *prev;
    struct thread_tracker *next;
};

struct thread_tracker *create_and_save(struct thread_tracker **head, void *(*func)(void *), void *arg)
{
    struct thread_tracker *curr, *temp;
    if(*head != NULL && (*head)->state != THREAD_RUNNING)
    {
        free(*head);
        *head = NULL;
    }

    if(*head == NULL)
    {
        *head = malloc(sizeof(struct thread_tracker));
        (*head)->prev = NULL;
        (*head)->next = NULL;

        curr = *head;
    }
    else
    {
        // clean the list
        for(curr = *head; curr->next != NULL; curr = curr->next)
        {
            // first entry is guaranteed to be running here
            if(curr->state != THREAD_RUNNING)
            {
                temp = curr->next;
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                free(curr);

                // correct next will happen in next iteration
                curr = temp->prev;
                continue;
            }
        }

        curr->next = malloc(sizeof(struct thread_tracker));
        curr->next->prev = curr;
        curr->next->next = NULL;

        curr = curr->next;
    }

    curr->arg = arg;
    curr->state = THREAD_RUNNING;

    if(pthread_create(&curr->handle, NULL, func, curr))
    {
        printf("failed to create new pthread\n");
        return NULL;
    }
    else
        return curr;
}

void *get_arg(struct thread_tracker *tracker)
{
    return tracker->arg;
}

void set_finished(struct thread_tracker *tracker)
{
    tracker->state = THREAD_EXITED;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include <mpich/mpi.h>
#include <omp.h>
#include "host_crypto.h"

#define N_FILES         48
#define N_NODES         8
#define FILE_PER_NODE   (N_FILES / N_NODES)
#define MSG_SEPARATE    1024 * 256

int read_data(unsigned char *dst, char *fname, unsigned int offset, unsigned int num)
{
    unsigned long ret;
    FILE *datafile = fopen(fname, "rb");

    if(datafile == NULL)
    {
        printf("failed to open datafile %s\n", fname);
        return -1;
    }

    ret = fread(&dst[offset], 1, num, datafile);
    if(ret != num)
    {
        printf("reading %s failed with ferror %i, feof %i\n",
               fname, ferror(datafile), feof(datafile));
        return -1;
    }

    fclose(datafile);
    return 0;
}

struct summary_stats
{
    double mean;
    double stddev;
};

struct summary_stats *calculate_stats(unsigned char *data,
                                      unsigned int len, int mul, int offset,
                                      int rank, int nodes)
{
    int i;
    double mean = 0, stddev = 0, temp;

    struct summary_stats *res;
    MPI_Status status;

    /*
     * First calculate the mean
     */

#pragma omp parallel for num_threads(32) default(none)  \
            firstprivate(len, mul, offset)              \
            shared(data)                                \
            reduction(+:mean)
    for(i = 0; i < len; i++)
        mean += (double) data[mul * i + offset];

    if(rank == 0)
    {
        for(i = 1; i < nodes; i++)
        {
            MPI_Recv(&temp, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            mean += temp;
        }

        mean /= (len * nodes);
        for(i = 1; i < nodes; i++)
            MPI_Send(&mean, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Sendrecv_replace(&mean, 1, MPI_DOUBLE,
                             0, 0, 0, 0,
                             MPI_COMM_WORLD, &status);
    }

    /*
     * Then the standard deviation
     */

#pragma omp parallel for num_threads(32) default(none)  \
            firstprivate(len, mul, offset, mean)        \
            shared(data)                                \
            reduction(+:stddev)
    for(i = 0; i < len; i++)
        stddev += pow(data[mul * i + offset] - mean, 2);

    if(rank == 0)
    {
        for(i = 1; i < nodes; i++)
        {
            MPI_Recv(&temp, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            stddev += temp;
        }

        stddev = sqrt(stddev / (len * nodes));
        for(i = 1; i < nodes; i++)
            MPI_Send(&stddev, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
    }
    else
    {
        PMPI_Sendrecv_replace(&stddev, 1, MPI_DOUBLE,
                              0, 0, 0, 0,
                              MPI_COMM_WORLD, &status);
    }

    res = malloc(sizeof(struct summary_stats));
    res->mean = mean;
    res->stddev = stddev;
    return res;
}

int main(int argc, char *argv[])
{
    int i, j, res;
    unsigned int i_byte, i_input, i_key, i_key_split;
    unsigned int trace_per_file = 0, msg_per_file = 0, num_traces = 0;
    int rank, nodes;

    char timing_name[256], msg_name[256];
    struct stat timing_finfo, msg_finfo;

    FILE *keyfile;
    struct aes_constants *c;
    unsigned char key[16], key_sched[176], msg_new[16], key_hyp;

    double cov, pearson, temp;
    struct summary_stats *timing_stats, *model_stats;
    unsigned char *msg = NULL, *timings = NULL, *model = NULL;

    MPI_Status status;

    if(argc != 2)
    {
        printf("usage: analyze [data dir]\n");
        return -1;
    }

    /*
     * First, read in the data from each file
     */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nodes);

    sprintf(timing_name, "%s/timing_dat00_%i.dat", argv[1], rank);
    sprintf(msg_name, "%s/msg_dat00_%i.dat", argv[1], rank);

    if(stat(timing_name, &timing_finfo) != 0)
    {
        printf("failed to stat %s\n", timing_name);
        return -1;
    }

    if(stat(msg_name, &msg_finfo) != 0)
    {
        printf("failed to stat %s\n", msg_name);
        return -1;
    }

    trace_per_file = timing_finfo.st_size;
    msg_per_file = msg_finfo.st_size / 16;
    num_traces = trace_per_file * FILE_PER_NODE;

    // allocate memory (big!)
    model = malloc(64 * num_traces);
    msg = malloc(16 * num_traces);
    timings = malloc(num_traces);

    sprintf(timing_name, "%s/KEY", argv[1]);
    keyfile = fopen(timing_name, "r");
    if(keyfile == NULL)
    {
        printf("failed to open key file\n");
        return -1;
    }

    for(i = 0; i < 16; i++)
    {
        fread(key_sched, 1, 2, keyfile);
        key_sched[2] = 0;
        key[i] = (unsigned char) strtol((char *) key_sched, NULL, 16);
    }

    fclose(keyfile);

    c = get_constants();
    expand_key(key, key_sched, 11, c);

    for(i = 0; i < FILE_PER_NODE; i++)
    {
        sprintf(timing_name, "%s/timing_dat00_%i.dat", argv[1], rank * FILE_PER_NODE + i);
        sprintf(msg_name, "%s/msg_dat00_%i.dat", argv[1], rank * FILE_PER_NODE + i);

        read_data(timings, timing_name, trace_per_file * (i % FILE_PER_NODE), trace_per_file);
        read_data(msg, msg_name, msg_per_file * (i % FILE_PER_NODE), msg_per_file);
    }

    /*
     * Then expand the messages so that we can create power models
     */

    res = 0;

#pragma omp parallel for num_threads(32) default(none)  \
            firstprivate(key_sched, msg_per_file)       \
            private(msg_new, j)                         \
            shared(msg, c)                              \
            reduction(max:res)
    for(i = 0; i < FILE_PER_NODE * msg_per_file; i++)
    {
        memcpy(&msg[i * MSG_SEPARATE], &msg[i], 16);
        memcpy(msg_new, &msg[i * MSG_SEPARATE], 16);

        for(j = 0; j < MSG_SEPARATE - 1; j++)
        {
            aes128_encrypt_ecb(msg_new, 16, key_sched, c);
            memcpy(&msg[i * MSG_SEPARATE + j + 1], msg_new, 16);
        }

        aes128_encrypt_ecb(msg_new, 16, key_sched, c);
        for(j = 0; j < 16; j++)
        {
            if(msg_new[j] != msg[(i + 1) * MSG_SEPARATE - 16 + j])
            {
                res = 1;
                break;
            }
        }
    }

    if(res)
    {
        printf("aes expansion failed for some thread\n");
        return -1;
    }

    /*
     * Start iterating through the byte positions
     */

    timing_stats = calculate_stats(timings, num_traces, 1, 0, rank, nodes);
    for(i_byte = 0; i_byte < 16; i_byte++)
    {
        for(i_key_split = 0; i_key_split < 4; i_key_split++)
        {
#pragma omp parallel for num_threads(32) default(none)      \
            firstprivate(i_key_split, i_byte, num_traces)   \
            private(key_hyp, i_input)                       \
            shared(model, msg)                              \

            for(i_key = 0; i_key < 64; i_key++)
            {
                key_hyp = 4 * i_key_split + i_key;
                for(i_input = 0; i_input < num_traces; i_input++)
                {
                    //TODO: power model if this doesn't work
                    model[i_key * num_traces + i_input] = (msg[i_input * 16 + i_byte] ^ key_hyp) % 64;
                }
            }

            for(i_key = 0; i_key < 64; i_key++)
            {
                model_stats = calculate_stats(model, num_traces, 1, i_key * num_traces, rank, nodes);
                cov = 0;

#pragma omp parallel for num_threads(32) default(none)          \
            firstprivate(num_traces, i_key)                     \
            shared(model, model_stats, timings, timing_stats)   \
            reduction(+:cov)

                for(i_input = 0; i_input < num_traces; i_input++)
                {
                    cov += (model[i_key * num_traces + i_input] - model_stats->mean) *
                           (timings[i_input] - timing_stats->mean);
                }

                if(rank == 0)
                {
                    for(i = 1; i < nodes; i++)
                    {
                        MPI_Recv(&temp, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
                        cov += temp;
                    }

                    cov /= (num_traces * nodes);
                    pearson = cov / (model_stats->stddev * timing_stats->stddev);

                    printf("%i\t%i\t%i\t%f\n", i_byte, i_key_split, i_key, pearson);
                }
                else
                {
                    MPI_Send(&cov, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    free(timing_stats);
}
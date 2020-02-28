#include <stdio.h>
#include <string.h>
#include <libgen.h>

struct entry
{
    unsigned char msg[16];
    unsigned char pad0;

    unsigned char timing;
    unsigned char pad1[2];
} __attribute__ ((packed));

int main(int argc, char *argv[])
{
    FILE *infile, *timingfile, *msgfile;
    char fname[128], c1[128], c2[128], *path, *name;

    strcpy(c1, argv[1]);
    strcpy(c2, argv[1]);

    int count = 0;
    unsigned long read;
    struct entry e;

    if(argc != 2)
    {
        printf("usage: strip [fname]\n");
        return -1;
    }

    path = dirname(c1);
    name = basename(c2);

    infile = fopen(argv[1], "rb");
    if(infile == NULL)
    {
        printf("failed to open file %s\n", argv[1]);
        return -1;
    }

    sprintf(fname, "%s/timing_%s", path, name);
    timingfile = fopen(fname, "wb");
    if(timingfile == NULL)
    {
        printf("failed to open timing output\n");
        return -1;
    }

    sprintf(fname, "%s/msg_%s", path, name);
    msgfile = fopen(fname, "wb");
    if(msgfile == NULL)
    {
        printf("failed to open message output\n");
        return -1;
    }

    while(!(ferror(infile) || feof(infile)))
    {
        read = fread(&e, sizeof(struct entry), 1, infile);
        if(read != 1)
            break;

        fwrite(&e.timing, 1, 1, timingfile);
        if(count % (1024 * 256) == 0)
        {
            fwrite(&e.msg, 16, 1, msgfile);
            printf("stripped %i entries\n", count);
        }

        count++;
    }

    printf("strip finished with ferror %i feof %i\n", ferror(infile), feof(infile));

    fflush(timingfile);
    fflush(msgfile);

    fclose(infile);
    fclose(timingfile);
    fclose(msgfile);
    return 0;
}
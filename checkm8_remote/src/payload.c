#include "../include/payload.h"

#include <elf.h>
#include <stdio.h>

void read_elf_header(const char *filename)
{
    Elf64_Ehdr header;
    FILE *file = fopen(filename, "rb");
    if(file)
    {
        fread(&header, 1, sizeof(header), file);
    }
}
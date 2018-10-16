#ifndef _ELFFILE_H_
#define _ELFFILE_H_
#include <elf.h>

typedef struct elffile {
    size_t size;
    char *buffer;
    Elf32_Ehdr *hdr;
    Elf32_Phdr *phdrs;
} elffile_t;

elffile_t *elffile_open(char *path);
char *elffile_locate(elffile_t *ef, uintptr_t address);
void elffile_write(elffile_t *ef, char *path);
void elffile_memcpy(elffile_t *ef, uintptr_t address, char *from, size_t size);

#endif

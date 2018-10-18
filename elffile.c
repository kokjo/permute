#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "elffile.h"

elffile_t *elffile_open(char *path){
    int fd = open(path, 0);
    struct stat stat;
    fstat(fd, &stat);
    elffile_t *ef = malloc(sizeof(elffile_t));
    ef->size = stat.st_size;
    ef->buffer = malloc(ef->size);
    read(fd, ef->buffer, ef->size);
    close(fd);
    ef->hdr = (void *)ef->buffer;
    ef->phdrs = (void *)&ef->buffer[ef->hdr->e_phoff];
    return ef;
}

char *elffile_locate(elffile_t *ef, uintptr_t address){
    for(int i = 0; i < ef->hdr->e_phnum; i++){
        if(ef->phdrs[i].p_type == PT_LOAD){
            if(ef->phdrs[i].p_vaddr <= address &&
                address < ef->phdrs[i].p_vaddr + ef->phdrs[i].p_filesz){
                size_t offset = ef->phdrs[i].p_offset + (address - ef->phdrs[i].p_vaddr);
                return &ef->buffer[offset];
            }
        }
    }
    return NULL;
}

void elffile_memcpy(elffile_t *ef, uintptr_t address, char *from, size_t size){
    char *buffer = elffile_locate(ef, address);
    memcpy(buffer, from, size);
}

void elffile_write(elffile_t *ef, char *path){
    uint32_t max_offset = 0xffffffff;
    for(int i = 0; i < ef->hdr->e_phnum; i++){
        if(ef->phdrs[i].p_type == PT_LOAD){
            max_offset = ef->phdrs[i].p_offset + ef->phdrs[i].p_filesz;
        }
    }
    ef->size = max_offset;
    ef->hdr->e_shnum = 0;
    ef->hdr->e_shoff = 0;
    ef->hdr->e_shstrndx = 0;
    

    int fd = open("tmp", O_CREAT | O_TRUNC | O_RDWR, 0777);
    write(fd, ef->buffer, ef->size);
    rename("tmp", path);
}

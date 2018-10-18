#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "vec.h"
#include "bytevec.h"
#include "set.h"
#include "dict.h"
#include "util.h"
#include "disassembler.h"
#include "reassembler.h"
#include "elffile.h"
#include "bitvec.h"
#include "codec.h"

#define FUNC ((uintptr_t)&main)
int iteration_count = -1;

bitvec_t *(*setup_func)(bitvec_t *, int argc, char **argv);
bitvec_t *crackme(bitvec_t *old_bv, int argc, char **argv);

int main(int argc, char **argv) {
    vec_t *basicblocks = find_all_basicblocks(FUNC);
    bitvec_t *old_bv = decode_perm(basicblocks);

#ifdef DEBUG
    printf("Permutation data length: %d bits\n", old_bv->used);
    printf("Permutation data: ");
    bitvec_print(old_bv);
#endif

    bitvec_t *new_bv;

    if(setup_func){
        new_bv = setup_func(old_bv, argc, argv);
    } else {
        new_bv = crackme(old_bv, argc, argv);
    }

#ifdef DEBUG
    printf("New permutation data: ");
    bitvec_print(new_bv);
#endif

    encode_perm(basicblocks, new_bv);

    bytevec_t *code = reassemble_basicblocks(FUNC, basicblocks);
    elffile_t *ef = elffile_open(argv[0]);
    elffile_memcpy(ef, FUNC, code->bytes, code->used);
    elffile_memcpy(ef, (uintptr_t)&iteration_count, (void*)&iteration_count, sizeof(int));
    elffile_memcpy(ef, (uintptr_t)&setup_func, (void*)&setup_func, sizeof(void *));
    elffile_write(ef, argv[0]);

#ifdef DEBUG
    printf("Iteration count: %d\n", iteration_count);
    printf("Number of basicblocks: %d\n", vec_length(basicblocks));
    printf("New code size: %d\n", code->used);
#endif

    return 0;
}

void _exit(int);

bitvec_t *setup(bitvec_t *old_bv, int argc, char **argv){
    if(argc != 2){
        printf("Usage: %s <flag>\n", argv[0]);
        _exit(0);
    }
    printf("First execution, setting up!\n");
    setup_func = NULL;
    iteration_count = 0;
    bitvec_t *bv = bitvec_new(0);
    char *flag = argv[1];
    bitvec_write_int(bv, 8, strlen(flag));
    while(*flag) bitvec_write_int(bv, 8, *flag++);
    bitvec_write_int(bv, 8, 0);
    while(bv->used < old_bv->used){
        bitvec_push(bv, random() & 1);
    }
    return bv;
}

bitvec_t *(*setup_func)(bitvec_t *, int argc, char **argv) = &setup;

/*
bitvec_t *crackme(bitvec_t *old_bv, int argc, char **argv){
    return old_bv;
}
*/

bitvec_t *crackme(bitvec_t *old_bv, int argc, char **argv){
    bitvec_t *bv = bitvec_new(old_bv->used);
    srand(iteration_count++);
    if(argc != 2 || strlen(argv[1]) != 1){
        printf("Usage: %s <flag byte>", argv[0]);
        return bv;
    }
    bitvec_reader_t *r = bitvec_reader_new(old_bv);
    uint8_t length = bitvec_reader_int(r, 8);
    if(length == 0){
        printf(bitvec_reader_int(r, 8) == 0 ? "WIN!\n" : "LOSE!\n");
        return old_bv;
    }
    bitvec_write_int(bv, 8, --length);
    uint8_t next_char = bitvec_reader_int(r, 8);
    uint8_t chksum = next_char ^ argv[1][0];
    while(length--){
        next_char = bitvec_reader_int(r, 8);
        bitvec_write_int(bv, 8, next_char);
    }
    chksum |= bitvec_reader_int(r, 8);
    bitvec_write_int(bv, 8, chksum);
    while(bv->used < old_bv->used){
        bitvec_push(bv, random() & 1);
    }
    return bv;
}

char __attribute__((section(".text"))) zeroes[4096] = {0};

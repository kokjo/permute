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

#define FUNC ((uintptr_t)&main)
int iteration_count = -1;
void (*setup_func)(char *, size_t);

bitvec_t *decode_perm(vec_t *bbs);
void encode_perm(vec_t *bbs, bitvec_t *bv);

int main(int argc, char **argv) {
    vec_t *basicblocks = find_all_basicblocks(FUNC);
    bitvec_t *old_bv = decode_perm(basicblocks);
    printf("Permutation data length: %d bits\n", old_bv->used);
    printf("Permutation data: ");
    bitvec_print(old_bv);

    srand(iteration_count++);
    bitvec_t *new_bv = bitvec_new(old_bv->used);
    for(int i = 0; i < old_bv->used; i++){
        bitvec_push(new_bv, random() & 1);
    }

    printf("New permutation data: ");
    bitvec_print(new_bv);

    encode_perm(basicblocks, new_bv);
    
    if(setup_func) setup_func(NULL, 0);

    bytevec_t *code = reassemble_basicblocks(FUNC, basicblocks);
    elffile_t *ef = elffile_open(argv[0]);
    elffile_memcpy(ef, FUNC, code->bytes, code->used);
    elffile_memcpy(ef, (uintptr_t)&iteration_count, (void*)&iteration_count, sizeof(int));
    elffile_memcpy(ef, (uintptr_t)&setup_func, (void*)&setup_func, sizeof(void *));
    elffile_write(ef, argv[0]);

    printf("Iteration count: %d\n", iteration_count);
    printf("Number of basicblocks: %d\n", vec_length(basicblocks));
    printf("New code size: %d\n", code->used);

    return 0;
}

bitvec_t *decode_perm(vec_t *bbs){
    vec_t *orig_perm = vec_new(vec_length(bbs));
    vec_t *sort_perm = vec_new(vec_length(bbs));
    dict_t *bb_id = dict_new(vec_length(bbs));

    for(int i = 0; i < vec_length(bbs); i++){
        bb_t *bb = vec_get(bbs, i);    
        vec_push(sort_perm, next_new(basicblock_address(bb)));
        vec_push(orig_perm, next_new(basicblock_address(bb)));
    }

    bool sorted = false;
    while(!sorted){
        sorted = true;
        for(int i = 0; i < vec_length(sort_perm)-1; i++){
            next_t *a = vec_get(sort_perm, i);
            next_t *b = vec_get(sort_perm, i+1);
            if(a->address > b->address){
                sorted = false;
                uintptr_t tmp = a->address;
                a->address = b->address;
                b->address = tmp;
            }
        }
    }

    for(int i = 0; i < vec_length(sort_perm); i++){
        next_t *a = vec_get(sort_perm, i);
        dict_put(bb_id, a->address, next_new(i));
    }

    bitvec_t *bv = bitvec_new(0);
    for(int i = 1; i < vec_length(bbs)-1; i += 1){
        next_t *a = vec_get(sort_perm, i);
        next_t *b = vec_get(orig_perm, i);
        next_t *id_a = dict_get(bb_id, a->address);
        next_t *id_b = dict_get(bb_id, b->address);
        if(id_a->address == id_b->address){
            bitvec_push(bv, 0);
        } else {
            bitvec_push(bv, 1);
        }
    }

    return bv;
}

void encode_perm(vec_t *bbs, bitvec_t *bv){
    int n = 0;
    int last = vec_length(bbs) - 1;
    for(int i = 1; i < vec_length(bbs)-1; i += 1){
        if(bv->data[n / 8] & (1 << (n & 7))){
            void *tmp = bbs->elem[i];
            bbs->elem[i] = bbs->elem[last];
            bbs->elem[last] = tmp;
        }
        n++;
    } 
}

void setup(char *buffer, size_t size){
    printf("First execution, setting up!\n");
    setup_func = NULL;
};

void (*setup_func)(char *, size_t) = &setup;

char __attribute__((section(".text"))) zeroes[4096] = {0};

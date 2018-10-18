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

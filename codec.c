#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vec.h"
#include "dict.h"
#include "codec.h"

#include "disassembler.h"

decoder_t *decoder_new(vec_t *bbs){
    decoder_t *dec = malloc(sizeof(decoder_t));
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

    dec->orig_perm = orig_perm;
    dec->sort_perm = sort_perm;
    dec->bb_id = bb_id;
    dec->idx = 1;

    return dec;
}

int decoder_data_size(decoder_t *dec){
    return vec_length(dec->sort_perm) - 2;
}

uint32_t decoder_read_int(decoder_t *dec, int bits){
    uint32_t result = 0;
    vec_t *sort_perm = dec->sort_perm;
    vec_t *orig_perm = dec->orig_perm;
    dict_t *bb_id = dec->bb_id;

    for(int i = dec->idx; i < (dec->idx + bits); i += 1){
        next_t *a = vec_get(sort_perm, i);
        next_t *b = vec_get(orig_perm, i);
        next_t *id_a = dict_get(bb_id, a->address);
        next_t *id_b = dict_get(bb_id, b->address);

        if(id_a->address != id_b->address){
            result |= 1 << (i - dec->idx);
        }
    }

    dec->idx += bits;

    return result;
}

encoder_t *encoder_new(vec_t *bbs){
    encoder_t *enc = malloc(sizeof(encoder_t));
    enc->bbs = bbs;
    enc->last = vec_length(enc->bbs) - 1;
    enc->idx = 1;
    return enc;
}

int encoder_data_size(encoder_t *enc){
    return enc->idx - 1;
}

void encoder_write_int(encoder_t *enc, int bits, uint32_t value){
    int last = enc->last;
    vec_t *bbs = enc->bbs;
    for(int i = enc->idx; i < (enc->idx + bits); i++){
        if(value & (1 << (i - enc->idx))){
            void *tmp = bbs->elem[i];
            bbs->elem[i] = bbs->elem[last];
            bbs->elem[last] = tmp;
        }
    }
    enc->idx += bits;
}


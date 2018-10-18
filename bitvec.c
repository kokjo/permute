#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "bitvec.h"

bitvec_t *bitvec_new(size_t size){
    bitvec_t *bv = malloc(sizeof(bitvec_t));
    bv->size = (size + 7) & ~7;
    bv->used = 0;
    bv->data = malloc(bv->size / 8);
    return bv;
}

void bitvec_push(bitvec_t *bv, int bit){
    if(bv->used == bv->size){
        bv->size += 8;
        bv->data = realloc(bv->data, bv->size / 8);
    }
    uint8_t *b = &bv->data[bv->used / 8];
    *b = *b & ~(1 << (bv->used & 7));
    if(bit) *b |= 1 << (bv->used & 7);
    bv->used++;
}

int bitvec_get(bitvec_t *bv, int idx){
    return (bv->data[idx / 8] >> (idx & 7)) & 1;
}

void bitvec_print(bitvec_t *bv){
    for(int i = 0; i < bv->used; i++){
        printf("%d", bitvec_get(bv, i));
    }
    printf("\n");
}


bitvec_reader_t *bitvec_reader_new(bitvec_t *bv){
    bitvec_reader_t *r = malloc(sizeof(bitvec_reader_t));
    r->bv = bv;
    r->idx = 0;
    return r;
}

uint32_t bitvec_reader_int(bitvec_reader_t *r, int bits){
    uint32_t num = 0;
    while(bits--){
        num = (num << 1) | bitvec_get(r->bv, r->idx++);
    }
    return num;
}

void bitvec_write_int(bitvec_t *bv, int bits, uint32_t num){
    while(bits--){
        bitvec_push(bv, num & (1 << bits));
    }
}


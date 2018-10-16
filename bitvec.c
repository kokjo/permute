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

void bitvec_print(bitvec_t *bv){
    for(int i = 0; i < bv->used; i++){
        printf("%d", (bv->data[i / 8] >> (i & 7)) & 1);
    }
    printf("\n");
}

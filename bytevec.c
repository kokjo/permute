#include <stdlib.h>
#include <stdint.h>
#include "bytevec.h"

bytevec_t *bytevec_new(size_t alloc_size){
    if(alloc_size == 0) alloc_size = 1;
    bytevec_t *bv = malloc(sizeof(bytevec_t)); 
    bv->bytes = malloc(alloc_size);
    bv->allocated = alloc_size;
    bv->used = 0; 
    return bv;
}

void bytevec_push(bytevec_t *bv, uint8_t byte){
    if(bv->used == bv->allocated){
        bv->allocated = bv->allocated*2;
        bv->bytes = realloc(bv->bytes, bv->allocated);
    }
    bv->bytes[bv->used++] = byte;
}

void bytevec_extend(bytevec_t *bv, uint8_t *ptr, size_t size){
    for(int i = 0; i < size; i++) bytevec_push(bv, ptr[i]);
}

size_t bytevec_length(bytevec_t *bv){
    return bv->used;
}

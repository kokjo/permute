#include <stdlib.h>
#include <stdio.h>
#include "vec.h"
#include "util.h"

vec_t *vec_new(size_t size) {
    vec_t *vec = malloc(sizeof(vec_t));
    vec->elem = malloc(size*sizeof(void *));
    vec->size = size;
    vec->used = 0;
    return vec;
}

void vec_push(vec_t *vec, void *elem) {
    if(vec->used == vec->size){
        vec->size++;
        vec->elem = realloc(vec->elem, vec->size*sizeof(void*));
    }
    vec->elem[vec->used++] = elem;
}

void *vec_get(vec_t *vec, size_t idx) {
    if(idx < vec->used) return vec->elem[idx];
    return NULL;
}

size_t vec_length(vec_t *vec) {
    return vec->used;
}

void *vec_pop(vec_t *vec){
    if(vec->used == 0) return NULL;
    return vec->elem[--vec->used];
}

void vec_extend(vec_t *vec1, vec_t *vec2){
    for(int i = 0; i < vec2->used; i++)
        vec_push(vec1, vec2->elem[i]);
}

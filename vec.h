#ifndef _VEC_H_
#define _VEC_H_
#include <stddef.h>
#include <inttypes.h>

typedef struct vec {
    void **elem;
    size_t size;
    size_t used;
} vec_t;

vec_t *vec_new(size_t size);
void vec_push(vec_t *vec, void *elem);
void *vec_get(vec_t *vec, size_t idx);
size_t vec_length(vec_t *vec);
void *vec_pop(vec_t *vec);
void vec_extend(vec_t *a, vec_t *b);

#endif

#ifndef _BYTEVEC_H_
#define _BYTEVEC_H_

typedef struct bytevec {
    char *bytes;
    size_t allocated;
    size_t used;
} bytevec_t;

bytevec_t *bytevec_new(size_t alloc_size);
void bytevec_push(bytevec_t *bv, char byte);
void bytevec_extend(bytevec_t *bv, char *ptr, size_t size);

size_t bytevec_length(bytevec_t *bv);

#endif

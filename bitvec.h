#ifndef _BITVEC_H_
#define _BITVEC_H_

typedef struct bitvec {
    uint8_t *data;
    size_t size;
    size_t used;
} bitvec_t;

bitvec_t *bitvec_new(size_t size);
void bitvec_push(bitvec_t *bv, int bit);
void bitvec_print(bitvec_t *bv);


#endif

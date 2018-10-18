#ifndef _BITVEC_H_
#define _BITVEC_H_

typedef struct bitvec {
    uint8_t *data;
    size_t size;
    size_t used;
} bitvec_t;

typedef struct bitvec_reader {
    bitvec_t *bv;
    size_t idx;
} bitvec_reader_t;

bitvec_t *bitvec_new(size_t size);
void bitvec_push(bitvec_t *bv, int bit);
void bitvec_print(bitvec_t *bv);

bitvec_reader_t *bitvec_reader_new(bitvec_t *bv);
uint32_t bitvec_reader_int(bitvec_reader_t *r, int bits);
void bitvec_write_int(bitvec_t *bv, int bits, uint32_t num);


#endif

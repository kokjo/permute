#ifndef _CODEC_H_
#define _CODEC_H_
#include "vec.h"
#include "bitvec.h"

typedef struct encoder {
    vec_t *bbs;
    int last;
    int idx;
} encoder_t;

typedef struct decoder {
    vec_t *orig_perm;
    vec_t *sort_perm;
    dict_t *bb_id;
    int idx;
} decoder_t;

encoder_t *encoder_new(vec_t *bbs);
int encoder_data_size(encoder_t *enc);
void encoder_write_int(encoder_t *enc, int bits, uint32_t value);

decoder_t *decoder_new(vec_t *bbs);
int decoder_data_size(decoder_t *dec);
uint32_t decoder_read_int(decoder_t *dec, int bits);
#endif

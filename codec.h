#ifndef _CODEC_H_
#define _CODEC_H_

#include "vec.h"
#include "bitvec.h"

bitvec_t *decode_perm(vec_t *bbs);
void encode_perm(vec_t *bbs, bitvec_t *bv);

#endif

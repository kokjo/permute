#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "vec.h"
#include "bytevec.h"
#include "set.h"
#include "dict.h"
#include "util.h"
#include "disassembler.h"
#include "reassembler.h"
#include "elffile.h"
#include "bitvec.h"
#include "codec.h"

void _exit(int);

#define FUNC ((uintptr_t)&main)
int iteration_count = -1; // this cannot be zero, it needs to be in .data.

void setup(decoder_t *dec, encoder_t *enc, int argc, char **argv);
void crackme(decoder_t *dec, encoder_t *enc, int argc, char **argv);
void (*setup_func)(decoder_t *, encoder_t *, int argc, char **argv) = &setup;

volatile int debug = 0;

int main(int argc, char **argv) {
    vec_t *basicblocks = find_all_basicblocks(FUNC);

    if(debug) {
        for(int i = 0; i < vec_length(basicblocks); i++){
            bb_t *bb = vec_get(basicblocks, i);
            basicblock_print(bb);
        }
    }

    encoder_t *enc = encoder_new(basicblocks);
    decoder_t *dec = decoder_new(basicblocks);

    if(setup_func){
        setup_func(dec, enc, argc, argv);
    } else {
        crackme(dec, enc, argc, argv);
    }

    bytevec_t *code = reassemble_basicblocks(FUNC, basicblocks);
    elffile_t *ef = elffile_open(argv[0]);
    elffile_memcpy(ef, FUNC, code->bytes, code->used);
    elffile_memcpy(ef, (uintptr_t)&iteration_count, (void*)&iteration_count, sizeof(int));
    elffile_memcpy(ef, (uintptr_t)&setup_func, (void*)&setup_func, sizeof(void *));
    elffile_write(ef, argv[0]);

#ifdef DEBUG
    printf("Iteration count: %d\n", iteration_count);
    printf("Number of basicblocks: %d\n", vec_length(basicblocks));
    printf("New code size: %d\n", code->used);
#endif

    return 0;
}

void setup(decoder_t *dec, encoder_t *enc, int argc, char **argv){
    if(argc != 2){
        //printf("Usage: %s <flag_to_be_loaded>\n", argv[0]);
        printf("Wrong!\n");
        _exit(0);
    }

    char *flag = argv[1];

    if(decoder_data_size(dec) < (strlen(flag) + 2) * 8){
        printf("flag too big\n");
        _exit(0);
    }

    //printf("First execution, setting up!\n");

    setup_func = NULL;
    iteration_count = 0;

    encoder_write_int(enc, 8, strlen(flag));
    while(*flag) encoder_write_int(enc, 8, *flag++);
    encoder_write_int(enc, 8, 0);
    while(encoder_data_size(enc) < decoder_data_size(dec)){
        encoder_write_int(enc, 1, random() & 1);
    }
}

void crackme(decoder_t *dec, encoder_t *enc, int argc, char **argv){
    srand(iteration_count++);
    if(argc != 2 || strlen(argv[1]) != 1){
        printf("Usage: %s <flag byte>\n", argv[0]);
        encoder_write_int(enc, 16, 0xff05);
        return;
    }
    uint8_t length = decoder_read_int(dec, 8);
    if(length == 0){
        printf(decoder_read_int(dec, 8) == 0 ? "WIN!\n" : "LOSE!\n");
        encoder_write_int(enc, 16, 0);
        return;
    }
    uint8_t next_char = decoder_read_int(dec, 8);
    uint8_t chksum = next_char ^ argv[1][0];
    encoder_write_int(enc, 8, --length);
    while(length--){
        next_char = decoder_read_int(dec, 8);
        encoder_write_int(enc, 8, next_char);
    }
    chksum |= decoder_read_int(dec, 8);
    encoder_write_int(enc, 8, chksum);
    while(encoder_data_size(enc) < decoder_data_size(dec)){
        encoder_write_int(enc, 1, random() & 1);
    }
}

/* there will be more and longer jumps in the new binary. Put in some padding */
char __attribute__((section(".text"))) zeroes[0x2000] = {0};

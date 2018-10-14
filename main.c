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

int fib(int n);

#define FUNC ((uintptr_t)&main)

int main(int argc, char **argv) {
//    printf("fib(11) = %d\n", fib(11));
    vec_t *basicblocks = vec_new(0);
    set_t *funcs = find_all_functions(FUNC, NULL);
    
    for(int i = 0; i < vec_length(funcs); i++){
        set_entry_t *e = vec_get(funcs, i);
    //    printf("func_%p:\n",(void *)e->item);
        vec_t *code = disassemble(e->item);
        cfg_t *cfg = make_cfg(code, e->item); 
        cleanup_cfg(cfg);
        for(int j = 0; j < vec_length(cfg->basicblocks); j++){
            bb_t *bb = dict_elem(cfg->basicblocks, j)->value;
            basicblock_print(bb);
            vec_push(basicblocks, bb);
        }
//        printf("\n");
    }

    srand(time(0));
    for(int i = 1; i < vec_length(basicblocks); i++){
        int range_size = vec_length(basicblocks) - i;
        int j = i + (random() % range_size);
        void *elem_i = basicblocks->elem[i];
        void *elem_j = basicblocks->elem[j];
        basicblocks->elem[i] = elem_j;
        basicblocks->elem[j] = elem_i;
    }

    bytevec_t *code = reassemble_basicblocks(FUNC, basicblocks);
    printf("new code size: %d\n", code->used);
    elffile_t *ef = elffile_open(argv[0]);
    char *buffer = elffile_locate(ef, FUNC);
    memcpy(buffer, code->bytes, code->used);
    elffile_write(ef, "tmp");
    rename("tmp", argv[0]);

    return 0;
}

int fib(int n){
    if(n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

char __attribute__((section(".text"))) zeroes[4096] = {0};

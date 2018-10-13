#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "vec.h"
#include "set.h"
#include "dict.h"
#include "util.h"
#include "disassembler.h"

int main(int argc, char **argv) {
    set_t *funcs = find_all_functions((uintptr_t)&main, NULL);
    
    for(int i = 0; i < vec_length(funcs); i++){
        set_entry_t *e = vec_get(funcs, i);
        printf("func_%p:\n",(void *)e->item);
        vec_t *code = disassemble(e->item);
        cfg_t *cfg = make_cfg(code, e->item); 
        for(int j = 0; j < vec_length(cfg->basicblocks); j++){
            bb_t *bb = dict_elem(cfg->basicblocks, j)->value;
            basicblock_print(bb);
        }
        printf("\n");
    }

    return 0;
}

char __attribute__((section(".text"))) zeroes[1024] = {0};

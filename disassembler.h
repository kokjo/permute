#ifndef _DISASSEMBLER_H_
#define _DISASSEMBLER_H_

#include <stddef.h>
#include <inttypes.h>
#include "vec.h"
#include "set.h"
#include "dict.h"
#include "util.h"

typedef struct next {
    uintptr_t address;
} next_t;

typedef struct instruction {
    uintptr_t address;
    size_t size;
    char *mnemonic;
    char *op_str;
    uint8_t bytes[16];
    vec_t *nexts;
    uintptr_t call_target;
} ins_t;

typedef struct basicblock {
    vec_t *prevs;
    vec_t *instructions;
} bb_t;

typedef struct control_flow_graph {
    uintptr_t address;
    dict_t *basicblocks;
} cfg_t;

next_t *next_new(uintptr_t address);
dict_t *disassemble(uintptr_t address);
set_t *find_all_calls(dict_t *instructions);
set_t *find_all_functions(uintptr_t address, vec_t *seen);
ins_t *get_instruction(vec_t *instructions, uintptr_t address);
bb_t *basicblock_new(vec_t *prevs, ins_t *first_ins);
uintptr_t basicblock_address(bb_t *bb);
vec_t *basicblock_nexts(bb_t *bb);
vec_t *basicblock_prevs(bb_t *bb);
void basicblock_push_ins(bb_t *bb, ins_t *ins);
void basicblock_print(bb_t *bb);

cfg_t *make_cfg(dict_t *instructions, uintptr_t address);
void remove_single_jump_bb(cfg_t *cfg, bb_t *bb);
void cleanup_cfg(cfg_t *cfg);
vec_t *find_all_basicblocks(uintptr_t address);

#endif

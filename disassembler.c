#define _GNU_SOURCE
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <capstone/capstone.h>
#include "vec.h"
#include "set.h"
#include "dict.h"
#include "util.h"
#include "disassembler.h"

void main(int argc, char **argv);

bool function_address_valid(uintptr_t address){
    return address >= (uintptr_t) main; 
}

next_t *next_new(uintptr_t address){
    next_t *next = malloc(sizeof(next_t));
    next->address = address;
    return next;
}

ins_t *from_capstone(cs_insn *insn){
    ins_t *ins = malloc(sizeof(ins_t));
    ins->address = insn->address;
    ins->size = insn->size;
    memcpy(ins->bytes, insn->bytes, ins->size);
    ins->mnemonic = strdup(insn->mnemonic);
    ins->op_str = strdup(insn->op_str);
    ins->nexts = vec_new(2); 
    ins->call_target = 0;
    vec_push(ins->nexts, next_new(ins->address + ins->size));
    if(!strcmp(ins->mnemonic, "ret")) free(vec_pop(ins->nexts));
    if(!strcmp(ins->mnemonic, "jmp")) free(vec_pop(ins->nexts));
    uintptr_t jump_target = strtoull(ins->op_str, NULL, 16);
    if(ins->mnemonic[0] == 'j') {
        vec_push(ins->nexts, next_new(jump_target));
        asprintf(&ins->op_str, "bb_0x%x", jump_target);
    }
    if(!strcmp(ins->mnemonic, "call") && function_address_valid(jump_target)) {
        ins->call_target = jump_target;
        asprintf(&ins->op_str, "func_0x%x", ins->call_target);
    }
    return ins;
}

dict_t *disassemble(uintptr_t address){
    csh handle;
    unsigned char code_buffer[16];
    const unsigned char *code;
    size_t size = sizeof(code_buffer);
    ins_t *ins;
    vec_t *queue;
    cs_insn *insn;
    next_t *next;
    dict_t *ins_dict;
    set_t *seen;
    
    if(cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK){
        fatal(1, "cs_open failed");
    }

    queue = vec_new(1);
    vec_push(queue, next_new(address));
    ins_dict = dict_new(0);
    seen = set_new(0);

    insn = cs_malloc(handle);;
    while((next = vec_pop(queue)) != NULL){
        uint64_t code_address = next->address;
        size = sizeof(code_buffer);
        memcpy(code_buffer, (void *) next->address, sizeof(code_buffer));
        code = code_buffer;
        if(!cs_disasm_iter(handle, &code, &size, &code_address, insn)) continue;
        ins = from_capstone(insn);
        dict_put(ins_dict, ins->address, ins);
        for(int i = 0; i < vec_length(ins->nexts); i++){
            next_t *next = vec_get(ins->nexts, i);
            if(set_add(seen, next->address)) vec_push(queue, next);
        }
    }

    return ins_dict;
}

bb_t *basicblock_new(vec_t *prevs, ins_t *first_ins){
    bb_t *bb = malloc(sizeof(bb_t));
    bb->instructions = vec_new(1);
    bb->prevs = prevs;
    vec_push(bb->instructions, first_ins);
    return bb;
}

uintptr_t basicblock_address(bb_t *bb){
    ins_t *ins = vec_get(bb->instructions, 0);
    return ins->address;
}

vec_t *basicblock_nexts(bb_t *bb){
    ins_t *ins = vec_get(bb->instructions, vec_length(bb->instructions)-1);
    return ins->nexts;
}

vec_t *basicblock_prevs(bb_t *bb){
    return bb->prevs;
}

void basicblock_push_ins(bb_t *bb, ins_t *ins){
    vec_push(bb->instructions, ins);
}

void basicblock_print(bb_t *bb){
    ins_t *ins;
    printf("bb_%p:\n", basicblock_address(bb));
    for(int i = 0; i < vec_length(bb->instructions); i++){
        ins = vec_get(bb->instructions, i);
        printf("  %s %s\n", ins->mnemonic, ins->op_str);
    }
    vec_t *nexts = basicblock_nexts(bb);
    if(vec_length(nexts) > 0 && strcmp(ins->mnemonic, "jmp")){
        next_t *next = vec_get(nexts, 0);
        printf("  jmp bb_%p\n", next->address);
    }
}

cfg_t *make_cfg(dict_t *instructions, uintptr_t address){
    dict_t *prevs = dict_new(0); // dict of set
    for(int i = 0; i < vec_length(instructions); i++){
        ins_t *ins = dict_elem(instructions, i)->value;
        for(int j = 0; j < vec_length(ins->nexts); j++) {
            next_t *next = vec_get(ins->nexts, j);
            set_t *prev = dict_get(prevs, next->address);
            if(prev == NULL) prev = set_new(0);
            set_add(prev, ins->address);
            dict_put(prevs, next->address, prev);
        }
    }

    cfg_t *cfg = malloc(sizeof(cfg_t));
    cfg->address = address;
    cfg->basicblocks = dict_new(0);
    vec_t *seen = vec_new(0); // vec of next;
    vec_t *queue = vec_new(0); // vec of next;
    vec_push(queue, next_new(address));
    next_t *next;
    
    while((next = vec_pop(queue)) != NULL){
        ins_t *ins = dict_get(instructions, next->address);
        set_t *prev = dict_get(prevs, next->address);
        bb_t *bb = basicblock_new(prev, ins);
        while(vec_length(ins->nexts) == 1) {
            next_t *next = vec_get(ins->nexts, 0);
            ins = dict_get(instructions, next->address);
            if(vec_length(dict_get(prevs, next->address)) > 1) break;
            basicblock_push_ins(bb, ins); 
        }
        vec_t *nexts = basicblock_nexts(bb);
        for(int i=0; i < vec_length(nexts); i++){
            next_t *next = vec_get(nexts, i);
            if(set_add(seen, next->address)) vec_push(queue, next);
        }
        dict_put(cfg->basicblocks, basicblock_address(bb), bb);
    }

    return cfg;
}

set_t *find_all_calls(dict_t *instructions){
    set_t *funcs = set_new(0);
    for(int i = 0; i < vec_length(instructions); i++){
        ins_t *ins = dict_elem(instructions, i)->value;
        if(ins->call_target) set_add(funcs, ins->call_target);
    }
    return funcs;
}

set_t *find_all_functions(uintptr_t address, set_t *seen) {
    if(seen == NULL) seen = set_new(1);
    set_t *all_funcs = set_new(0);
    if(!function_address_valid(address) || set_contains(seen, address)) return all_funcs;
    set_add(all_funcs, address);
    set_add(seen, address);
    dict_t *ins_dict = disassemble(address);
    vec_t *funcs = find_all_calls(ins_dict);
    for(int i = 0; i < vec_length(funcs); i++){
        set_entry_t *e = vec_get(funcs, i);
        set_extend(all_funcs, find_all_functions(e->item, seen));
    }    
    return all_funcs;
}

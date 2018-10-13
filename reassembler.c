#include "disassembler.h"
#include "dict.h"
#include "vec.h"

void remove_single_jump_bb(cfg_t *cfg, bb_t *bb){
    uintptr_t next_bb = ((next_t *)vec_get(basicblock_nexts(bb), 0))->address;
    for(int i = 0; i < vec_length(bb->prevs); i++){
        set_entry_t *e = vec_get(bb->prevs, i);
        bb_t *prev_bb = dict_get(cfg->basicblocks, e->item);
        vec_t *nexts = basicblock_nexts(bb);
        for(int j = 0; j < vec_length(nexts); j++){
            next_t *next = vec_get(nexts, j);
            if(next->address == basicblock_address(bb)) next->address = next_bb;
        }
    }
}

void cleanup_cfg(cfg_t *cfg){
    for(int i = 0; i < vec_length(cfg->basicblocks); i++){
        bb_t *bb = dict_elem(cfg->basicblocks, i)->value;
        if(vec_length(bb->instructions) == 1){
            ins_t *ins = vec_get(bb->instructions, 0);
            if(!strcmp(ins->mnemonic, "jmp")){
                remove_single_jump_bb(cfg, bb);
            }
        } 
    }
}

typedef struct bytevec {
    char *bytes;
    size_t allocated;
    size_t used;
} bytevec_t;

bytevec_t *bytevec_new(size_t alloc_size){
    bytevec_t *bv = malloc(sizeof(bytevec_t)); 
    bv->bytes = malloc(alloc_size);
    bv->allocated = alloc_size;
    bv->used = 0; 
}

void bytevec_push(bytevec_t *bv, char byte){
    if(bv->used == bv->allocated){
        bv->allocated = bv->allocated*2;
        bv->bytes = realloc(bv->bytes, bv->allocated);
    }
    bv->bytes[bv->used++] = byte;
}

void bytevec_extend(bytevec_t *bv, char *ptr, size_t size){
    for(int i = 0; i < size; i++) bytevec_push(bv, ptr[i]);
}

size_t bytevec_length(bytevec_t *bv){
    return bv->used;
}

typedef struct assembler_state {
    uintptr_t origin;
    bytevec_t *code;
    dict_t *syms;
} asm_st_t;

uintptr_t asm_lookup_sym(asm_st_t *asm_st, uintptr_t sym){
    next_t *next = dict_get(asm_st->syms, sym);
    if(next) return next->address;
    return sym;
}

void asm_define_sym(asm_st_t *asm_st, uintptr_t sym, uintptr_t address){
    dict_put(asm_st->syms, sym, next_new(address));
}

uintptr_t asm_current_address(asm_st_t *asm_st){
    return asm_st->origin + bytevec_length(asm_st->code);
}

void asm_emit_rel_addr(asm_st_t *asm_st, uintptr_t address){
    uint32_t rel_addr = address - (asm_current_address(asm_st) + 4);
    bytevec_extend(asm_st->code, (void *)&rel_addr, sizeof(uint32_t));
}

void asm_emit_ins(asm_st_t *asm_st, ins_t *ins){
    if(ins->mnemonic[0] != 'j' && strcmp(ins->mnemonic, "call")){
        bytevec_extend(asm_st->code, ins->bytes, ins->size);
        return;
    }

    uintptr_t sym_addr = asm_lookup_sym(asm_st, strtoull(ins->op_str, NULL, 16));

    if(!strcmp(ins->mnemonic, "call")){
        bytevec_push(asm_st->code, 0xe8);
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    if(!strcmp(ins->mnemonic, "jmp")){
        bytevec_push(asm_st->code, 0xe9);
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    // conditional short jumps (8bit relative address)
    if((ins->bytes[0] & 0xf0) == 0x70) {
        bytevec_push(asm_st->code, 0x0f);
        bytevec_push(asm_st->code, 0x80 | (ins->bytes[0] & 0x0f));
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    // conditional long jumps (32bit relative address
    if(ins->bytes[0] == 0x0f && ((ins->bytes[1] & 0xf0) == 0x80)) {
        bytevec_extend(asm_st->code, ins->bytes, 2);
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }
}

void asm_emit_bb(asm_st_t *asm_st, bb_t *bb){
    asm_define_sym(asm_st, basicblock_address(bb), asm_current_address(asm_st));
    for(int i = 0; i < vec_length(bb->instructions); i++){
        asm_emit_ins(asm_st, vec_get(bb->instructions, i));
    }
    vec_t *nexts = basicblock_nexts(bb);
    if(vec_length(nexts) > 0){
        next_t *next = vec_get(nexts, 0);
        bytevec_push(asm_st->code, 0xe9);
        asm_emit_rel_addr(asm_st, next->address);
    }
}

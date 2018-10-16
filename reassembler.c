#include "dict.h"
#include "vec.h"
#include "bytevec.h"

#include "disassembler.h"
#include "reassembler.h"

asm_st_t *asm_new(uintptr_t origin, dict_t *syms){
    if(syms == NULL) syms = dict_new(0);
    asm_st_t *asm_st = malloc(sizeof(asm_st_t));
    asm_st->origin = origin;
    asm_st->code = bytevec_new(0);
    asm_st->syms = syms;
    return asm_st;
}

uintptr_t asm_lookup_sym(asm_st_t *asm_st, uintptr_t sym){
    next_t *next = dict_get(asm_st->syms, sym);
    if(next){
        return next->address;
    }
    return sym;
}

void asm_define_sym(asm_st_t *asm_st, uintptr_t sym, uintptr_t address){
//    printf("defining symbol: 0x%x -> 0x%x\n", sym, address);
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
    if(ins->mnemonic[0] != 'j' && strcmp(ins->mnemonic, "call") != 0){
        bytevec_extend(asm_st->code, ins->bytes, ins->size);
        return;
    }

    uintptr_t sym_addr = asm_lookup_sym(asm_st, strtoull(ins->op_str, NULL, 16));

    if(ins->bytes[0] == 0xe8){
        bytevec_push(asm_st->code, 0xe8);
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    if(ins->bytes[0] == 0xe9){
        bytevec_push(asm_st->code, 0xe9);
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    // conditional short jumps (8bit relative address)
    if((ins->bytes[0] & 0xf0) == 0x70) {
        // convert to long jump
        bytevec_push(asm_st->code, 0x0f);
        bytevec_push(asm_st->code, 0x80 | (ins->bytes[0] & 0x0f));
        asm_emit_rel_addr(asm_st, sym_addr);
        return;
    }

    // conditional long jumps (32bit relative address)
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
        asm_emit_rel_addr(asm_st, asm_lookup_sym(asm_st, next->address));
    }
}

bytevec_t *reassemble_basicblocks(uintptr_t origin, vec_t *basicblocks){
    asm_st_t *asm_st = asm_new(origin, NULL);
    for(int i = 0; i < vec_length(basicblocks); i++){
        bb_t *bb = vec_get(basicblocks, i);
        asm_emit_bb(asm_st, bb);
    }
    asm_st = asm_new(origin, asm_st->syms);
    for(int i = 0; i < vec_length(basicblocks); i++){
        bb_t *bb = vec_get(basicblocks, i);
        asm_emit_bb(asm_st, bb);
    }
    return asm_st->code;
}

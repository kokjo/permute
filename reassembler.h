#ifndef _REASSEMBLER_H_
#define _REASSEMBLER_H_


typedef struct assembler_state {
    uintptr_t origin;
    bytevec_t *code;
    dict_t *syms;
} asm_st_t;

asm_st_t *asm_new(uintptr_t origin, dict_t *syms);
uintptr_t asm_lookup_sym(asm_st_t *asm_st, uintptr_t sym);
void asm_define_sym(asm_st_t *asm_st, uintptr_t sym, uintptr_t address);
uintptr_t asm_current_address(asm_st_t *asm_st);
void asm_emit_rel_addr(asm_st_t *asm_st, uintptr_t address);
void asm_emit_ins(asm_st_t *asm_st, ins_t *ins);
void asm_emit_bb(asm_st_t *asm_st, bb_t *bb);
bytevec_t *reassemble_basicblocks(uintptr_t origin, vec_t *basicblocks);

#endif

#include "sm83.h"
#include "sm83_opcodes.h"
#include "sm83_alu.h"

/* 8-bit load instruction declarations */
static unsigned op_ld_r8_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_a_imm16mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_imm16mem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_sp_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_cmem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_a_cmem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_a_imm8mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_imm8mem_a(sm83_t* sm83, uint8_t opcode);

/* 16-bit load instruction declarations */
static unsigned op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_sp_hl(sm83_t* sm83, uint8_t opcode);
static unsigned op_push_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_pop_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_hl_spe8(sm83_t* sm83, uint8_t opcode);

/* 8-bit arithmetic and logical instruction declarations */
static unsigned op_alu_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_alu_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_inc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_dec_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ccf(sm83_t* sm83, uint8_t opcode);
static unsigned op_scf(sm83_t* sm83, uint8_t opcode);
static unsigned op_daa(sm83_t* sm83, uint8_t opcode);
static unsigned op_cpl(sm83_t* sm83, uint8_t opcode);

/* 16-bit arithmetic instruction declarations */
static unsigned op_inc_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_dec_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_add_hl_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_add_sp_e8(sm83_t* sm83, uint8_t opcode);

/* Rotate, shift, and bit operation instruction declarations */
static unsigned op_rlca(sm83_t* sm83, uint8_t opcode);
static unsigned op_rrca(sm83_t* sm83, uint8_t opcode);
static unsigned op_rla(sm83_t* sm83, uint8_t opcode);
static unsigned op_rra(sm83_t* sm83, uint8_t opcode);
static unsigned op_rlc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rrc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rl_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rr_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_sla_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_sra_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_swap_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_srl_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_bit_b3_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_res_b3_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_set_b3_r8(sm83_t* sm83, uint8_t opcode);

/* Control flow instruction declarations */
static unsigned op_jp_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_jp_hl(sm83_t* sm83, uint8_t opcode);
static unsigned op_jp_cc_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_call_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_call_cc_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ret(sm83_t* sm83, uint8_t opcode);
static unsigned op_ret_cc(sm83_t* sm83, uint8_t opcode);
static unsigned op_reti(sm83_t* sm83, uint8_t opcode);
static unsigned op_rst(sm83_t* sm83, uint8_t opcode);

/* Miscellaneous instruction declarations */
static unsigned op_halt(sm83_t* sm83, uint8_t opcode);
static unsigned op_stop(sm83_t* sm83, uint8_t opcode);
static unsigned op_di(sm83_t* sm83, uint8_t opcode);
static unsigned op_ei(sm83_t* sm83, uint8_t opcode);
static unsigned op_nop(sm83_t* sm83, uint8_t opcode);

/* Disassembly helper declarations */
static void format_operand(char* out, size_t out_size, sm83_operand_t operand, uint8_t imm8, uint16_t imm16);
static const char* operation_name(sm83_operation_t operation);
static const char* register_name(sm83_register_t reg);
static const char* condition_name(sm83_cond_t cond);

/* Opcode dispatch table */

const sm83_instruction_t sm83_opcode_table[256] = {
    [0x00] = {op_nop, 1, SM83_OP_NOP, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x01] = {op_ld_r16_imm16, 3, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_BC}, {SM83_OPERAND_IMM16, 0}},
    [0x02] = {op_ld_r16mem_a, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_BC}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x03] = {op_inc_r16, 1, SM83_OP_INC, {SM83_OPERAND_R16, SM83_REG_BC}, {SM83_OPERAND_NONE, 0}},
    [0x04] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_NONE, 0}},
    [0x05] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_NONE, 0}},
    [0x06] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_IMM8, 0}},
    [0x07] = {op_rlca, 1, SM83_OP_RLCA, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x08] = {op_ld_imm16mem_sp, 3, SM83_OP_LD, {SM83_OPERAND_MEM_IMM16, 0}, {SM83_OPERAND_R16, SM83_REG_SP}},
    [0x09] = {op_add_hl_r16, 1, SM83_OP_ADD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_R16, SM83_REG_BC}},
    [0x0A] = {op_ld_a_r16mem, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_BC}},
    [0x0B] = {op_dec_r16, 1, SM83_OP_DEC, {SM83_OPERAND_R16, SM83_REG_BC}, {SM83_OPERAND_NONE, 0}},
    [0x0C] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_NONE, 0}},
    [0x0D] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_NONE, 0}},
    [0x0E] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_IMM8, 0}},
    [0x0F] = {op_rrca, 1, SM83_OP_RRCA, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},

    [0x10] = {op_stop, 2, SM83_OP_STOP, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x11] = {op_ld_r16_imm16, 3, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_DE}, {SM83_OPERAND_IMM16, 0}},
    [0x12] = {op_ld_r16mem_a, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_DE}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x13] = {op_inc_r16, 1, SM83_OP_INC, {SM83_OPERAND_R16, SM83_REG_DE}, {SM83_OPERAND_NONE, 0}},
    [0x14] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_NONE, 0}},
    [0x15] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_NONE, 0}},
    [0x16] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_IMM8, 0}},
    [0x17] = {op_rla, 1, SM83_OP_RLA, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x18] = {op_jr_imm8, 2, SM83_OP_JR, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0x19] = {op_add_hl_r16, 1, SM83_OP_ADD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_R16, SM83_REG_DE}},
    [0x1A] = {op_ld_a_r16mem, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_DE}},
    [0x1B] = {op_dec_r16, 1, SM83_OP_DEC, {SM83_OPERAND_R16, SM83_REG_DE}, {SM83_OPERAND_NONE, 0}},
    [0x1C] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_NONE, 0}},
    [0x1D] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_NONE, 0}},
    [0x1E] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_IMM8, 0}},
    [0x1F] = {op_rra, 1, SM83_OP_RRA, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},

    [0x20] = {op_jr_cc_imm8, 2, SM83_OP_JR, {SM83_OPERAND_COND, SM83_COND_NZ}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0x21] = {op_ld_r16_imm16, 3, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_IMM16, 0}},
    [0x22] = {op_ld_r16mem_a, 1, SM83_OP_LD, {SM83_OPERAND_HLI, 0}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x23] = {op_inc_r16, 1, SM83_OP_INC, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0x24] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_NONE, 0}},
    [0x25] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_NONE, 0}},
    [0x26] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_IMM8, 0}},
    [0x27] = {op_daa, 1, SM83_OP_DAA, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x28] = {op_jr_cc_imm8, 2, SM83_OP_JR, {SM83_OPERAND_COND, SM83_COND_Z}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0x29] = {op_add_hl_r16, 1, SM83_OP_ADD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_R16, SM83_REG_HL}},
    [0x2A] = {op_ld_a_r16mem, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_HLI, 0}},
    [0x2B] = {op_dec_r16, 1, SM83_OP_DEC, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0x2C] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_NONE, 0}},
    [0x2D] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_NONE, 0}},
    [0x2E] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_IMM8, 0}},
    [0x2F] = {op_cpl, 1, SM83_OP_CPL, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},

    [0x30] = {op_jr_cc_imm8, 2, SM83_OP_JR, {SM83_OPERAND_COND, SM83_COND_NC}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0x31] = {op_ld_sp_imm16, 3, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_SP}, {SM83_OPERAND_IMM16, 0}},
    [0x32] = {op_ld_r16mem_a, 1, SM83_OP_LD, {SM83_OPERAND_HLD, 0}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x33] = {op_inc_r16, 1, SM83_OP_INC, {SM83_OPERAND_R16, SM83_REG_SP}, {SM83_OPERAND_NONE, 0}},
    [0x34] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0x35] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0x36] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_IMM8, 0}},
    [0x37] = {op_scf, 1, SM83_OP_SCF, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x38] = {op_jr_cc_imm8, 2, SM83_OP_JR, {SM83_OPERAND_COND, SM83_COND_C}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0x39] = {op_add_hl_r16, 1, SM83_OP_ADD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_R16, SM83_REG_SP}},
    [0x3A] = {op_ld_a_r16mem, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_HLD, 0}},
    [0x3B] = {op_dec_r16, 1, SM83_OP_DEC, {SM83_OPERAND_R16, SM83_REG_SP}, {SM83_OPERAND_NONE, 0}},
    [0x3C] = {op_inc_r8, 1, SM83_OP_INC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_NONE, 0}},
    [0x3D] = {op_dec_r8, 1, SM83_OP_DEC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_NONE, 0}},
    [0x3E] = {op_ld_r8_imm8, 2, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0x3F] = {op_ccf, 1, SM83_OP_CCF, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x40] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x41] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x42] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x43] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x44] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x45] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x46] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x47] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_B}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x48] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x49] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x4A] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x4B] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x4C] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x4D] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x4E] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x4F] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0x50] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x51] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x52] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x53] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x54] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x55] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x56] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x57] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_D}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x58] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x59] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x5A] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x5B] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x5C] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x5D] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x5E] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x5F] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_E}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0x60] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x61] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x62] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x63] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x64] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x65] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x66] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x67] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_H}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x68] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x69] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x6A] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x6B] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x6C] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x6D] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x6E] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x6F] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_L}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0x70] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x71] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x72] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x73] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x74] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x75] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x76] = {op_halt, 1, SM83_OP_HALT, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0x77] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_MEM_R16, SM83_REG_HL}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x78] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x79] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x7A] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x7B] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x7C] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x7D] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x7E] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x7F] = {op_ld_r8_r8, 1, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0x80] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x81] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x82] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x83] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x84] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x85] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x86] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x87] = {op_alu_r8, 1, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x88] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x89] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x8A] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x8B] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x8C] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x8D] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x8E] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x8F] = {op_alu_r8, 1, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0x90] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x91] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x92] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x93] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x94] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x95] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x96] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x97] = {op_alu_r8, 1, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0x98] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0x99] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0x9A] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0x9B] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0x9C] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0x9D] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0x9E] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0x9F] = {op_alu_r8, 1, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0xA0] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0xA1] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0xA2] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0xA3] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0xA4] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0xA5] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0xA6] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0xA7] = {op_alu_r8, 1, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0xA8] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0xA9] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0xAA] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0xAB] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0xAC] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0xAD] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0xAE] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0xAF] = {op_alu_r8, 1, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0xB0] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0xB1] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0xB2] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0xB3] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0xB4] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0xB5] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0xB6] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0xB7] = {op_alu_r8, 1, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0xB8] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_B}},
    [0xB9] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_C}},
    [0xBA] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_D}},
    [0xBB] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_E}},
    [0xBC] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_H}},
    [0xBD] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_L}},
    [0xBE] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_R16, SM83_REG_HL}},
    [0xBF] = {op_alu_r8, 1, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_R8, SM83_REG_A}},

    [0xC0] = {op_ret_cc, 1, SM83_OP_RET, {SM83_OPERAND_COND, SM83_COND_NZ}, {SM83_OPERAND_NONE, 0}},
    [0xC1] = {op_pop_r16, 1, SM83_OP_POP, {SM83_OPERAND_R16, SM83_REG_BC}, {SM83_OPERAND_NONE, 0}},
    [0xC2] = {op_jp_cc_imm16, 3, SM83_OP_JP, {SM83_OPERAND_COND, SM83_COND_NZ}, {SM83_OPERAND_IMM16, 0}},
    [0xC3] = {op_jp_imm16, 3, SM83_OP_JP, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_IMM16, 0}},
    [0xC4] = {op_call_cc_imm16, 3, SM83_OP_CALL, {SM83_OPERAND_COND, SM83_COND_NZ}, {SM83_OPERAND_IMM16, 0}},
    [0xC5] = {op_push_r16, 1, SM83_OP_PUSH, {SM83_OPERAND_R16, SM83_REG_BC}, {SM83_OPERAND_NONE, 0}},
    [0xC6] = {op_alu_imm8, 2, SM83_OP_ADD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xC7] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x00}, {SM83_OPERAND_NONE, 0}},
    [0xC8] = {op_ret_cc, 1, SM83_OP_RET, {SM83_OPERAND_COND, SM83_COND_Z}, {SM83_OPERAND_NONE, 0}},
    [0xC9] = {op_ret, 1, SM83_OP_RET, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0xCA] = {op_jp_cc_imm16, 3, SM83_OP_JP, {SM83_OPERAND_COND, SM83_COND_Z}, {SM83_OPERAND_IMM16, 0}},
    [0xCC] = {op_call_cc_imm16, 3, SM83_OP_CALL, {SM83_OPERAND_COND, SM83_COND_Z}, {SM83_OPERAND_IMM16, 0}},
    [0xCD] = {op_call_imm16, 3, SM83_OP_CALL, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_IMM16, 0}},
    [0xCE] = {op_alu_imm8, 2, SM83_OP_ADC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xCF] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x08}, {SM83_OPERAND_NONE, 0}},

    [0xD0] = {op_ret_cc, 1, SM83_OP_RET, {SM83_OPERAND_COND, SM83_COND_NC}, {SM83_OPERAND_NONE, 0}},
    [0xD1] = {op_pop_r16, 1, SM83_OP_POP, {SM83_OPERAND_R16, SM83_REG_DE}, {SM83_OPERAND_NONE, 0}},
    [0xD2] = {op_jp_cc_imm16, 3, SM83_OP_JP, {SM83_OPERAND_COND, SM83_COND_NC}, {SM83_OPERAND_IMM16, 0}},
    [0xD4] = {op_call_cc_imm16, 3, SM83_OP_CALL, {SM83_OPERAND_COND, SM83_COND_NC}, {SM83_OPERAND_IMM16, 0}},
    [0xD5] = {op_push_r16, 1, SM83_OP_PUSH, {SM83_OPERAND_R16, SM83_REG_DE}, {SM83_OPERAND_NONE, 0}},
    [0xD6] = {op_alu_imm8, 2, SM83_OP_SUB, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xD7] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x10}, {SM83_OPERAND_NONE, 0}},
    [0xD8] = {op_ret_cc, 1, SM83_OP_RET, {SM83_OPERAND_COND, SM83_COND_C}, {SM83_OPERAND_NONE, 0}},
    [0xD9] = {op_reti, 1, SM83_OP_RETI, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0xDA] = {op_jp_cc_imm16, 3, SM83_OP_JP, {SM83_OPERAND_COND, SM83_COND_C}, {SM83_OPERAND_IMM16, 0}},
    [0xDC] = {op_call_cc_imm16, 3, SM83_OP_CALL, {SM83_OPERAND_COND, SM83_COND_C}, {SM83_OPERAND_IMM16, 0}},
    [0xDE] = {op_alu_imm8, 2, SM83_OP_SBC, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xDF] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x18}, {SM83_OPERAND_NONE, 0}},

    [0xE0] = {op_ldh_imm8mem_a, 2, SM83_OP_LDH, {SM83_OPERAND_MEM_HIGH_IMM8, 0}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0xE1] = {op_pop_r16, 1, SM83_OP_POP, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0xE2] = {op_ldh_cmem_a, 1, SM83_OP_LDH, {SM83_OPERAND_MEM_HIGH_C, SM83_REG_C}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0xE5] = {op_push_r16, 1, SM83_OP_PUSH, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_NONE, 0}},
    [0xE6] = {op_alu_imm8, 2, SM83_OP_AND, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xE7] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x20}, {SM83_OPERAND_NONE, 0}},
    [0xE8] = {op_add_sp_e8, 2, SM83_OP_ADD, {SM83_OPERAND_R16, SM83_REG_SP}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0xE9] = {op_jp_hl, 1, SM83_OP_JP, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_R16, SM83_REG_HL}},
    [0xEA] = {op_ld_imm16mem_a, 3, SM83_OP_LD, {SM83_OPERAND_MEM_IMM16, 0}, {SM83_OPERAND_R8, SM83_REG_A}},
    [0xEE] = {op_alu_imm8, 2, SM83_OP_XOR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xEF] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x28}, {SM83_OPERAND_NONE, 0}},

    [0xF0] = {op_ldh_a_imm8mem, 2, SM83_OP_LDH, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_HIGH_IMM8, 0}},
    [0xF1] = {op_pop_r16, 1, SM83_OP_POP, {SM83_OPERAND_R16, SM83_REG_AF}, {SM83_OPERAND_NONE, 0}},
    [0xF2] = {op_ldh_a_cmem, 1, SM83_OP_LDH, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_HIGH_C, SM83_REG_C}},
    [0xF3] = {op_di, 1, SM83_OP_DI, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0xF5] = {op_push_r16, 1, SM83_OP_PUSH, {SM83_OPERAND_R16, SM83_REG_AF}, {SM83_OPERAND_NONE, 0}},
    [0xF6] = {op_alu_imm8, 2, SM83_OP_OR, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xF7] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x30}, {SM83_OPERAND_NONE, 0}},
    [0xF8] = {op_ld_hl_spe8, 2, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_HL}, {SM83_OPERAND_SIGNED_IMM8, 0}},
    [0xF9] = {op_ld_sp_hl, 1, SM83_OP_LD, {SM83_OPERAND_R16, SM83_REG_SP}, {SM83_OPERAND_R16, SM83_REG_HL}},
    [0xFA] = {op_ld_a_imm16mem, 3, SM83_OP_LD, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_MEM_IMM16, 0}},
    [0xFB] = {op_ei, 1, SM83_OP_EI, {SM83_OPERAND_NONE, 0}, {SM83_OPERAND_NONE, 0}},
    [0xFE] = {op_alu_imm8, 2, SM83_OP_CP, {SM83_OPERAND_R8, SM83_REG_A}, {SM83_OPERAND_IMM8, 0}},
    [0xFF] = {op_rst, 1, SM83_OP_RST, {SM83_OPERAND_IMM16, 0x38}, {SM83_OPERAND_NONE, 0}}
};

const sm83_instruction_t sm83_opcode_table_cb[256] = {
    [0x00] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x01] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x02] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x03] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x04] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x05] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x06] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x07] = { op_rlc_r8, 2, SM83_OP_RLC, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x08] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x09] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0A] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0B] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0C] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0D] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0E] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x0F] = { op_rrc_r8, 2, SM83_OP_RRC, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },

    [0x10] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x11] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x12] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x13] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x14] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x15] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x16] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x17] = { op_rl_r8, 2, SM83_OP_RL, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x18] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x19] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1A] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1B] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1C] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1D] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1E] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x1F] = { op_rr_r8, 2, SM83_OP_RR, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },

    [0x20] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x21] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x22] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x23] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x24] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x25] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x26] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x27] = { op_sla_r8, 2, SM83_OP_SLA, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x28] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x29] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2A] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2B] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2C] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2D] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2E] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x2F] = { op_sra_r8, 2, SM83_OP_SRA, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },

    [0x30] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x31] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x32] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x33] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x34] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x35] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x36] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x37] = { op_swap_r8, 2, SM83_OP_SWAP, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x38] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_B }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x39] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_C }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3A] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_D }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3B] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_E }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3C] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_H }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3D] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_L }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3E] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_MEM_R8, SM83_REG_HL }, { SM83_OPERAND_NONE, SM83_REG_NONE } },
    [0x3F] = { op_srl_r8, 2, SM83_OP_SRL, { SM83_OPERAND_R8, SM83_REG_A }, { SM83_OPERAND_NONE, SM83_REG_NONE } },

    [0x40] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x41] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x42] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x43] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x44] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x45] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x46] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x47] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x48] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x49] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x4A] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x4B] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x4C] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x4D] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x4E] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x4F] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0x50] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x51] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x52] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x53] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x54] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x55] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x56] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x57] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x58] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x59] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x5A] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x5B] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x5C] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x5D] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x5E] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x5F] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0x60] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x61] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x62] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x63] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x64] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x65] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x66] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x67] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x68] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x69] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x6A] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x6B] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x6C] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x6D] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x6E] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x6F] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0x70] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x71] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x72] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x73] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x74] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x75] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x76] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x77] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x78] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x79] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x7A] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x7B] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x7C] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x7D] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x7E] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x7F] = { op_bit_b3_r8, 2, SM83_OP_BIT, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0x80] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x81] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x82] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x83] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x84] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x85] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x86] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x87] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x88] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x89] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x8A] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x8B] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x8C] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x8D] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x8E] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x8F] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0x90] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x91] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x92] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x93] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x94] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x95] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x96] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x97] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0x98] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0x99] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0x9A] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0x9B] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0x9C] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0x9D] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0x9E] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0x9F] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xA0] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xA1] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xA2] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xA3] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xA4] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xA5] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xA6] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xA7] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xA8] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xA9] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xAA] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xAB] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xAC] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xAD] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xAE] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xAF] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xB0] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xB1] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xB2] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xB3] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xB4] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xB5] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xB6] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xB7] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xB8] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xB9] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xBA] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xBB] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xBC] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xBD] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xBE] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xBF] = { op_res_b3_r8, 2, SM83_OP_RES, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xC0] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xC1] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xC2] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xC3] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xC4] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xC5] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xC6] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xC7] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 0 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xC8] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xC9] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xCA] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xCB] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xCC] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xCD] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xCE] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xCF] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 1 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xD0] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xD1] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xD2] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xD3] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xD4] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xD5] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xD6] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xD7] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 2 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xD8] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xD9] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xDA] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xDB] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xDC] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xDD] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xDE] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xDF] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 3 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xE0] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xE1] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xE2] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xE3] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xE4] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xE5] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xE6] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xE7] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 4 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xE8] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xE9] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xEA] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xEB] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xEC] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xED] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xEE] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xEF] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 5 }, { SM83_OPERAND_R8, SM83_REG_A } },

    [0xF0] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xF1] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xF2] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xF3] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xF4] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xF5] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xF6] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xF7] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 6 }, { SM83_OPERAND_R8, SM83_REG_A } },
    [0xF8] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_B } },
    [0xF9] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_C } },
    [0xFA] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_D } },
    [0xFB] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_E } },
    [0xFC] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_H } },
    [0xFD] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_L } },
    [0xFE] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_MEM_R8, SM83_REG_HL } },
    [0xFF] = { op_set_b3_r8, 2, SM83_OP_SET, { SM83_OPERAND_BIT, 7 }, { SM83_OPERAND_R8, SM83_REG_A } }
};

void sm83_disassemble(sm83_t* sm83, uint8_t opcode, const sm83_instruction_t* table, uint16_t pc, char* out, size_t out_size) {
    sm83_instruction_t instruction = table[opcode];

    uint8_t b1 = instruction.length > 1 ? sm83_peek8(sm83, pc + 1) : 0;
    uint8_t b2 = instruction.length > 2 ? sm83_peek8(sm83, pc + 2) : 0;

    uint16_t imm16 = ((uint16_t)(b2 << 8) | b1);

    char dst[24] = "";
    char src[24] = "";

    format_operand(dst, sizeof dst, instruction.dst, b1, imm16);
    format_operand(src, sizeof src, instruction.src, b1, imm16);

    if (instruction.dst.kind == SM83_OPERAND_NONE) {
        snprintf(out, out_size, "%s", operation_name(instruction.operation));
    } else if (instruction.src.kind == SM83_OPERAND_NONE) {
        snprintf(out, out_size, "%s %s", operation_name(instruction.operation), dst);
    } else {
        snprintf(out, out_size, "%s %s, %s", operation_name(instruction.operation), dst, src);
    }
}


static void format_operand(char* out, size_t out_size, sm83_operand_t operand, uint8_t imm8, uint16_t imm16) {
    switch (operand.kind) {
    case SM83_OPERAND_NONE:
        out[0] = '\0';
        break;
    case SM83_OPERAND_R8:
    case SM83_OPERAND_R16:
        snprintf(out, out_size, "%s", register_name((sm83_register_t)operand.value));
        break;
    case SM83_OPERAND_MEM_R8:
    case SM83_OPERAND_MEM_R16:
    case SM83_OPERAND_MEM_HIGH_C:
        snprintf(out, out_size, "[%s]", register_name((sm83_register_t)operand.value));
        break;
    case SM83_OPERAND_HLI:
        snprintf(out, out_size, "[HL+]");
        break;
    case SM83_OPERAND_HLD:
        snprintf(out, out_size, "[HL-]");
        break;
    case SM83_OPERAND_IMM8:
        snprintf(out, out_size, "$%02X", imm8);
        break;
    case SM83_OPERAND_SIGNED_IMM8:
        snprintf(out, out_size, "%+d", (int8_t)imm8);
        break;
    case SM83_OPERAND_IMM16:
        snprintf(out, out_size, "$%04X", imm16);
        break;
    case SM83_OPERAND_MEM_HIGH_IMM8:
        snprintf(out, out_size, "[$%02X]", imm8);
        break;
    case SM83_OPERAND_MEM_IMM16:
        snprintf(out, out_size, "[$%04X]", imm16);
        break;
    case SM83_OPERAND_COND:
        snprintf(out, out_size, "%s", condition_name((sm83_cond_t)operand.value));
        break;
    case SM83_OPERAND_BIT:
        snprintf(out, out_size, "%u", operand.value);
        break;
    default:
        snprintf(out, out_size, "???");
        break;
    }
}

static const char* operation_name(sm83_operation_t operation) {
    switch (operation) {
    case SM83_OP_NOP: return "NOP";
    case SM83_OP_LD: return "LD";
    case SM83_OP_LDH: return "LDH";
    case SM83_OP_INC: return "INC";
    case SM83_OP_DEC: return "DEC";
    case SM83_OP_ADD: return "ADD";
    case SM83_OP_ADC: return "ADC";
    case SM83_OP_SUB: return "SUB";
    case SM83_OP_SBC: return "SBC";
    case SM83_OP_AND: return "AND";
    case SM83_OP_XOR: return "XOR";
    case SM83_OP_OR: return "OR";
    case SM83_OP_CP: return "CP";
    case SM83_OP_DAA: return "DAA";
    case SM83_OP_CPL: return "CPL";
    case SM83_OP_SCF: return "SCF";
    case SM83_OP_CCF: return "CCF";
    case SM83_OP_RLCA: return "RLCA";
    case SM83_OP_RRCA: return "RRCA";
    case SM83_OP_RLA: return "RLA";
    case SM83_OP_RRA: return "RRA";
    case SM83_OP_RLC: return "RLC";
    case SM83_OP_RRC: return "RRC";
    case SM83_OP_RL: return "RL";
    case SM83_OP_RR: return "RR";
    case SM83_OP_SLA: return "SLA";
    case SM83_OP_SRA: return "SRA";
    case SM83_OP_SWAP: return "SWAP";
    case SM83_OP_SRL: return "SRL";
    case SM83_OP_BIT: return "BIT";
    case SM83_OP_RES: return "RES";
    case SM83_OP_SET: return "SET";
    case SM83_OP_JP: return "JP";
    case SM83_OP_JR: return "JR";
    case SM83_OP_CALL: return "CALL";
    case SM83_OP_RET: return "RET";
    case SM83_OP_RETI: return "RETI";
    case SM83_OP_RST: return "RST";
    case SM83_OP_PUSH: return "PUSH";
    case SM83_OP_POP: return "POP";
    case SM83_OP_HALT: return "HALT";
    case SM83_OP_STOP: return "STOP";
    case SM83_OP_DI: return "DI";
    case SM83_OP_EI: return "EI";
    default: return "???";
    }
}

static const char* register_name(sm83_register_t reg) {
    switch (reg) {
    case SM83_REG_A: return "A";
    case SM83_REG_B: return "B";
    case SM83_REG_C: return "C";
    case SM83_REG_D: return "D";
    case SM83_REG_E: return "E";
    case SM83_REG_H: return "H";
    case SM83_REG_L: return "L";
    case SM83_REG_AF: return "AF";
    case SM83_REG_BC: return "BC";
    case SM83_REG_DE: return "DE";
    case SM83_REG_HL: return "HL";
    case SM83_REG_SP: return "SP";
    case SM83_REG_PC: return "PC";
    default: return "???";
    }
}

static const char* condition_name(sm83_cond_t cond) {
    switch (cond) {
    case SM83_COND_NZ: return "NZ";
    case SM83_COND_Z: return "Z";
    case SM83_COND_NC: return "NC";
    case SM83_COND_C: return "C";
    }
}

/* 8-bit load instruction definitions */

static unsigned op_ld_r8_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = OPCODE_Y(opcode);
    uint8_t src = OPCODE_Z(opcode);

    if (dst == 6 && src == 6) {
        sm83->halted = true;
        sm83->state = SM83_STATE_HALTED;
    }

    uint8_t value = sm83_read_r8(sm83, src);
    sm83_write_r8(sm83, dst, value);

    return 1;
}

static unsigned op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = OPCODE_Y(opcode);
    uint8_t value = sm83_fetch8(sm83);

    sm83_write_r8(sm83, dst, value);

    return 2;
}

static unsigned op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint16_t address = sm83_read_r16mem(sm83, pair);
    bus_write(sm83->bus, BUS_CALLER_SM83, address, sm83->registers.a);

    if (pair == SM83_R16MEM_HLI) {
        sm83_write_r16(sm83, SM83_R16_HL, address + 1);
    } else if(pair == SM83_R16MEM_HLD) {
        sm83_write_r16(sm83, SM83_R16_HL, address - 1);
    }

    return 2;
}

static unsigned op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint16_t address = sm83_read_r16mem(sm83, pair);
    sm83->registers.a = bus_read(sm83->bus, BUS_CALLER_SM83, address);

    if (pair == SM83_R16MEM_HLI) {
        sm83_write_r16(sm83, SM83_R16_HL, address + 1);
    } else if(pair == SM83_R16MEM_HLD) {
        sm83_write_r16(sm83, SM83_R16_HL, address - 1);
    }

    return 2;
}

static unsigned op_ld_a_imm16mem(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t address = sm83_fetch16(sm83);
    sm83->registers.a = bus_read(sm83->bus, BUS_CALLER_SM83, address);

    return 4;
}

static unsigned op_ld_imm16mem_a(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t address = sm83_fetch16(sm83);
    bus_write(sm83->bus, BUS_CALLER_SM83, address, sm83->registers.a);

    return 4;
}

static unsigned op_ld_sp_imm16(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t address = sm83_fetch16(sm83);
    sm83->registers.sp = address;

    return 3;
}

static unsigned op_ldh_a_cmem(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_read_r8(sm83, SM83_R8_C);
    uint8_t value = bus_read(sm83->bus, BUS_CALLER_SM83, address);

    sm83->registers.a = value;

    return 2;
}

static unsigned op_ldh_cmem_a(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_read_r8(sm83, SM83_R8_C);
    bus_write(sm83->bus, BUS_CALLER_SM83, address, sm83->registers.a);

    return 2;
}

static unsigned op_ldh_a_imm8mem(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 | sm83_fetch8(sm83);
    uint8_t value = bus_read(sm83->bus, BUS_CALLER_SM83, address);

    sm83->registers.a = value;

    return 3;
}

static unsigned op_ldh_imm8mem_a(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_fetch8(sm83);
    bus_write(sm83->bus, BUS_CALLER_SM83, address, sm83->registers.a);

    return 3;
}


/* 16-bit load instruction definitions */

static unsigned op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint16_t value = sm83_fetch16(sm83);
    sm83_write_r16(sm83, pair, value);

    return 3;
}

static unsigned op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t address = sm83_fetch16(sm83); 
    uint16_t sp = sm83->registers.sp;

    bus_write(sm83->bus, BUS_CALLER_SM83, address, (uint8_t)(sp & 0xFF));
    address++;
    bus_write(sm83->bus, BUS_CALLER_SM83, address, (uint8_t)(sp >> 8));

    return 5;
}

static unsigned op_ld_sp_hl(sm83_t* sm83, uint8_t opcode) {
    uint16_t value = sm83_read_r16(sm83, SM83_R16_HL);
    sm83_write_r16(sm83, SM83_R16_SP, value);

    return 2;
}

static unsigned op_pop_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint16_t value = sm83_stack_pop_r16(sm83); 
    sm83_write_r16stk(sm83, pair, value);

    return 3;
}

static unsigned op_push_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);
    uint16_t value = sm83_read_r16stk(sm83, pair);

    sm83_stack_push_r16(sm83, value);

    return 4;
}

static unsigned op_ld_hl_spe8(sm83_t* sm83, uint8_t opcode) {
    int8_t offset = (int8_t)sm83_fetch8(sm83);
    uint16_t value = alu_add_sp_e8(sm83, sm83->registers.sp, offset);

    sm83_write_r16(sm83, SM83_R16_HL, value);

    return 3;
}


/* 8-bit arithmetic and logical instruction definitions */

static unsigned op_alu_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t lhs = sm83->registers.a;
    uint8_t rhs = sm83_read_r8(sm83, OPCODE_Z(opcode));
    sm83_alu_op_t operation = OPCODE_Y(opcode);

    uint8_t result = alu_execute(sm83, operation, lhs, rhs);
    if (operation != SM83_ALU_CP) {
        sm83->registers.a = result;
    }

    return OPCODE_Z(opcode) == SM83_R8_HLMEM ? 2 : 1;
}

static unsigned op_alu_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t lhs = sm83->registers.a;
    uint8_t rhs = sm83_fetch8(sm83);
    sm83_alu_op_t operation = OPCODE_Y(opcode);

    uint8_t result = alu_execute(sm83, operation, lhs, rhs);
    if (operation != SM83_ALU_CP) {
        sm83->registers.a = result;
    }

    return 2;
}

static unsigned op_inc_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t operand = OPCODE_Y(opcode);
    uint8_t value = sm83_read_r8(sm83, operand);
    uint8_t result = value + 1;

    bool half_carry = (value & 0x0F) == 0;

    uint8_t flags = 0;

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (half_carry) flags |= SM83_FLAG_H_MASK;

    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK, flags);
    sm83_write_r8(sm83, operand, result);

    return operand == SM83_R8_HLMEM ? 3 : 1;
}

static unsigned op_dec_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t operand = OPCODE_Y(opcode);
    uint8_t value = sm83_read_r8(sm83, operand);
    uint8_t result = (uint8_t)(value - 1);

    bool half_carry = (value & 0x0F) == 0;

    uint8_t flags = 0;

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    flags |= SM83_FLAG_N_MASK;
    if (half_carry) flags |= SM83_FLAG_H_MASK;

    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK, flags);
    sm83_write_r8(sm83, operand, result);

    return operand == SM83_R8_HLMEM ? 3 : 1;
}

static unsigned op_ccf(sm83_t* sm83, uint8_t opcode) {
    bool carry = sm83_get_flag(sm83, SM83_FLAG_C_MASK);

    sm83_update_flags(sm83, SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, carry ? 0 : SM83_FLAG_C_MASK);

    return 1;
}

static unsigned op_scf(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83_update_flags(sm83, SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, SM83_FLAG_C_MASK);

    return 1;
}

static unsigned op_daa(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t a = sm83->registers.a;
    bool n = sm83_get_flag(sm83, SM83_FLAG_N_MASK);
    bool h = sm83_get_flag(sm83, SM83_FLAG_H_MASK);
    bool c = sm83_get_flag(sm83, SM83_FLAG_C_MASK);

    uint8_t correction = 0;
    if (!n) {
        if(h || (a & 0x0F) > 0x09) correction |= 0x06;
        if(c || a > 0x99) {
            correction |= 0x60;
            c = true;
        }

        a = (uint8_t)(a + correction);
    } else {
        if(h) correction |= 0x06;
        if(c) correction |= 0x60;
    }

    sm83->registers.a = a;

    uint8_t flags = 0;

    if (a == 0) flags |= SM83_FLAG_Z_MASK;
    if (c) flags |= SM83_FLAG_C_MASK;

    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return 1;
}

static unsigned op_cpl(sm83_t* sm83, uint8_t opcode) {
    sm83->registers.a = (uint8_t)~sm83->registers.a;

    sm83_update_flags(sm83, SM83_FLAG_N_MASK | SM83_FLAG_H_MASK, SM83_FLAG_N_MASK | SM83_FLAG_H_MASK);

    return 1;
}


/* 16-bit arithmetic instruction definitions */

static unsigned op_inc_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t operand = OPCODE_P(opcode);
    uint16_t value = sm83_read_r16(sm83, operand);
    uint16_t result = value + 1;

    sm83_write_r16(sm83, operand, result);

    return 2;
}

static unsigned op_dec_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t operand = OPCODE_P(opcode);
    uint16_t value = sm83_read_r16(sm83, operand);
    uint16_t result = value - 1;

    sm83_write_r16(sm83, operand, result);

    return 2;
}

static unsigned op_add_hl_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint16_t lhs = sm83_read_r16(sm83, SM83_R16_HL);
    uint16_t rhs = sm83_read_r16(sm83, pair);

    uint16_t result = alu_add16(sm83, lhs, rhs);
    sm83_write_r16(sm83, SM83_R16_HL, result);

    return 2;
}

static unsigned op_add_sp_e8(sm83_t* sm83, uint8_t opcode) {
    int8_t offset = sm83_fetch8(sm83);
    uint16_t sp = sm83->registers.sp;

    uint16_t result = alu_add_sp_e8(sm83, sp, offset);
    sm83->registers.sp = result;

    return 4;
}

/* Rotate, shift, and bit operation instruction definitions */

static unsigned op_rlca(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool carry = (value & 0x80) != 0;

    uint8_t result = (uint8_t)((value << 1) | (carry ? 1 : 0));

    uint8_t flags = carry ? SM83_FLAG_C_MASK : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return 1;
}

static unsigned op_rrca(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (carry ? 0x80 : 0x00));

    uint8_t flags = carry ? SM83_FLAG_C_MASK : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return 1;
}

static unsigned op_rla(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C_MASK);
    bool carry = (value & 0x80) != 0;

    uint8_t result = (uint8_t)((value << 1) | (old_carry ? 1 : 0));
   
    uint8_t flags = carry ? SM83_FLAG_C_MASK : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return 1;
}

static unsigned op_rra(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C_MASK);
    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (old_carry ? 0x80 : 0x00));

    uint8_t flags = carry ? SM83_FLAG_C_MASK : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return 1;
}

static unsigned op_rlc_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool carry = (value & 0x80) != 0;

    uint8_t result = (uint8_t)((value << 1) | (carry ? 1 : 0));

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_rrc_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (carry ? 0x80 : 0x00));

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_rl_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C_MASK);
    bool carry = (value & 0x80) != 0;

    uint8_t result = (uint8_t)((value << 1) | (old_carry ? 1 : 0));
   
    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_rr_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C_MASK);
    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (old_carry ? 0x80 : 0x00));

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_sla_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool carry = (value & 0x80) != 0;

    uint8_t result = value << 1;

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);
    
    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_sra_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool carry = (value & 0x01) != 0;

    uint8_t result = (value & 0x80) | (value >> 1);

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_swap_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    uint8_t result = (value << 4) | (value >> 4);
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, result == 0 ? SM83_FLAG_Z_MASK : 0);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_srl_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    bool carry = (value & 0x01) != 0;

    uint8_t result = value >> 1;

    uint8_t flags = 0; 

    if (result == 0) flags |= SM83_FLAG_Z_MASK;
    if (carry) flags |= SM83_FLAG_C_MASK;
    
    sm83_write_r8(sm83, index, result);
    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_bit_b3_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    uint8_t bit = OPCODE_Y(opcode);
    bool is_set = (value & (1u << bit)) != 0;

    uint8_t flags = 0;

    if (!is_set) flags |= SM83_FLAG_Z_MASK;
    flags |= SM83_FLAG_H_MASK;

    sm83_update_flags(sm83, SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK, flags);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_res_b3_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    uint8_t bit = OPCODE_Y(opcode);
    
    value &= ~(1u << bit);
    sm83_write_r8(sm83, index, value);

    return index == SM83_R8_HLMEM ? 4 : 2;
}

static unsigned op_set_b3_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = OPCODE_Z(opcode);
    uint8_t value = sm83_read_r8(sm83, index);

    uint8_t bit = OPCODE_Y(opcode);
    
    value |= (1u << bit);
    sm83_write_r8(sm83, index, value);

    return index == SM83_R8_HLMEM ? 4 : 2;
}


/* Control flow instruction definitions */

static unsigned op_jp_imm16(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t immediate = sm83_fetch16(sm83);
    
    sm83->registers.pc = immediate;

    return 4;
}

static unsigned op_jp_hl(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = sm83_read_r16(sm83, SM83_R16_HL);

    sm83->registers.pc = address;

    return 1;
}

static unsigned op_jp_cc_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = !sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_jp_imm16(sm83, opcode);
    }

    (void)sm83_fetch16(sm83);
    
    return 3;
}

static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    int8_t offset = sm83_fetch8(sm83);
    sm83->registers.pc += offset;

    return 3;
}

static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = !sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_jr_imm8(sm83, opcode);
    }

    (void)sm83_fetch8(sm83);

    return 2;
}

static unsigned op_call_imm16(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = sm83_fetch16(sm83);

    sm83_stack_push_r16(sm83, sm83->registers.pc);
    sm83->registers.pc = address;

    return 6;  
}

static unsigned op_call_cc_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = !sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_call_imm16(sm83, opcode);
    }

    return 3;
}

static unsigned op_ret(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->registers.pc = sm83_stack_pop_r16(sm83);;

    return 4;
}

static unsigned op_ret_cc(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = !sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_ret(sm83, opcode) + 1;
    }

    return 2;
}

static unsigned op_reti(sm83_t* sm83, uint8_t opcode) {
    (void)op_ei(sm83, opcode);
    (void)op_ret(sm83, opcode);

    return 4;
}

static unsigned op_rst(sm83_t* sm83, uint8_t opcode) {
    uint16_t vector = (uint16_t)OPCODE_Y(opcode) << 3;

    sm83_stack_push_r16(sm83, sm83->registers.pc);
    
    sm83->registers.pc = vector;

    return 4;
}

/* Miscellaneous instruction definitions */

static unsigned op_nop(sm83_t* sm83, uint8_t opcode) {
    (void)sm83;
    (void)opcode;

    return 1;
}

static unsigned op_di(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->ime = false;

    return 1;
}

/* Enables interrupts. The IME flag should only be set after the next instruction. */
static unsigned op_ei(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->ime = true;

    return 1;
}

static unsigned op_halt(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->halted = false;

    return 1;
}

static unsigned op_stop(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;
    (void)sm83_fetch8(sm83);

    sm83->stopped = true; 

    return 1;
}

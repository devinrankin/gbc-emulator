#include "sm83.h"
#include "sm83_opcodes.h"
#include "sm83_alu.h"

#define INS(fn, len, family) { (fn), (len), (family) }

/* 8-bit load instruction prototypes */
static unsigned op_ld_r8_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_a_imm16mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_imm16mem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_cmem_a(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_a_cmem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_a_imm8mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ldh_imm8mem_a(sm83_t* sm83, uint8_t opcode);

/* 16-bit load instruction prototypes */
static unsigned op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_sp_hl(sm83_t* sm83, uint8_t opcode);
static unsigned op_push_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_pop_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_hl_spe8(sm83_t* sm83, uint8_t opcode);

/* 8-bit arithmetic and logical instruction prototypes */
static unsigned op_alu_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_alu_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_inc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_dec_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ccf(sm83_t* sm83, uint8_t opcode);
static unsigned op_scf(sm83_t* sm83, uint8_t opcode);
static unsigned op_daa(sm83_t* sm83, uint8_t opcode);
static unsigned op_cpl(sm83_t* sm83, uint8_t opcode);

/* 16-bit arithmetic instruction prototypes */
static unsigned op_inc_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_dec_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_add_hl_r16(sm83_t* sm83, uint8_t opcode);
static unsigned op_add_sp_e8(sm83_t* sm83, uint8_t opcode);

/* Rotate, shift, and bit operation instruction prototypes */
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

/* Control flow instruction prototypes */
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

/* Miscellaneous instruction prototypes */
static unsigned op_halt(sm83_t* sm83, uint8_t opcode);
static unsigned op_stop(sm83_t* sm83, uint8_t opcode);
static unsigned op_di(sm83_t* sm83, uint8_t opcode);
static unsigned op_ei(sm83_t* sm83, uint8_t opcode);
static unsigned op_nop(sm83_t* sm83, uint8_t opcode);

// static unsigned op_noimpl(sm83_t* sm83, uint8_t opcode);

/* Stack manipulation handler prototypes */
static uint8_t stack_pop(sm83_t* sm83);
static void stack_push(sm83_t* sm83, uint8_t value);


/* Opcode dispatch table */

const sm83_instruction_t sm83_opcode_table[256] = {
    [0x00] = INS(op_nop, 1, "NOP"),
    [0x01] = INS(op_ld_r16_imm16, 3, "LD"),
    [0x02] = INS(op_ld_r16mem_a, 1, "LD"),
    [0x03] = INS(op_inc_r16, 1, "INC"),
    [0x04] = INS(op_inc_r8, 1, "INC"),
    [0x05] = INS(op_dec_r8, 1, "DEC"),
    [0x06] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x07] = INS(op_rlca, 1, "RLCA"),
    [0x08] = INS(op_ld_imm16mem_sp, 3, "LD"),
    [0x09] = INS(op_add_hl_r16, 1, "ADD"),
    [0x0A] = INS(op_ld_a_r16mem, 1, "LD"),
    [0x0B] = INS(op_dec_r16, 1, "DEC"),
    [0x0C] = INS(op_inc_r8, 1, "INC"),
    [0x0D] = INS(op_dec_r8, 1, "DEC"),
    [0x0E] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x0F] = INS(op_rrca, 1, "RRCA"),

    [0x10] = INS(op_stop, 2, "STOP"),
    [0x11] = INS(op_ld_r16_imm16, 3, "LD"),
    [0x12] = INS(op_ld_r16mem_a, 1, "LD"),
    [0x13] = INS(op_inc_r16, 1, "INC"),
    [0x14] = INS(op_inc_r8, 1, "INC"),
    [0x15] = INS(op_dec_r8, 1, "DEC"),
    [0x16] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x17] = INS(op_rla, 1, "RLA"),
    [0x18] = INS(op_jr_imm8, 2, "JR"),
    [0x19] = INS(op_add_hl_r16, 1, "ADD"),
    [0x1A] = INS(op_ld_a_r16mem, 1, "LD"),
    [0x1B] = INS(op_dec_r16, 1, "DEC"),
    [0x1C] = INS(op_inc_r8, 1, "INC"),
    [0x1D] = INS(op_dec_r8, 1, "DEC"),
    [0x1E] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x1F] = INS(op_rra, 1, "RRA"),

    [0x20] = INS(op_jr_cc_imm8, 2, "JR"),
    [0x21] = INS(op_ld_r16_imm16, 3, "LD"),
    [0x22] = INS(op_ld_r16mem_a, 1, "LD"),
    [0x23] = INS(op_inc_r16, 1, "INC"),
    [0x24] = INS(op_inc_r8, 1, "INC"),
    [0x25] = INS(op_dec_r8, 1, "DEC"),
    [0x26] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x27] = INS(op_daa, 1, "DAA"),
    [0x28] = INS(op_jr_cc_imm8, 2, "JR"),
    [0x29] = INS(op_add_hl_r16, 1, "ADD"),
    [0x2A] = INS(op_ld_a_r16mem, 1, "LD"),
    [0x2B] = INS(op_dec_r16, 1, "DEC"),
    [0x2C] = INS(op_inc_r8, 1, "INC"),
    [0x2D] = INS(op_dec_r8, 1, "DEC"),
    [0x2E] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x2F] = INS(op_cpl, 1, "CPL"),

    [0x30] = INS(op_jr_cc_imm8, 2, "JR"),
    [0x31] = INS(op_ld_r16_imm16, 3, "LD"),
    [0x32] = INS(op_ld_r16mem_a, 1, "LD"),
    [0x33] = INS(op_inc_r16, 1, "INC"),
    [0x34] = INS(op_inc_r8, 1, "INC"),
    [0x35] = INS(op_dec_r8, 1, "DEC"),
    [0x36] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x37] = INS(op_scf, 1, "SCF"),
    [0x38] = INS(op_jr_cc_imm8, 2, "JR"),
    [0x39] = INS(op_add_hl_r16, 1, "ADD"),
    [0x3A] = INS(op_ld_a_r16mem, 1, "LD"),
    [0x3B] = INS(op_dec_r16, 1, "DEC"),
    [0x3C] = INS(op_inc_r8, 1, "INC"),
    [0x3D] = INS(op_dec_r8, 1, "DEC"),
    [0x3E] = INS(op_ld_r8_imm8, 2, "LD"),
    [0x3F] = INS(op_ccf, 1, "CCF"),

    [0x40 ... 0x75] = INS(op_ld_r8_r8, 1, "LD"),
    [0x76] = INS(op_halt, 1, "HALT"),
    [0x77 ... 0x7F] = INS(op_ld_r8_r8, 1, "LD"),

    [0x80 ... 0xBF] = INS(op_alu_r8, 1, "ALU"),

    [0xC0] = INS(op_ret_cc, 1, "RET"),
    [0xC1] = INS(op_pop_r16, 1, "POP"),
    [0xC2] = INS(op_jp_cc_imm16, 3, "JP"),
    [0xC3] = INS(op_jp_imm16, 3, "JP"),
    [0xC4] = INS(op_call_cc_imm16, 3, "CALL"),
    [0xC5] = INS(op_push_r16, 1, "PUSH"),
    [0xC6] = INS(op_alu_imm8, 2, "ALU"),
    [0xC7] = INS(op_rst, 1, "RST"),
    [0xC8] = INS(op_ret_cc, 1, "RET"),
    [0xC9] = INS(op_ret, 1, "RET"),
    [0xCA] = INS(op_jp_cc_imm16, 3, "JP"),
    [0xCB] = INS(NULL, 1, "PREFIX"),
    [0xCC] = INS(op_call_cc_imm16, 3, "CALL"),
    [0xCD] = INS(op_call_imm16, 3, "CALL"),
    [0xCE] = INS(op_alu_imm8, 2, "ALU"),
    [0xCF] = INS(op_rst, 1, "RST"),

    [0xD0] = INS(op_ret_cc, 1, "RET"),
    [0xD1] = INS(op_pop_r16, 1, "POP"),
    [0xD2] = INS(op_jp_cc_imm16, 3, "JP"),
    [0xD4] = INS(op_call_cc_imm16, 3, "CALL"),
    [0xD5] = INS(op_push_r16, 1, "PUSH"),
    [0xD6] = INS(op_alu_imm8, 2, "ALU"),
    [0xD7] = INS(op_rst, 1, "RST"),
    [0xD8] = INS(op_ret_cc, 1, "RET"),
    [0xD9] = INS(op_reti, 1, "RETI"),
    [0xDA] = INS(op_jp_cc_imm16, 3, "JP"),
    [0xDC] = INS(op_call_cc_imm16, 3, "CALL"),
    [0xDE] = INS(op_alu_imm8, 2, "ALU"),
    [0xDF] = INS(op_rst, 1, "RST"),

    [0xE0] = INS(op_ldh_imm8mem_a, 2, "LDH"),
    [0xE1] = INS(op_pop_r16, 1, "POP"),
    [0xE2] = INS(op_ldh_cmem_a, 1, "LDH"),
    [0xE5] = INS(op_push_r16, 1, "PUSH"),
    [0xE6] = INS(op_alu_imm8, 2, "ALU"),
    [0xE7] = INS(op_rst, 1, "RST"),
    [0xE8] = INS(op_add_sp_e8, 2, "ADD"),
    [0xE9] = INS(op_jp_hl, 1, "JP"),
    [0xEA] = INS(op_ld_imm16mem_a, 3, "LD"),
    [0xEE] = INS(op_alu_imm8, 2, "ALU"),
    [0xEF] = INS(op_rst, 1, "RST"),

    [0xF0] = INS(op_ldh_a_imm8mem, 2, "LDH"),
    [0xF1] = INS(op_pop_r16, 1, "POP"),
    [0xF2] = INS(op_ldh_a_cmem, 1, "LDH"),
    [0xF3] = INS(op_di, 1, "DI"),
    [0xF5] = INS(op_push_r16, 1, "PUSH"),
    [0xF6] = INS(op_alu_imm8, 2, "ALU"),
    [0xF7] = INS(op_rst, 1, "RST"),
    [0xF8] = INS(op_ld_hl_spe8, 2, "LD"),
    [0xF9] = INS(op_ld_sp_hl, 1, "LD"),
    [0xFA] = INS(op_ld_a_imm16mem, 3, "LD"),
    [0xFB] = INS(op_ei, 1, "EI"),
    [0xFE] = INS(op_alu_imm8, 2, "ALU"),
    [0xFF] = INS(op_rst, 1, "RST")
};

const sm83_instruction_t sm83_opcode_table_cb[256] = {
    [0x00 ... 0x07] = INS(op_rlc_r8, 1, "RLC"),
    [0x08 ... 0x0F] = INS(op_rrc_r8, 1, "RRC"),
    [0x10 ... 0x17] = INS(op_rl_r8, 1, "RL"),
    [0x18 ... 0x1F] = INS(op_rr_r8, 1, "RR"),
    [0x20 ... 0x27] = INS(op_sla_r8, 1, "SLA"),
    [0x28 ... 0x2F] = INS(op_sra_r8, 1, "SRA"),
    [0x30 ... 0x37] = INS(op_swap_r8, 1, "SWAP"),
    [0x38 ... 0x3F] = INS(op_srl_r8, 1, "SRL"),

    [0x40 ... 0x47] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x48 ... 0x4F] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x50 ... 0x57] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x58 ... 0x5F] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x60 ... 0x67] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x68 ... 0x6F] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x70 ... 0x77] = INS(op_bit_b3_r8, 1, "BIT"),
    [0x78 ... 0x7F] = INS(op_bit_b3_r8, 1, "BIT"),

    [0x80 ... 0x87] = INS(op_res_b3_r8, 1, "RES"),
    [0x88 ... 0x8F] = INS(op_res_b3_r8, 1, "RES"),
    [0x90 ... 0x97] = INS(op_res_b3_r8, 1, "RES"),
    [0x98 ... 0x9F] = INS(op_res_b3_r8, 1, "RES"),
    [0xA0 ... 0xA7] = INS(op_res_b3_r8, 1, "RES"),
    [0xA8 ... 0xAF] = INS(op_res_b3_r8, 1, "RES"),
    [0xB0 ... 0xB7] = INS(op_res_b3_r8, 1, "RES"),
    [0xB8 ... 0xBF] = INS(op_res_b3_r8, 1, "RES"),

    [0xC0 ... 0xC7] = INS(op_set_b3_r8, 1, "SET"),
    [0xC8 ... 0xCF] = INS(op_set_b3_r8, 1, "SET"),
    [0xD0 ... 0xD7] = INS(op_set_b3_r8, 1, "SET"),
    [0xD8 ... 0xDF] = INS(op_set_b3_r8, 1, "SET"),
    [0xE0 ... 0xE7] = INS(op_set_b3_r8, 1, "SET"),
    [0xE8 ... 0xEF] = INS(op_set_b3_r8, 1, "SET"),
    [0xF0 ... 0xF7] = INS(op_set_b3_r8, 1, "SET"),
    [0xF8 ... 0xFF] = INS(op_set_b3_r8, 1, "SET")
};

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
    bus_write8(sm83->bus, address, sm83->registers.a);

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
    sm83->registers.a = bus_read8(sm83->bus, address);

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
    sm83->registers.a = bus_read8(sm83->bus, address);

    return 4;
}

static unsigned op_ld_imm16mem_a(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t address = sm83_fetch16(sm83);
    bus_write8(sm83->bus, address, sm83->registers.a);

    return 4;
}

static unsigned op_ldh_a_cmem(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_read_r8(sm83, SM83_R8_C);
    uint8_t value = bus_read8(sm83->bus, address);

    sm83->registers.a = value;

    return 2;
}

static unsigned op_ldh_cmem_a(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_read_r8(sm83, SM83_R8_C);
    bus_write8(sm83->bus, address, sm83->registers.a);

    return 2;
}

static unsigned op_ldh_a_imm8mem(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_fetch8(sm83);
    uint8_t value = bus_read8(sm83->bus, address);

    sm83->registers.a = value;

    return 3;
}

static unsigned op_ldh_imm8mem_a(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = 0xFF00 & sm83_fetch8(sm83);
    bus_write8(sm83->bus, address, sm83->registers.a);

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

    bus_write8(sm83->bus, address, (uint8_t)(sp & 0xFF));
    address++;
    bus_write8(sm83->bus, address, (uint8_t)(sp >> 8));

    return 5;
}

static unsigned op_ld_sp_hl(sm83_t* sm83, uint8_t opcode) {
    uint16_t value = sm83_read_r16(sm83, SM83_R16_HL);
    sm83_write_r16(sm83, SM83_R16_SP, value);

    return 2;
}

static unsigned op_pop_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint8_t low = stack_pop(sm83);
    uint8_t high = stack_pop(sm83);
    
    uint16_t value = ((uint16_t)high << 8) | low;
    sm83_write_r16stk(sm83, pair, value);

    return 3;
}

static unsigned op_push_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);
    uint16_t value = sm83_read_r16stk(sm83, pair);

    uint8_t high = (uint8_t)(value >> 8);
    uint8_t low = (uint8_t)(value & 0xFF); 

    stack_push(sm83, high);
    stack_push(sm83, low);

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
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_jp_imm16(sm83, opcode);
    }

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
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_jr_imm8(sm83, opcode);
    }

    return 2;
}

static unsigned op_call_imm16(sm83_t* sm83, uint8_t opcode) {
    uint16_t address = sm83_fetch16(sm83);

    uint8_t high = (uint8_t)(address >> 8);
    uint8_t low = (uint8_t)(address & 0xFF); 

    stack_push(sm83, high);
    stack_push(sm83, low);

    sm83->registers.pc = address;

    return 6;  
}

static unsigned op_call_cc_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C_MASK); break;
    }

    if (is_cond) {
        return op_call_imm16(sm83, opcode);
    }

    return 3;
}

static unsigned op_ret(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t low = stack_pop(sm83);
    uint8_t high = stack_pop(sm83);

    sm83->registers.pc = ((uint16_t)high << 8) | low;

    return 4;
}

static unsigned op_ret_cc(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z_MASK); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N_MASK); break;
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
    uint16_t pc = sm83->registers.pc;

    uint8_t high = (uint8_t)(pc >> 8);
    uint8_t low = (uint8_t)(pc & 0xFF); 

    stack_push(sm83, high);
    stack_push(sm83, low);

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

    sm83->halted = true;

    return 1;
}

static unsigned op_stop(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;
    (void)sm83_fetch8(sm83);

    sm83->stopped = true; 

    return 1;
}


/* Stack manipulation function definitions */
static void stack_push(sm83_t* sm83, uint8_t value) {
    sm83->registers.sp--;
    bus_write8(sm83->bus, sm83->registers.sp, value);
}

static uint8_t stack_pop(sm83_t* sm83) {
    uint8_t value = bus_read8(sm83->bus, sm83->registers.sp);
    sm83->registers.sp++;

    return value;
}

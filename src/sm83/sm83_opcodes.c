#include "sm83.h"
#include "sm83_opcodes.h"
#include "sm83_alu.h"

/* 8-bit load instruction prototypes */
static unsigned op_ld_r8_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode);

/* 16-bit load instruction prototypes */
static unsigned op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode);

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

/* Rotate, shift, and bit operation instruction prototypes */
static unsigned op_rlca(sm83_t* sm83, uint8_t opcode);
static unsigned op_rla(sm83_t* sm83, uint8_t opcode);

/* Control flow instruction prototypes */
static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode);

/* Miscellaneous instruction prototypes */
static unsigned op_halt(sm83_t* sm83, uint8_t opcode);
static unsigned op_stop(sm83_t* sm83, uint8_t opcode);
static unsigned op_di(sm83_t* sm83, uint8_t opcode);
static unsigned op_ei(sm83_t* sm83, uint8_t opcode);
static unsigned op_nop(sm83_t* sm83, uint8_t opcode);

static unsigned op_noimpl(sm83_t* sm83, uint8_t opcode);


/* Opcode dispatch table */

const sm83_instruction_t sm83_opcode_table[256] = {
};
/* 8-bit load instruction definitions */

static unsigned op_ld_r8_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = OPCODE_Y(opcode);
    uint8_t src = OPCODE_Z(opcode);

    if(dst == 6 && src == 6) {
        sm83->halted = true;
        sm83->state = SM83_STATE_HALTED;
    }

    uint8_t value = sm83_read_r8(sm83, src);
    sm83_write_r8(sm83, dst, value);

    return 1;
}

static unsigned op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = OPCODE_Y(opcode);

    uint8_t value = bus_read8(sm83->bus, sm83->registers.pc + 1);

    sm83_write_r8(sm83, dst, value);

    return 2;
}


/* 16-bit load instruction definitions */

static unsigned op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint8_t value = bus_read16(sm83->bus, sm83->registers.pc);
    sm83_write_r16(sm83, pair, value);

    return 3;
}

static unsigned op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint8_t address = 0;
    switch(pair) {
        case 0: address = (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c; break;
        case 1: address = (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e; break;
        case 2: 
            address = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
            sm83_write_r16(sm83, 2, address + 1);
            break;
        case 3:
            address = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
            sm83_write_r16(sm83, 2, address - 1);
            break;               
    }
    bus_write8(sm83->bus, address, sm83_read_r8(sm83, SM83_REGISTER_A));

    return 2;
}

static unsigned op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t pc = sm83->registers.pc;

    uint8_t low = bus_read8(sm83->bus, pc + 1);
    uint8_t high = bus_read8(sm83->bus, pc + 2);

    uint16_t addressess = (uint16_t)low | ((uint16_t)high << 8);

    uint16_t sp = sm83->registers.sp;

    bus_write8(sm83->bus, addressess, (uint8_t)(sp & 0xFF));
    bus_write8(sm83->bus, addressess + 1, (uint8_t)(sp >> 8));

    return 5;
}

static unsigned op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = OPCODE_P(opcode);

    uint8_t address = 0;
    switch(pair) {
        case 0: address = (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c; break;
        case 1: address = (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e; break;
        case 2: 
                address = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, SM83_REGISTER_HL, address + 1);
                break;
        case 3:
                address = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, SM83_REGISTER_HL, address - 1);
                break;               
    }
    sm83_write_r8(sm83, SM83_REGISTER_A, bus_read8(sm83->bus, address));

    return 2;
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

    return OPCODE_Z(opcode) == 6 ? 2 : 1;
}

static unsigned op_alu_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t lhs = sm83->registers.a;
    uint8_t rhs = bus_read8(sm83->bus, sm83->registers.pc + 1);
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

    if(result == 0) flags |= SM83_FLAG_Z;
    if(half_carry) flags |= SM83_FLAG_H;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H, flags);
    sm83_write_r8(sm83, operand, result);

    return 1;
}

static unsigned op_dec_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t operand = OPCODE_Y(opcode);
    uint8_t value = sm83_read_r8(sm83, operand);
    uint8_t result = (uint8_t)(value - 1);

    bool half_carry = (value & 0x0F) == 0;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if(half_carry) flags |= SM83_FLAG_H;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H, flags);
    sm83_write_r8(sm83, operand, result);

    return 1;
}

static unsigned op_daa(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t a = sm83->registers.a;
    bool n = sm83_get_flag(sm83, SM83_FLAG_N);
    bool h = sm83_get_flag(sm83, SM83_FLAG_H);
    bool c = sm83_get_flag(sm83, SM83_FLAG_C);

    uint8_t correction = 0;
    if(!n) {
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

    if(a == 0) flags |= SM83_FLAG_Z;
    if(c) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}

static unsigned op_scf(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);

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
    
    uint16_t hl = sm83_read_r16(sm83, SM83_REGISTER_HL);
    uint16_t value = sm83_read_r16(sm83, pair);

    uint32_t result = hl + value;
    uint16_t result16 = (uint16_t)result;

    bool half_carry = ((hl & 0xFFF) + (value & 0xFFF) > 0xFFF);
    bool carry = result > 0xFFFF;

    uint8_t flags;

    if(half_carry) flags |= SM83_FLAG_H;
    if(carry) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    sm83_write_r16(sm83, SM83_REGISTER_HL, result16);

    return 2;
}


/* Rotate, shift, and bit operation instruction definitions */

static unsigned op_rlca(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C);

    bool carry = (sm83->registers.a & 0x80) != 0;
    sm83->registers.a = (uint8_t)((sm83->registers.a << 1) | (old_carry ? 1 : 0));
    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);

    return 1;
}

static unsigned op_rla(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    bool carry = (sm83->registers.a & 0x80) != 0;
    sm83->registers.a = (uint8_t)((sm83->registers.a << 1) | (carry ? 1 : 0));
    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);

    return 1;
}


/* Control flow instruction definitions */

static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t old_pc = sm83->registers.pc;
    uint8_t offset = bus_read8(sm83->bus, old_pc + 1);

    sm83->registers.pc = old_pc + offset;

    return 3;
}
static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond_type = ((opcode >> 3) & 0x03) << 8;

    bool cond;
    switch(cond_type) {
        case 0: cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
    }

    if(cond) {
        return op_jr_imm8(sm83, opcode);
    }

    return 2;
}


/* Miscellaneous instruction definitions */

static unsigned op_nop(sm83_t* sm83, uint8_t opcode) {
    (void)sm83;
    (void)opcode;

    return 1;
}

static unsigned op_halt(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->halted = true;

    return 1;
}

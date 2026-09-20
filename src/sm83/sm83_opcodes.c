#include "sm83.h"
#include "sm83_opcodes.h"
#include "sm83_alu.h"

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
static unsigned op_rl_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rr_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rlc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned op_rrc_r8(sm83_t* sm83, uint8_t opcode);

/* Control flow instruction prototypes */
static unsigned op_jp_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_jp_cc_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned op_call_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_call_cc_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned op_ret(sm83_t* sm83, uint8_t opcode);
static unsigned op_ret_cc(sm83_t* sm83, uint8_t opcode);
static unsigned op_reti(sm83_t* sm83, uint8_t opcode);
static unsigned op_rst_n8(sm83_t* sm83, uint8_t opcode);

/* Miscellaneous instruction prototypes */
static unsigned op_halt(sm83_t* sm83, uint8_t opcode);
static unsigned op_stop(sm83_t* sm83, uint8_t opcode);
static unsigned op_di(sm83_t* sm83, uint8_t opcode);
static unsigned op_ei(sm83_t* sm83, uint8_t opcode);
static unsigned op_nop(sm83_t* sm83, uint8_t opcode);

static unsigned op_noimpl(sm83_t* sm83, uint8_t opcode);

/* Stack manipulation handler prototypes */
static uint8_t stack_pop(sm83_t* sm83);
static void stack_push(sm83_t* sm83, uint8_t value);


/* Opcode dispatch table */

const sm83_instruction_t sm83_opcode_table[256] = {
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

    if (result == 0) flags |= SM83_FLAG_Z;
    if (half_carry) flags |= SM83_FLAG_H;

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

    if (result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if (half_carry) flags |= SM83_FLAG_H;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H, flags);
    sm83_write_r8(sm83, operand, result);

    return 1;
}

static unsigned op_ccf(sm83_t* sm83, uint8_t opcode) {
    bool carry = sm83_get_flag(sm83, SM83_FLAG_C);

    sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, carry ? 0 : SM83_FLAG_C);

    return 1;
}

static unsigned op_scf(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, SM83_FLAG_C);

    return 1;
}

static unsigned op_daa(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t a = sm83->registers.a;
    bool n = sm83_get_flag(sm83, SM83_FLAG_N);
    bool h = sm83_get_flag(sm83, SM83_FLAG_H);
    bool c = sm83_get_flag(sm83, SM83_FLAG_C);

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

    if (a == 0) flags |= SM83_FLAG_Z;
    if (c) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}

static unsigned op_cpl(sm83_t* sm83, uint8_t opcode) {
    sm83->registers.a = (uint8_t)~sm83->registers.a;

    sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_H, SM83_FLAG_N | SM83_FLAG_H);

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

    uint8_t flags = carry ? SM83_FLAG_C : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}

static unsigned op_rrca(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (carry ? 0x80 : 0x00));

    uint8_t flags = carry ? SM83_FLAG_C : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}

static unsigned op_rla(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C);
    bool carry = (value & 0x80) != 0;

    uint8_t result = (uint8_t)((value << 1) | (old_carry ? 1 : 0));
   
    uint8_t flags = carry ? SM83_FLAG_C : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}

static unsigned op_rra(sm83_t* sm83, uint8_t opcode) {
    uint8_t value = sm83->registers.a;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C);
    bool carry = (value & 0x01) != 0;

    uint8_t result = (uint8_t)((value >> 1) | (old_carry ? 0x80 : 0x00));

    uint8_t flags = carry ? SM83_FLAG_C : 0;
    
    sm83->registers.a = result;
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    return 1;
}


/* Control flow instruction definitions */

static unsigned op_jp_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t immediate = sm83_fetch8(sm83);
    sm83->registers.pc = immediate;

    return 4;
}

static unsigned op_jp_cc_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
    }

    if (is_cond) {
        return op_jp_imm8(sm83, opcode);
    }

    return 3;
}

static unsigned op_jr_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t old_pc = sm83->registers.pc;
    uint8_t offset = sm83_fetch8(sm83); 

    sm83->registers.pc = old_pc + offset;

    return 3;
}

static unsigned op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond = OPCODE_COND(opcode);

    bool is_cond;
    switch (cond) {
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
    }

    if (is_cond) {
        return op_jp_imm8(sm83, opcode);
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
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
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
        case 0: is_cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: is_cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: is_cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: is_cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
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
    (void)sm83;
    (void)opcode;

    /* TODO */
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

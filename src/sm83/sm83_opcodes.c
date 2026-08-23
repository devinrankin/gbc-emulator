#include "sm83.h"
#include "sm83_opcodes.h"

// Instruction handlers
static unsigned sm83_op_unimpl(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_nop(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_inc_r16(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_inc_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_dec_r16(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_dec_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_rlca(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_rla(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_imm16mem_sp(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_add_hl_r16(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_stop(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_jr_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_ld_r8_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_halt(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_add_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_add_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_adc_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_adc_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_sub_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_sub_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_sbc_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_sbc_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_and_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_and_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_or_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_or_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_xor_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_xor_a_imm8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_cp_a_r8(sm83_t* sm83, uint8_t opcode);
static unsigned sm83_op_cp_a_imm8(sm83_t* sm83, uint8_t opcode);

// ALU helpers
static void sm83_op_add_a(sm83_t* sm83, uint8_t opcode);
static void sm83_op_adc_a(sm83_t* sm83, uint8_t value);
static void sm83_op_sub_a(sm83_t* sm83, uint8_t value);
static void sm83_op_sbc_a(sm83_t* sm83, uint8_t value);
static void sm83_op_and_a(sm83_t* sm83, uint8_t value);
static void sm83_op_or_a(sm83_t* sm83, uint8_t value);
static void sm83_op_xor_a(sm83_t* sm83, uint8_t value);
static void sm83_op_cp_a(sm83_t* sm83, uint8_t value);

// Opcode dispatch table
const sm83_instruction_t sm83_opcode_table[256] = {
};


static unsigned sm83_op_nop(sm83_t* sm83, uint8_t opcode) {
    (void)sm83;
    (void)opcode;

    return 1;
}

static unsigned sm83_op_ld_r16_imm16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;

    uint8_t val = bus_read16(sm83->cartridge, sm83->registers.pc);
    sm83_write_r16(sm83, pair, val);
    
    return 3;
}

static unsigned sm83_op_ld_r16mem_a(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;

    uint8_t addr = 0;
    switch(pair) {
        case 0: addr = (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c; break;
        case 1: addr = (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e; break;
        case 2: 
                addr = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, 2, addr + 1);
                break;
        case 3:
                addr = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, 2, addr - 1);
                break;               
    }
    bus_write8(sm83->cartridge, addr, sm83_read_r8_or_hl(sm83, SM83_REGISTER_IDX_A));
    
    return 2;
}

static unsigned sm83_op_inc_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;
    uint16_t value = sm83_read_r16(sm83, pair);
    uint16_t result = value + 1;
    
    sm83_write_r16(sm83, pair, result);

    return 2;
}

static unsigned sm83_op_inc_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = (opcode >> 3) & 0x07;
    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    uint8_t result = value + 1;

    bool half_carry = (value & 0x0F) == 0;
    
    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    if(half_carry) flags |= SM83_FLAG_H;
    
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H, flags);
    sm83_write_r8_or_hl(sm83, index, result);

    return 1;
}

static unsigned sm83_op_dec_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;
    uint16_t value = sm83_read_r16(sm83, pair);
    uint16_t result = value - 1;
    
    sm83_write_r16(sm83, pair, result);

    return 2;
}

static unsigned sm83_op_dec_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = (opcode >> 3) & 0x07;
    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    uint8_t result = (uint8_t)(value - 1);
    
    bool half_carry = (value & 0x0F) == 0;

    uint8_t flags = 0;
    
    if(result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if(half_carry) flags |= SM83_FLAG_H;
    
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H, flags);
    sm83_write_r8_or_hl(sm83, index, result);

    return 1;
}

static unsigned sm83_op_ld_r8_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = (opcode >> 3) & 0x07;

    uint8_t val = bus_read8(sm83->cartridge, sm83->registers.pc + 1);

    sm83_write_r8_or_hl(sm83, dst, val);

    return 2;
}

static unsigned sm83_op_rlca(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    bool old_carry = sm83_get_flag(sm83, SM83_FLAG_C);
    
    bool carry = (sm83->registers.a & 0x80) != 0;
    sm83->registers.a = (uint8_t)((sm83->registers.a << 1) | (old_carry ? 1 : 0));
    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);

    return 1;
}

static unsigned sm83_op_rla(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    bool carry = (sm83->registers.a & 0x80) != 0;
    sm83->registers.a = (uint8_t)((sm83->registers.a << 1) | (carry ? 1 : 0));
    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);

    return 1;
}

static unsigned sm83_op_ld_imm16mem_sp(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t pc = sm83->registers.pc;

    uint8_t low = bus_read8(sm83->cartridge, pc + 1);
    uint8_t high = bus_read8(sm83->cartridge, pc + 2);

    uint16_t address = (uint16_t)low | ((uint16_t)high << 8);

    uint16_t sp = sm83->registers.sp;

    bus_write8(sm83->cartridge, address, (uint8_t)(sp & 0xFF));
    bus_write8(sm83->cartridge, address + 1, (uint8_t)(sp >> 8));

    return 5;
}

static unsigned sm83_op_add_hl_r16(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;
    
    uint16_t hl = sm83_read_r16(sm83, SM83_REGISTER_PAIR_HL);
    uint16_t value = sm83_read_r16(sm83, pair);
    
    uint32_t result = hl + value;
    uint16_t result16 = (uint16_t)result;

    bool half_carry = ((hl & 0xFFF) + (value & 0xFFF) > 0xFFF);
    bool carry = result > 0xFFFF;
    
    uint8_t flags;

    if(half_carry) flags |= SM83_FLAG_H;
    if(carry) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);

    sm83_write_r16(sm83, SM83_REGISTER_PAIR_HL, result16);

    return 2;
}

static unsigned sm83_op_ld_a_r16mem(sm83_t* sm83, uint8_t opcode) {
    uint8_t pair = (opcode >> 4) & 0x03;

    uint8_t addr = 0;
    switch(pair) {
        case 0: addr = (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c; break;
        case 1: addr = (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e; break;
        case 2: 
                addr = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, SM83_REGISTER_PAIR_HL, addr + 1);
                break;
        case 3:
                addr = (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
                sm83_write_r16(sm83, SM83_REGISTER_PAIR_HL, addr - 1);
                break;               
    }
    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, bus_read8(sm83->cartridge, addr));
    
    return 2;
}

static unsigned sm83_op_jr_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint16_t old_pc = sm83->registers.pc;
    uint8_t offset = bus_read8(sm83->cartridge, old_pc + 1);

    sm83->registers.pc = old_pc + offset;

    return 3;
}
static unsigned sm83_op_jr_cc_imm8(sm83_t* sm83, uint8_t opcode) {
    uint8_t cond_type = ((opcode >> 3) & 0x03) << 8;
    
    bool cond;
    switch(cond_type) {
        case 0: cond = sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 1: cond = sm83_get_flag(sm83, SM83_FLAG_Z); break;
        case 2: cond = !sm83_get_flag(sm83, SM83_FLAG_N); break;
        case 3: cond = sm83_get_flag(sm83, SM83_FLAG_C); break;
    }

    if(cond) {
        return sm83_op_jr_imm8(sm83, opcode);
    }

    return 2;
}

static unsigned sm83_op_daa(sm83_t* sm83, uint8_t opcode) {
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

static unsigned sm83_op_scf(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83_update_flags(sm83, SM83_FLAG_C, SM83_FLAG_C);
    
    return 1;
}

static unsigned sm83_op_ld_r8_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t dst = (opcode >> 3) & 0x07;
    uint8_t src = opcode & 0x07;
    if(dst == 6 && src == 6) {
        sm83->halted = true;
        sm83->state = SM83_STATE_HALTED;
    }

    uint8_t val = sm83_read_r8_or_hl(sm83, src);
    sm83_write_r8_or_hl(sm83, dst, val);

    return 1;
}

static unsigned sm83_op_halt(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    sm83->halted = true;

    return 1;
}

static void sm83_op_add_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
    uint16_t result = a + value;

    uint8_t flags = 0;

    if((uint8_t)result == 0) flags |= SM83_FLAG_Z;
    if((a & 0x0F) + (value & 0x0F) > 0x0F) flags |= SM83_FLAG_H;
    if(result > 0xFF) flags |= SM83_FLAG_C;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, (uint8_t)result);

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
}

static unsigned sm83_op_add_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;

    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_add_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_add_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    sm83_op_add_a(sm83, value);

    return 2;
}

static void sm83_op_adc_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
 
    uint8_t carry_in = sm83_get_flag(sm83, SM83_FLAG_C) ? 1 : 0;
    uint16_t result = a + value;
      
    uint8_t flags = 0;

    if((uint8_t)result == 0) flags |= SM83_FLAG_Z;
    if((a & 0x0F) + (value & 0x0F) > 0x0F) flags |= SM83_FLAG_H;
    if(result > 0xFF) flags |= SM83_FLAG_C;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, (uint8_t)result);

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
}


static unsigned sm83_op_adc_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;

    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_adc_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_adc_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    sm83_op_adc_a(sm83, value);
    
    return 2;
}


static void sm83_op_sub_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
    uint16_t result = a - value;

    uint8_t flags = 0;

    if((uint8_t)result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if((a & 0x0F) + (value & 0x0F) > 0x0F) flags |= SM83_FLAG_H;
    if(result > 0xFF) flags |= SM83_FLAG_C;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, (uint8_t)result);

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
}

static unsigned sm83_op_sub_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;
 
    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_sub_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_sub_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;
    
    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    sm83_op_sub_a(sm83, value);

    return 2;
}

static void sm83_op_sbc_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;

    uint8_t carry_in = sm83_get_flag(sm83, SM83_FLAG_C) ? 1 : 0;
    uint16_t subtrahend = (uint16_t)value + carry_in;
    
    uint16_t result = (uint16_t)a - subtrahend;

    uint8_t flags = 0;

    if((uint8_t)result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if((a & 0x0F) < (value & 0x0F) + carry_in) flags |= SM83_FLAG_H;
    if((uint16_t)a < subtrahend) flags |= SM83_FLAG_C;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, (uint8_t)result);
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
}

static unsigned sm83_op_sbc_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;

    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_sub_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_sbc_a_imm8(sm83_t* sm83, uint8_t opcode) {
   (void)opcode;
    
    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    uint8_t a = sm83->registers.a;

    sm83_op_sub_a(sm83, value);

    return 2;
}

static void sm83_op_and_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
    uint8_t result = value & a;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_H;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, result);
    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_H, flags);
}

static unsigned sm83_op_and_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;
    
    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_sub_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_and_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    sm83_op_and_a(sm83, value);

    return 2;
}

static void sm83_op_or_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
    uint8_t result = value | a;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, result);
    sm83_update_flags(sm83, SM83_FLAG_Z, flags);
}

static unsigned sm83_op_or_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;

    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_or_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_or_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1);
    sm83_op_or_a(sm83, value); 

    return 2;
}

static void sm83_op_xor_a(sm83_t* sm83, uint8_t value) {
    uint8_t a = sm83->registers.a;
    uint8_t result = value ^ a;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;

    sm83_write_r8_or_hl(sm83, SM83_REGISTER_IDX_A, result);
    sm83_update_flags(sm83, SM83_FLAG_Z, flags);

}

static unsigned sm83_op_xor_a_r8(sm83_t* sm83, uint8_t opcode) {
    uint8_t index = opcode & 0x07;

    uint8_t value = sm83_read_r8_or_hl(sm83, index);
    sm83_op_xor_a(sm83, value);

    return index == 6 ? 2 : 1;
}

static unsigned sm83_op_xor_a_imm8(sm83_t* sm83, uint8_t opcode) {
    (void)opcode;

    uint8_t value = bus_read8(sm83->cartridge, sm83->registers.pc + 1); 
    sm83_op_xor_a(sm83, value);

    return 2;
}


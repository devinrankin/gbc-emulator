#include "sm83.h"
#include "sm83_opcodes.h"

static uint32_t handle_interrupt(sm83_t* sm83);

void sm83_init(sm83_t* sm83, bus_t* bus) {
    *sm83 = (sm83_t) {
        .state = SM83_STATE_INIT,
        .halted = false,
        .registers = {0},
        .total_cycles = 0,
        .ime = false,
    };
    sm83->registers.pc = 0x100;
    sm83->registers.sp = 0xFFFE;
    sm83->registers.a = 0x01;
   
    sm83->bus = bus;
}

/* TODO: Implement IE & IF (Bus) and IME (internal) register check */
/* Perform one step of the fetch, decode, execute loop. */
uint32_t sm83_step(sm83_t* sm83) {
/* TODO: Implement full step logic after opcode table completed */
    uint32_t cycles;

    bool interrupt_pending = (sm83->bus->io.interrupt_enable & sm83->bus->io.interrupt_flags) != 0;
    if (sm83->halted && !interrupt_pending) {
        return 4;
    } else if (sm83->halted && interrupt_pending) {
        if (sm83->ime) {
            sm83->halted = false;
            return handle_interrupt(sm83);
        }

        if (!sm83->ime) {
            // TODO: https://gbdev.io/pandocs/halt.htmlhttps://gbdev.io/pandocs/halt
        }
    }

    if (sm83->ime && interrupt_pending) {
        return handle_interrupt(sm83);
    }
    
    uint8_t opcode = sm83_fetch8(sm83);
    if (opcode == 0xCB) {
        opcode = sm83_fetch8(sm83);
        cycles = sm83_opcode_table_cb[opcode].handler(sm83, opcode);
    } else {
        cycles = sm83_opcode_table[opcode].handler(sm83, opcode);
    }
    
    sm83->total_cycles += cycles;

    return cycles;
}

/* Returns the contents of an 8-bit register, or a byte at the address pointed to by the contents of the 16-bit HL register. */
uint8_t sm83_read_r8(sm83_t* sm83, uint8_t index) {
    switch (index) {
    case 0: return sm83->registers.b;
    case 1: return sm83->registers.c;
    case 2: return sm83->registers.d;
    case 3: return sm83->registers.e;
    case 4: return sm83->registers.h;
    case 5: return sm83->registers.l;
    case 6: return bus_read(sm83->bus, BUS_MASTER_SM83, sm83_read_r16(sm83, SM83_R16_HL));
    case 7: return sm83->registers.a;
    default:
        NO_IMPL
    }
}

/* Stores a value into an 8-bit register, or into the address pointed to by the contents of the 16-bit HL register. */
void sm83_write_r8(sm83_t* sm83, uint8_t index, uint8_t value) {
    switch (index) {
    case 0: sm83->registers.b = value; break;
    case 1: sm83->registers.c = value; break;
    case 2: sm83->registers.d = value; break;
    case 3: sm83->registers.e = value; break;
    case 4: sm83->registers.h = value; break;
    case 5: sm83->registers.l = value; break;
    case 6: bus_write(sm83->bus, BUS_MASTER_SM83, sm83_read_r16(sm83, SM83_R16_HL), value); break;
    case 7: sm83->registers.a = value; break;
    default:
        NO_IMPL
    }
}

/* Returns the contents of a 16-bit register, or the stack pointer. */
uint16_t sm83_read_r16(sm83_t* sm83, uint8_t pair) {
    switch (pair) {
    case 0: return (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c;
    case 1: return (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e;
    case 2: return (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
    case 3: return sm83->registers.sp;
    default:
        NO_IMPL
    }
}

uint16_t sm83_read_r16stk(sm83_t* sm83, uint8_t pair) {
    switch (pair) {
    case 0: return (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c;
    case 1: return (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e;
    case 2: return (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
    case 3: return (uint16_t)(sm83->registers.a << 8) | (uint16_t)sm83->registers.f;
    default:
        NO_IMPL
    }
}

uint16_t sm83_read_r16mem(sm83_t* sm83, uint8_t pair) {
    switch (pair) {
    case 0: return (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c;
    case 1: return (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e;
    case 2: case 3: return (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
    default:
        NO_IMPL
    }
}

/* Stores a value into a 16-bit regsiter, or into the stack pointer. */
void sm83_write_r16(sm83_t* sm83, uint8_t pair, uint16_t value) {
    switch (pair) {
    case 0: 
        sm83->registers.b = (uint8_t)(value >> 8);
        sm83->registers.c = (uint8_t)(value & 0xFF);
        break;
    case 1:
        sm83->registers.d = (uint8_t)(value >> 8);
        sm83->registers.e = (uint8_t)(value & 0xFF);
        break;
    case 2: 
        sm83->registers.h = (uint8_t)(value >> 8);
        sm83->registers.l = (uint8_t)(value & 0xFF);
        break;
    case 3: 
        sm83->registers.sp = value;
        break;
    default:
        NO_IMPL
    }
}

void sm83_write_r16stk(sm83_t* sm83, uint8_t pair, uint16_t value) {
    switch (pair) {
    case 0: 
        sm83->registers.b = (uint8_t)(value >> 8);
        sm83->registers.c = (uint8_t)(value & 0xFF);
        break;
    case 1:
        sm83->registers.d = (uint8_t)(value >> 8);
        sm83->registers.e = (uint8_t)(value & 0xFF);
        break;
    case 2: 
        sm83->registers.h = (uint8_t)(value >> 8);
        sm83->registers.l = (uint8_t)(value & 0xFF);
        break;
    case 3: 
        sm83->registers.a = (uint8_t)(value >> 8);
        sm83_update_flags(sm83, SM83_FLAG_N_MASK | SM83_FLAG_Z_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK, (uint8_t)(value & 0xFF));
        break;
    default:
        NO_IMPL
    }
}

void sm83_write_r16mem(sm83_t* sm83, uint8_t pair, uint16_t value) {
    switch (pair) {
    case 0: 
        sm83->registers.b = (uint8_t)(value >> 8);
        sm83->registers.c = (uint8_t)(value & 0xFF);
        break;
    case 1:
        sm83->registers.d = (uint8_t)(value >> 8);
        sm83->registers.e = (uint8_t)(value & 0xFF);
        break;
    case 2: case 3: 
        sm83->registers.h = (uint8_t)(value >> 8);
        sm83->registers.l = (uint8_t)(value & 0xFF);
        break;
    default:
        NO_IMPL
    }
}

/* Returns one of the Z, N, H, or C flags.
 * Compatible masks: SM83_FLAG_Z_MASK, SM83_FLAG_N_MASK, SM83_FLAG_H_MASK, SM83_FLAG_C_MASK. */
bool sm83_get_flag(sm83_t* sm83, uint8_t flag) {
    return (sm83->registers.f & flag) != 0;
}

/* Given a mask of affected flags and their new values, updates the flag register. 
 * Example usage: sm83_update_flags(sm83, SM83_FLAG_N_MASK | SM83_FLAG_C_MASK, n | c) */
void sm83_update_flags(sm83_t* sm83, uint8_t mask, uint8_t values) {
    sm83->registers.f = (sm83->registers.f & (uint8_t)~mask) | (values & mask);

    /* Ensures the lower 4 bits of F to 0 should the above arithmetic set them. */
    sm83->registers.f &= SM83_FLAG_Z_MASK | SM83_FLAG_N_MASK | SM83_FLAG_H_MASK | SM83_FLAG_C_MASK;
}

uint16_t sm83_stack_pop_r16(sm83_t* sm83) {
    uint8_t low = bus_read(sm83->bus, BUS_MASTER_SM83, sm83->registers.sp);
    sm83->registers.sp++;
    uint8_t high = bus_read(sm83->bus, BUS_MASTER_SM83, sm83->registers.sp);
    sm83->registers.sp++;

    return ((uint16_t)high << 8) | low;
}

void sm83_stack_push_r16(sm83_t* sm83, uint16_t value) {
    uint8_t high = (uint8_t)(value >> 8);
    uint8_t low = (uint8_t)(value & 0xFF);
    
    sm83->registers.sp--;
    bus_write(sm83->bus, BUS_MASTER_SM83, sm83->registers.sp, high);

    sm83->registers.sp--;
    bus_write(sm83->bus, BUS_MASTER_SM83, sm83->registers.sp, low);
}

/* Fetches the next byte in ROM and increments the program counter. */
uint8_t sm83_fetch8(sm83_t* sm83) {
    uint8_t value = bus_read(sm83->bus, BUS_MASTER_SM83, sm83->registers.pc++);

    return value;
}

/* Fetches the next 2 bytes in ROM and increments the program counter twice. */
uint16_t sm83_fetch16(sm83_t* sm83) {
    uint8_t low = sm83_fetch8(sm83);
    uint8_t high = sm83_fetch8(sm83);

    return ((uint16_t)high << 8) | low;
}

static uint32_t handle_interrupt(sm83_t* sm83) {
    uint8_t interrupt_flags = sm83->bus->io.interrupt_flags;
    uint8_t address = 0;

    if (BIT(interrupt_flags, INTERRUPT_BIT_VBLANK)) {
        BIT_CLEAR(interrupt_flags, INTERRUPT_BIT_VBLANK);
        address = INTERRUPT_ADDR_VBLANK;
    } else if (BIT(interrupt_flags, INTERRUPT_BIT_STAT)) {
        BIT_CLEAR(interrupt_flags, INTERRUPT_BIT_STAT);
        address = INTERRUPT_ADDR_STAT;
    } else if (BIT(interrupt_flags, INTERRUPT_BIT_TIMER)) {
        BIT_CLEAR(interrupt_flags, INTERRUPT_BIT_TIMER);
        address = INTERRUPT_ADDR_TIMER;
    } else if (BIT(interrupt_flags, INTERRUPT_BIT_SERIAL)) {
        BIT_CLEAR(interrupt_flags, INTERRUPT_BIT_SERIAL);
        address = INTERRUPT_ADDR_SERIAL;
    } else if (BIT(interrupt_flags, INTERRUPT_BIT_JOYPAD)) {
        BIT_CLEAR(interrupt_flags, INTERRUPT_BIT_JOYPAD);
        address = INTERRUPT_ADDR_JOYPAD;
    }

    sm83->ime = false;
    sm83_stack_push_r16(sm83, sm83->registers.pc);
    sm83->registers.pc = (uint16_t)address;

    return 5;
}

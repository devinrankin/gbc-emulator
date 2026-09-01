#include "sm83.h"
#include "sm83_opcodes.h"

/* Fetches the next byte in ROM and increments the program counter. */
static uint8_t sm83_fetch8(sm83_t* sm83) {
    uint8_t value = bus_read8(sm83->bus, sm83->registers.pc);
    sm83->registers.pc++;

    return value;
}

void sm83_init(sm83_t* sm83) {
    *sm83 = (sm83_t) {
        .state = SM83_STATE_RESET,
        .halted = false,
        .registers = {0}
    };
    sm83->registers.pc = 0x150;
}

/* Returns the contents of an 8-bit register, or a byte at the address pointed to by the contents of the 16-bit HL register. */
uint8_t sm83_read_r8_or_hl(sm83_t* sm83, uint8_t index) {
    switch(index) {
        case 0: return sm83->registers.b;
        case 1: return sm83->registers.c;
        case 2: return sm83->registers.d;
        case 3: return sm83->registers.e;
        case 4: return sm83->registers.h;
        case 5: return sm83->registers.l;
        case 6: return bus_read8(sm83->bus, sm83_read_r16(sm83, SM83_REGISTER_PAIR_HL));
        case 7: return sm83->registers.a;
        default:
            NO_IMPL
    }
}

/* Stores a value into an 8-bit register, or into the address pointed to by the contents of the 16-bit HL register. */
void sm83_write_r8_or_hl(sm83_t* sm83, uint8_t index, uint8_t val) {
    switch(index) {
        case 0: sm83->registers.b = val; break;
        case 1: sm83->registers.c = val; break;
        case 2: sm83->registers.d = val; break;
        case 3: sm83->registers.e = val; break;
        case 4: sm83->registers.h = val; break;
        case 5: sm83->registers.l = val; break;
        case 6: bus_write8(sm83->bus, sm83_read_r16(sm83, SM83_REGISTER_PAIR_HL), val); break;
        default:
            NO_IMPL
    }
}

/* Returns the contents of a 16-bit register, or the stack pointer. */
uint16_t sm83_read_r16(sm83_t* sm83, uint8_t pair) {
    switch(pair) {
        case 0: return (uint16_t)(sm83->registers.b << 8) | (uint16_t)sm83->registers.c;
        case 1: return (uint16_t)(sm83->registers.d << 8) | (uint16_t)sm83->registers.e;
        case 2: return (uint16_t)(sm83->registers.h << 8) | (uint16_t)sm83->registers.l;
        case 3: return sm83->registers.sp;
        default:
                NO_IMPL
    }
}

/* Stores a value into a 16-bit regsiter, or into the stack pointer. */
void sm83_write_r16(sm83_t* sm83, uint8_t pair, uint16_t val) {
    switch(pair) {
        case 0: 
            sm83->registers.b = (uint8_t)((val & 0xFF00) >> 8);
            sm83->registers.c = (uint8_t)(val & 0x00FF);
            break;
        case 1:
            sm83->registers.d= (uint8_t)((val & 0xFF00) >> 8);
            sm83->registers.e = (uint8_t)(val & 0x00FF);
            break;
        case 2: 
            sm83->registers.h = (uint8_t)((val & 0xFF00) >> 8);
            sm83->registers.l = (uint8_t)(val & 0x00FF);
            break;
        case 3: 
            sm83->registers.sp = val;
        default:
                NO_IMPL
    }
}

/* Perform one step of the fetch, decode, execute loop. */
void sm83_step(sm83_t* sm83) {
/* TODO: Implement full step logic after opcode table completed */
    if(sm83->halted) {
        return;
    }

    uint8_t opcode = sm83_fetch8(sm83);
}

/* Returns one of the Z, N, H, or C flags.
 * Compatible macros: SM83_FLAG_Z, SM83_FLAG_N, SM83_FLAG_H, SM83_FLAG_C. */
bool sm83_get_flag(sm83_t* sm83, uint8_t flag) {
    return (sm83->registers.f & flag) != 0;
}

/* Given a mask of affected flags and their new values, updates the flag register. */
void sm83_update_flags(sm83_t* sm83, uint8_t mask, uint8_t values) {
    sm83->registers.f = 
        (sm83->registers.f & (uint8_t)~mask) |
        (values & mask);

    sm83->registers.f &= SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C;
}

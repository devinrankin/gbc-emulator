#include "sm83.h"
#include "sm83_opcodes.h"

/* Fetches the next byte in ROM and increments the program counter. */
static uint8_t sm83_fetch8(sm83_t* sm83);

void sm83_init(sm83_t* sm83, bus_t* bus) {
    *sm83 = (sm83_t) {
        .state = SM83_STATE_INIT,
        .halted = false,
        .registers = {0},
        .total_cycles = 0
    };
    sm83->registers.pc = 0x150;
    
    sm83->bus = bus;
}

void sm83_dump(sm83_t* sm83) {
    printf("Dumping SM83...\n");

    printf("State: %d\n", sm83->state);
    printf("Halted: %b\n", sm83->halted);
    printf("Total Cycles Elapsed: %lu\n\n", sm83->total_cycles);
    printf("Registers: \n");
    printf("\tPC: %u\n", sm83->registers.pc);
    printf("\tSP: %u\n", sm83->registers.sp);
    printf("\tB: %u", sm83->registers.b);
    printf("\tC: %u", sm83->registers.c);
    printf("\tD: %u", sm83->registers.d);
    printf("\tE: %u", sm83->registers.e);
    printf("\tH: %u", sm83->registers.h);
    printf("\tL: %u", sm83->registers.l);
    printf("\tA: %u", sm83->registers.a);
    printf("\tF: %u", sm83->registers.f);
}

/* TODO: Implement IE & IF (Bus) and IME (internal) register check */
/* Perform one step of the fetch, decode, execute loop. */
uint32_t sm83_step(sm83_t* sm83) {
/* TODO: Implement full step logic after opcode table completed */
    uint32_t cycles;
    
    if (sm83->halted) {
        cycles = 4;
    } else {
        uint8_t opcode = sm83_fetch8(sm83);
        cycles = (sm83_opcode_table[opcode].handler)(sm83, opcode);
    }

    sm83->total_cycles += cycles;
    return cycles;
}

/* Returns the contents of an 8-bit register, or a byte at the address pointed to by the contents of the 16-bit HL register. */
uint8_t sm83_read_r8_or_hl(sm83_t* sm83, uint8_t index) {
    switch (index) {
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
void sm83_write_r8_or_hl(sm83_t* sm83, uint8_t index, uint8_t value) {
    switch (index) {
        case 0: sm83->registers.b = value; break;
        case 1: sm83->registers.c = value; break;
        case 2: sm83->registers.d = value; break;
        case 3: sm83->registers.e = value; break;
        case 4: sm83->registers.h = value; break;
        case 5: sm83->registers.l = value; break;
        case 6: bus_write8(sm83->bus, sm83_read_r16(sm83, SM83_REGISTER_PAIR_HL), value); break;
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

/* Stores a value into a 16-bit regsiter, or into the stack pointer. */
void sm83_write_r16(sm83_t* sm83, uint8_t pair, uint16_t value) {
    switch (pair) {
        case 0: 
            sm83->registers.b = (uint8_t)((value & 0xFF00) >> 8);
            sm83->registers.c = (uint8_t)(value & 0x00FF);
            break;
        case 1:
            sm83->registers.d = (uint8_t)((value & 0xFF00) >> 8);
            sm83->registers.e = (uint8_t)(value & 0x00FF);
            break;
        case 2: 
            sm83->registers.h = (uint8_t)((value & 0xFF00) >> 8);
            sm83->registers.l = (uint8_t)(value & 0x00FF);
            break;
        case 3: 
            sm83->registers.sp = value;
        default:
                NO_IMPL
    }
}

/* Returns one of the Z, N, H, or C flags.
 * Compatible masks: SM83_FLAG_Z, SM83_FLAG_N, SM83_FLAG_H, SM83_FLAG_C. */
bool sm83_get_flag(sm83_t* sm83, uint8_t flag) {
    return (sm83->registers.f & flag) != 0;
}

/* Given a mask of affected flags and their new values, updates the flag register. 
 * Example usage: sm83_update_flags(sm83, SM83_FLAG_N | SM83_FLAG_C, n | c) */
void sm83_update_flags(sm83_t* sm83, uint8_t mask, uint8_t values) {
    sm83->registers.f = (sm83->registers.f & (uint8_t)~mask) | (values & mask);

    /* Ensures the lower 4 bits of F to 0 should the above arithmetic set them. */
    sm83->registers.f &= SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C;
}

static uint8_t sm83_fetch8(sm83_t* sm83) {
    uint8_t value = bus_read8(sm83->bus, sm83->registers.pc);
    sm83->registers.pc++;

    return value;
}

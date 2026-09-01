#ifndef SM83_H
#define SM83_H

#include <stdint.h>
#include <stdbool.h>

#include "../mem/bus.h"

#define SM83_GB_CLOCK_HZ (4194304u)
#define SM83_CGB_CLOCK_HZ (8388608u)

#define SM83_REGISTER_IDX_A 7
#define SM83_REGISTER_PAIR_HL 2

#define SM83_FLAG_Z 0x80
#define SM83_FLAG_N 0x40
#define SM83_FLAG_H 0x20
#define SM83_FLAG_C 0x10

typedef struct {
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint8_t a;
    uint8_t f;
    uint16_t sp;
    uint16_t pc;
} registers_t;

typedef enum {
    SM83_STATE_RESET,
    SM83_STATE_RUNNING,
    SM83_STATE_HALTED
} sm83_state_t;

typedef struct sm83 sm83_t;

struct sm83 {
    sm83_state_t state;
    registers_t registers;
    bus_t* bus;

    bool halted;
    bool ime;
};

void sm83_init(sm83_t* sm83);
void sm83_step(sm83_t* sm83);
void sm83_run(uint8_t* rom, sm83_t* sm83);

bool sm83_get_flag(sm83_t* sm83, uint8_t flag);
void sm83_update_flags(sm83_t* sm83, uint8_t mask, uint8_t values);

uint8_t sm83_read_r8_or_hl(sm83_t* sm83, uint8_t index);
void sm83_write_r8_or_hl(sm83_t* sm83, uint8_t index, uint8_t value);

uint16_t sm83_read_r16(sm83_t* sm83, uint8_t pair);
void sm83_write_r16(sm83_t* sm83, uint8_t pair, uint16_t value);

#endif

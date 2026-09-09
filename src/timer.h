#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

#define TIMER_REG_DIV 0xFF04
#define TIMER_REG_TIMA 0xFF05
#define TIMER_REG_TMA 0xFF06
#define TIMER_REG_TAC 0xFF07

#define TAC_TIMER_ENABLED(tac) (((tac) & 0x04u) != 0)
#define TAC_CLOCK_SELECT(tac) ((tac) & 0x03u)

#define TIMER_TAC_SELECT_4096HZ 9
#define TIMER_TAC_SELECT_262144HZ 3
#define TIMER_TAC_SELECT_65536HZ 5
#define TIMER_TAC_SELECT_16384HZ 7

#define TIMER_CLOCK_FREQ 4194304u
#define TIMER_CGB_CLOCK_FREQ 8388608u

typedef struct {
    uint16_t div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;

    uint8_t reload_delay;

    bool timer_interrupt;
} gbc_timer_t;

void timer_init(gbc_timer_t* timer);
void timer_tick(gbc_timer_t* timer);
uint8_t timer_read(gbc_timer_t* timer, uint16_t address);
void timer_write(gbc_timer_t* timer, uint16_t address, uint8_t value);

#endif

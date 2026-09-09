#include "timer.h"

static inline bool check_falling_edge(uint16_t previous, uint16_t current, uint8_t bit);

void timer_init(gbc_timer_t *timer) {
    *timer = (gbc_timer_t){0};
}

void timer_tick(gbc_timer_t* timer) {
    bool timer_updated = false;
    switch (TAC_CLOCK_SELECT(timer->tac)) {
        case 0x00:
            timer_updated = check_falling_edge(timer->div, timer->div++, TIMER_TAC_SELECT_4096HZ);
        case 0x01:
            timer_updated = check_falling_edge(timer->div, timer->div++, TIMER_TAC_SELECT_262144HZ);
        case 0x02: 
            timer_updated = check_falling_edge(timer->div, timer->div++, TIMER_TAC_SELECT_65536HZ);
        case 0x03:
            timer_updated = check_falling_edge(timer->div, timer->div++, TIMER_TAC_SELECT_16384HZ);
    }

    if (timer_updated && TAC_TIMER_ENABLED(timer->tac)) {
        timer->tima++; 
    }
    
    /* TODO: interrupt */
    if(timer->tima == 0xFF) {
        timer->tima = timer->tma;
        timer->timer_interrupt = true;
    }
}

uint8_t timer_read(gbc_timer_t* timer, uint16_t address) {
    switch (address) {
        case TIMER_REG_DIV:
            return timer->div;
        case TIMER_REG_TIMA:
            return timer->tima;
        case TIMER_REG_TMA:
            return timer->tma;
        case TIMER_REG_TAC:
            return timer->tac;
        default:
            return 0xFF;
    }
}

void timer_write(gbc_timer_t* timer, uint16_t address, uint8_t value) {
    switch (address) {
        case TIMER_REG_DIV:
            timer->div = 0;
        case TIMER_REG_TIMA:
            timer->tima = value;
            break;
        case TIMER_REG_TMA:
            timer->tma = value;
            break;
        case TIMER_REG_TAC:
            timer->tac = value;
            break;
        default:
            return;
    }
}

static inline bool check_falling_edge(uint16_t previous, uint16_t current, uint8_t bit) {
    uint16_t mask = (uint16_t)(1 << bit);

    return (previous & mask) != 0 && (current & mask) == 0;
}

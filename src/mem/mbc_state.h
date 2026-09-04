#ifndef MBC_STATE_H
#define MBC_STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t rom_bank_low;
    uint8_t bank_high;
    uint8_t banking_mode;
    bool ram_enabled;
} mbc1_state_t;

typedef struct {
    uint8_t rom_bank;
    bool ram_enabled;
} mbc2_state_t;

typedef struct {
    uint8_t rom_bank;
    uint8_t ram_rtc_select;
    bool ram_enabled;

    struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t days_low;
        uint8_t control;
    } rtc;

    bool rtc_latched;
} mbc3_state_t;

typedef struct {
    uint16_t rom_bank;
    uint8_t ram_bank;
    bool ram_enabled;
} mbc5_state_t;

#endif

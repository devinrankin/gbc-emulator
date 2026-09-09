#ifndef GBC_H
#define GBC_H

#include "bus.h"
#include "timer.h"
#include "sm83/sm83.h"
#include "mem/cartridge.h"

typedef struct {
    sm83_t sm83;
    bus_t bus;
    cartridge_t cartridge;
    gbc_timer_t timer;
} gbc_t;

void gbc_init(gbc_t* gbc, const char* rom_path);

#endif

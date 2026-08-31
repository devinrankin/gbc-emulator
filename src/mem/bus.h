#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include "../mem/cartridge.h"

typedef struct {
    cartridge_t* cartridge;

    uint8_t vram[0x2000];

} bus_t;

uint8_t bus_read8(cartridge_t* cartridge, uint16_t addr);
void bus_write8(cartridge_t* cartridge, uint16_t addr, uint8_t value);

uint16_t bus_read16(cartridge_t* cartridge, uint16_t addr);
void bus_write16(cartridge_t* cartridge, uint16_t addr, uint16_t value);

#endif

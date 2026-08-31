#ifndef BUS_H
#define BUS_H

#include "../common.h"
#include "../mem/cartridge.h"

uint8_t bus_read8(cartridge_t* cartridge, uint16_t addr);
void bus_write8(cartridge_t* cartridge, uint16_t addr, uint8_t value);

uint16_t bus_read16(cartridge_t* cartridge, uint16_t addr);
void bus_write16(cartridge_t* cartridge, uint16_t addr, uint16_t value);

#endif

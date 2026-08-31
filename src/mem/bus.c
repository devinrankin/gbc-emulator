#include "bus.h"

uint8_t bus_read8(cartridge_t* cartridge, uint16_t addr) {
    if(addr < 0x4000) {
        return cartridge_read8(cartridge, addr);
    }
    NO_IMPL
}

void bus_write8(cartridge_t* cartridge, uint16_t addr, uint8_t value) {
    if(addr < 0x4000) {
        cartridge_write8(cartridge, addr, value);
        return;
    }
    NO_IMPL
}

uint16_t bus_read16(cartridge_t* cartridge , uint16_t addr) {
    uint16_t lo = cartridge_read8(cartridge, addr);
    uint16_t hi = cartridge_read8(cartridge, addr + 1);

    return lo | (hi << 8);
}

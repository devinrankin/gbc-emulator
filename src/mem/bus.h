#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include "../common.h"
#include "cartridge.h"

#define BUS_VRAM_SIZE 16 * SIZE_KIB
#define BUS_WRAM_SIZE 32 * SIZE_KIB
#define BUS_OAM_SIZE 0xA0
#define BUS_IO_SIZE 0x80 
#define BUS_HRAM_SIZE 0x7F

#define BUS_VRAM_BANK_SIZE 8 * SIZE_KIB
#define BUS_WRAM_BANK_SIZE 4 * SIZE_KIB

#define BUS_REG_VBK 0xFF4F
#define BUS_REG_SVBK 0xFF70

typedef struct {
    cartridge_t* cartridge;

    uint8_t vram[BUS_VRAM_SIZE];
    uint8_t wram[BUS_WRAM_SIZE];

    uint8_t oam[BUS_OAM_SIZE];
    uint8_t io[BUS_IO_SIZE];

    uint8_t hram[BUS_HRAM_SIZE];

    uint8_t vram_bank;
    uint8_t wram_bank;

    uint8_t interrupt_flags;
    uint8_t interrupt_enable;
} bus_t;

void bus_init(bus_t* bus, cartridge_t* cartridge);

uint8_t bus_read8(bus_t* bus, uint16_t address);
void bus_write8(bus_t* bus, uint16_t address, uint8_t value);

uint16_t bus_read16(bus_t* bus, uint16_t address);

#endif

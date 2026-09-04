#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#include "../common.h"
#include "cartridge.h"

/* TODO: write macros for memory blocks */

#define BUS_VRAM_SIZE (16 * SIZE_KIB)
#define BUS_WRAM_SIZE (32 * SIZE_KIB)
#define BUS_OAM_SIZE 0xA0
#define BUS_IO_SIZE 0x80 
#define BUS_HRAM_SIZE 0x7F

#define BUS_VRAM_BANK_SIZE (8 * SIZE_KIB)
#define BUS_WRAM_BANK_SIZE (4 * SIZE_KIB)

#define BUS_REG_VBK 0xFF4F
#define BUS_REG_SVBK 0xFF70

    /*
     * Game Boy Color Memory Layout:
     *  $0000 - $3FFF ROM (bank 0)
     *  $4000 - $7FFF ROM (bank 1..n)
     *  $8000 - $9FFF VRAM (bank 0 non-CGB, bank 0-1 CGB)
     *  $A000 - $BFFF RAM (external from cartridge)
     *  $C000 - $CFFF WRAM (bank 0)
     *  $D000 - $DFFF WRAM (bank 1-7, CGB-only)
     *  $E000 - $FDFF Echo RAM
     *  $FE00 - $FE9F OAM (Object Attribute Memory)
     *  $FEA0 - $FEFF Not Usable
     *  $FF00 - $FF7F I/O Registers
     *  $FF80 - $FFFE HRAM (High RAM)
     *  $FFFF - $FFFF IE (Interrupt Enable Register)
     */

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
void bus_write16(bus_t* bus, uint16_t address, uint16_t value);

#endif

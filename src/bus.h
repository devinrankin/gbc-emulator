#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#include "mem/cartridge.h"
#include "io.h"

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

#define ADDR_ROM0_BASE 0x0000
#define ADDR_ROM0_SIZE 0x4000

/* Rom Banks 1-X */
#define ADDR_ROMX_BASE 0x4000
#define ADDR_ROMX_SIZE 0x4000

#define ADDR_VRAM_BASE 0x8000
#define ADDR_VRAM_SIZE 0x2000

/* External (Cartridge) RAM */
#define ADDR_EXRAM_BASE 0xA000
#define ADDR_EXRAM_SIZE 0x2000

#define ADDR_WRAM0_BASE 0xC000
#define ADDR_WRAM0_SIZE 0x1000

/* WRAM Banks 1-7 */
#define ADDR_WRAMX_BASE 0xD000
#define ADDR_WRAMX_SIZE 0x1000

/* Echo RAM (mirror of C000 - DDFF) */
#define ADDR_ERAM_BASE 0xE000
#define ADDR_ERAM_SIZE 0x1E00

/* Object Attribute Memory */
#define ADDR_OAM_BASE 0xFE00
#define ADDR_OAM_SIZE 0x00A0

/* Not usable */
#define ADDR_NU_BASE 0xFEA0
#define ADDR_NU_SIZE 0x0060

#define ADDR_IO_BASE 0xFF00
#define ADDR_IO_SIZE 0x0080

#define ADDR_HRAM_BASE 0xFF80
#define ADDR_HRAM_SIZE 0x007F

/* Interrupt Enable register */
#define ADDR_IE 0xFFFF

typedef enum {
    BUS_MASTER_SM83,
    BUS_MASTER_PPU,
    BUS_MASTER_NONE,
} bus_master_t;

typedef struct {
    cartridge_t* cartridge;
    io_t io;

    uint8_t vram[ADDR_VRAM_SIZE * 2u];
    uint8_t wram[ADDR_WRAM0_SIZE * 8u];

    uint8_t oam[ADDR_OAM_SIZE];

    uint8_t hram[ADDR_HRAM_SIZE];

    uint8_t vram_bank;
    uint8_t wram_bank;

    uint8_t interrupt_flags;
    uint8_t interrupt_enable;
} bus_t;

void bus_init(bus_t* bus, cartridge_t* cartridge);

uint8_t bus_read(bus_t* bus, bus_master_t consumer, uint16_t address);
void bus_write(bus_t* bus, bus_master_t consumer, uint16_t address, uint8_t value);

#endif

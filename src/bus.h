#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#include "mem/cartridge.h"
#include "io.h"

#define BUS_WRAM_SIZE (32 * SIZE_KIB)
#define BUS_OAM_SIZE 0xA0
#define BUS_HRAM_SIZE 0x7F

#define BUS_VRAM_BANK_SIZE (8 * SIZE_KIB)
#define BUS_VRAM_BANK_COUNT 2u

#define BUS_WRAM_BANK_SIZE (4 * SIZE_KIB)


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

#define ADDR_ROM0_START 0x0000
#define ADDR_ROM0_END 0x3FFF

/* Rom Banks 1-NN */
#define ADDR_ROMNN_START 0x4000
#define ADDR_ROMNN_END 0x7FFF

#define ADDR_VRAM_START 0x8000
#define ADDR_VRAM_END 0x9FFF

/* External (Cartridge) RAM */
#define ADDR_EXRAM_START 0xA000
#define ADDR_EXRAM_END 0xBFFF

#define ADDR_WRAM0_START 0xC000
#define ADDR_WRAM0_END 0xCFFF

/* WRAM Banks 1-7 */
#define ADDR_WRAMNN_START 0xD000
#define ADDR_WRAMNN_END 0xDFFF

/* Echo RAM (mirror of C000 - DDFF) */
#define ADDR_ERAM_START 0xE000
#define ADDR_ERAM_END 0xFDFF

/* Object Attribute Memory */
#define ADDR_OAM_START 0xFE00
#define ADDR_OAM_END 0xFE9F

/* Not usable */
#define ADDR_NU_START 0xFEA0
#define ADDR_NU_END 0xFEFF

#define ADDR_IO_START 0xFF00
#define ADDR_IO_END 0xFF7F

#define BUS_ADDR_VBK 0xFF4F
#define BUS_ADDR_SVBK 0xFF70

#define ADDR_HRAM_START 0xFF80
#define ADDR_HRAM_END 0xFFFE

/* Interrupt Enable register */
#define ADDR_IE 0xFFFF

typedef enum {
    CONSUMER_NONE,
    CONSUMER_SM83,
    CONSUMER_PPU
} consumer_t;

typedef struct {
    cartridge_t* cartridge;
    io_t* io;

    uint8_t vram[BUS_VRAM_BANK_SIZE * BUS_VRAM_BANK_COUNT];
    uint8_t wram[BUS_WRAM_SIZE];

    uint8_t oam[BUS_OAM_SIZE];

    uint8_t hram[BUS_HRAM_SIZE];

    uint8_t vram_bank;
    uint8_t wram_bank;

    uint8_t interrupt_flags;
    uint8_t interrupt_enable;
} bus_t;

void bus_init(bus_t* bus, cartridge_t* cartridge);

uint8_t bus_read8(bus_t* bus, consumer_t consumer, uint16_t address);
void bus_write8(bus_t* bus, consumer_t consumer, uint16_t address, uint8_t value);

#endif

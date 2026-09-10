#ifndef ADDRESS_MAP_H
#define ADDRESS_MAP_H


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

#define ADDR_HRAM_START 0xFF80
#define ADDR_HRAM_END 0xFFFE

/* Interrupt Enable register */
#define ADDR_IE 0xFFFF

#endif

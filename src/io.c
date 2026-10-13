#include "io.h"

uint8_t io_read(io_t* io, uint16_t address) {
    switch (address) {
        case IO_ADDR_JOYP: return joypad_read(io->joypad);
        case IO_ADDR_DIV: case IO_ADDR_TIMA: case IO_ADDR_TMA: case IO_ADDR_TAC: return timer_read(io->timer, address);
        case IO_ADDR_IF: return io->interrupt_flags;
        case IO_ADDR_NR10 ... IO_ADDR_WAVERAMF: return apu_read(io->apu, address);
        case IO_ADDR_LCDC: return io->lcdc;
        case IO_ADDR_STAT: return io->stat;
        case IO_ADDR_SCY: return io->scy;
        case IO_ADDR_SCX: return io->scx;
        case IO_ADDR_LY: return io->ly;
        case IO_ADDR_LYC: return io->lyc;
        case IO_ADDR_BGP: return io->bgp;
        case IO_ADDR_OBP0: return io->obp0;
        case IO_ADDR_OBP1: return io->obp1;
        case IO_ADDR_WX: return io->wx;
        case IO_ADDR_WY: return io->wy;
        case IO_ADDR_KEY0: return io->key0;
        case IO_ADDR_KEY1: return io->key1;
        case IO_ADDR_VBK: return io->vbk;
        case IO_ADDR_BGPI: return io->bgpi;
        case IO_ADDR_BGPD: return io->bgpd;
        case IO_ADDR_OBPI: return io->obpi;
        case IO_ADDR_OBPD: return io->obpd;
        case IO_ADDR_SVBK: return io->svbk;
        case IO_ADDR_IE: return io->interrupt_enable;
        default: return 0xFF;
    }
}

void io_write(io_t* io, uint16_t address, uint8_t value) {
    switch(address) {    
        case IO_ADDR_JOYP: joypad_write(io->joypad, value); break;
        case IO_ADDR_DIV: case IO_ADDR_TIMA: case IO_ADDR_TMA: case IO_ADDR_TAC: timer_write(io->timer, address, value); break;
        case IO_ADDR_IF: io->interrupt_flags = value; break;
        case IO_ADDR_NR10 ... IO_ADDR_WAVERAMF: apu_write(io->apu, address, value);
        case IO_ADDR_LCDC: io->lcdc = value; break;
        case IO_ADDR_STAT: io->stat = value; break;
        case IO_ADDR_SCY: io->scy = value; break;
        case IO_ADDR_SCX: io->scx = value; break;
        case IO_ADDR_LY: io->ly = value; break;
        case IO_ADDR_LYC: io->lyc = value; break;
        case IO_ADDR_BGP: io->bgp = value; break;
        case IO_ADDR_OBP0: io->obp0 = value; break;
        case IO_ADDR_OBP1: io->obp1 = value; break;
        case IO_ADDR_WX: io->wx = value; break;
        case IO_ADDR_WY: io->wy = value; break;
        case IO_ADDR_KEY0: io->key0 = value; break;
        case IO_ADDR_KEY1: io->key1 = value; break;
        case IO_ADDR_VBK: io->vbk = value; break;
        case IO_ADDR_BGPI: io->bgpi = value; break;
        case IO_ADDR_BGPD: io->bgpd = value; break;
        case IO_ADDR_OBPI: io->obpi = value; break;
        case IO_ADDR_OBPD: io->obpd = value; break;
        case IO_ADDR_SVBK: io->svbk = value; break;
        case IO_ADDR_IE: io->interrupt_enable = value; break;
        default: break;
    }
}

#include "bus.h"
#include "mem/address_map.h"

static uint8_t bus_read_vram(bus_t* bus, uint16_t address);
static void bus_write_vram(bus_t* bus, uint16_t address, uint8_t value);
static uint8_t bus_read_wram(bus_t* bus, uint16_t address); 
static void bus_write_wram(bus_t* bus, uint16_t address, uint8_t value); 
static void bus_set_vram_bank(bus_t* bus, uint8_t value);
static void bus_set_wram_bank(bus_t* bus, uint8_t value);

void bus_init(bus_t* bus, cartridge_t* cartridge, gbc_timer_t* timer) {
    *bus = (bus_t) {
        .cartridge = cartridge,
        .timer = timer,
        .vram_bank = 0,
        .wram_bank = 1,
        .interrupt_flags = 0,
        .interrupt_enable = 0
    };
}

/* Returns a byte from a pre-defined block of memory pointed to by the specified address. */
uint8_t bus_read8(bus_t* bus, uint16_t address) {
    if(address <= ADDR_ROMNN_END || (address >= ADDR_EXRAM_END && address <= ADDR_EXRAM_START)) {
        return cartridge_read(bus->cartridge, address);
    }

    if(address <= ADDR_VRAM_END) {
        return bus_read_vram(bus, address); 
    }

    if(address <= ADDR_WRAMNN_END) {
        return bus_read_wram(bus, address);
    }

    if(address <= ADDR_ERAM_END) {
        return bus_read_wram(bus, address - 0x2000);
    }

    if(address <= ADDR_OAM_END) {
        return bus->oam[address - 0xFE00];
    }

    if(address <= ADDR_NU_END) {
        return 0xFF;
    }

    if(address == 0xFF0F) {
        return bus->interrupt_flags;
    }

    if(address == BUS_REG_VBK) {
        return bus->vram_bank | 0xFE;
    }

    if(address == BUS_REG_SVBK) {
        return bus->wram_bank | 0xF8;
    }

    if(address <= ADDR_IO_END) {
        return bus->io[address - 0xFF00];
    }

    if(address <= ADDR_HRAM_END) {
        return bus->hram[address - 0xFF80];
    }

    return bus->interrupt_enable;
}

/* Attempts to write a byte to a pre-defined block of memory pointed to by the specified address. */
void bus_write8(bus_t* bus, uint16_t address, uint8_t value) {
    if(address <= ADDR_ROMNN_END  || (address >= ADDR_EXRAM_END && address <= ADDR_EXRAM_END)) {
        cartridge_write(bus->cartridge, address, value);
        return;
    }

    if(address <= ADDR_VRAM_END) {
        bus_write_vram(bus, address, value);
        return;
    }

    if(address <= ADDR_WRAMNN_END) {
        bus_write_wram(bus, address, value);
        return;
    }

    if(address <= ADDR_ERAM_END) {
        bus_write_wram(bus, address - 0x2000, value);
        return;
    }

    if(address <= ADDR_OAM_END) {
        bus->oam[address - 0xFE00] = value;
        return;
    }

    if(address <= ADDR_NU_END) {
        return;
    }

    if(address == 0xFF0F) {
        bus->interrupt_flags = value;
        return;
    }

    if(address == BUS_REG_VBK) {
        bus_set_vram_bank(bus, value);
        return;
    }

    if(address == BUS_REG_SVBK) {
        bus_set_wram_bank(bus, value);
        return;
    }

    if(address <= ADDR_IO_END) {
        bus->io[address - 0xFF00] = value;
        return;
    }

    if(address <= ADDR_HRAM_END) {
        bus->hram[address - 0xFF80] = value;
        return;
    }

    bus->interrupt_enable = value;
}

uint16_t bus_read16(bus_t* bus, uint16_t address) {
    uint8_t low = bus_read8(bus, address);
    uint8_t high = bus_read8(bus, address + 1);

    return (uint16_t)((high << 8) | low);
}

void bus_write16(bus_t* bus, uint16_t address, uint16_t value) {
    bus_write8(bus, address, value & 0xFF);
    bus_write8(bus, address + 1, value >> 8);
}

/* Returns the byte in VRAM pointed to by the specified address. */
static uint8_t bus_read_vram(bus_t* bus, uint16_t address) {
    uint16_t physical_offset = (uint16_t)bus->vram_bank * BUS_VRAM_BANK_SIZE + (address - 0x8000);

    return bus->vram[physical_offset];
}

/* Writes a byte into VRAM at the specified address. */
static void bus_write_vram(bus_t* bus, uint16_t address, uint8_t value) {
    uint16_t physical_offset = (uint16_t)bus->vram_bank * BUS_VRAM_BANK_SIZE + (address - 0x8000);

    bus->vram[physical_offset] = value;
}

/* Returns the byte in WRAM pointed to by the specified address. */
static uint8_t bus_read_wram(bus_t* bus, uint16_t address) {
    uint16_t physical_offset;

    if(address <= ADDR_WRAM0_END) {
        physical_offset = address - ADDR_WRAM0_START;
    } else {
        physical_offset = (uint16_t)bus->wram_bank * BUS_WRAM_BANK_SIZE + (address - ADDR_WRAMNN_START);
    }

    return bus->wram[physical_offset];
}

/* Writes a byte into WRAM at the specified address. */
static void bus_write_wram(bus_t* bus, uint16_t address, uint8_t value) {
    uint16_t physical_offset;

    if(address <= ADDR_WRAM0_END) {
        physical_offset = address - ADDR_WRAM0_START;
    } else {
        physical_offset = (uint16_t)bus->wram_bank * BUS_WRAM_BANK_SIZE + (address - ADDR_WRAMNN_START);
    }

    bus->wram[physical_offset] = value;
}

static void bus_set_vram_bank(bus_t* bus, uint8_t value) {
    bus->vram_bank = value & 0x01;
}

static void bus_set_wram_bank(bus_t* bus, uint8_t value) {
    uint8_t bank = value & 0x07;
    bus->wram_bank = (bank == 0) ? 1 : 0;
}


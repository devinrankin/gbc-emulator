#include "bus.h"

static uint8_t vram_read(bus_t* bus, bus_master_t master, uint16_t address);
static void vram_write(bus_t* bus, bus_master_t master, uint16_t address, uint8_t value);
static uint8_t wram_read(bus_t* bus, uint16_t address); 
static void wram_write(bus_t* bus, uint16_t address, uint8_t value); 

void bus_init(bus_t* bus, cartridge_t* cartridge) {
    *bus = (bus_t) {
        .cartridge = cartridge,
        .vram_bank = 0,
        .wram_bank = 1,
        .interrupt_flags = 0,
        .interrupt_enable = 0,
    };
}

/* Returns a byte from a pre-defined block of memory pointed to by the specified address. */
uint8_t bus_read(bus_t* bus, bus_master_t master, uint16_t address) {
    if (BETWEEN(address, ADDR_ROM0_BASE, ADDR_ROMX_BASE + ADDR_ROMX_SIZE) || BETWEEN(address, ADDR_EXRAM_BASE, ADDR_EXRAM_BASE + ADDR_EXRAM_SIZE)) {
        return cartridge_read(bus->cartridge, address);
    }

    if (BETWEEN(address, ADDR_VRAM_BASE, ADDR_VRAM_BASE + ADDR_VRAM_SIZE)) {
        return vram_read(bus, master, address); 
    }

    if (BETWEEN(address, ADDR_WRAM0_BASE, ADDR_WRAMX_BASE + ADDR_WRAMX_SIZE)) {
        return wram_read(bus, address);
    }

    if (BETWEEN(address, ADDR_ERAM_BASE, ADDR_ERAM_BASE + ADDR_ERAM_SIZE)) {
        return wram_read(bus, address - (ADDR_ERAM_BASE - ADDR_WRAM0_BASE));
    }

    if (BETWEEN(address, ADDR_OAM_BASE, ADDR_OAM_BASE + ADDR_OAM_SIZE)) {
        return bus->oam[address - ADDR_OAM_BASE];
    }

    if (BETWEEN(address, ADDR_NU_BASE, ADDR_NU_BASE + ADDR_NU_SIZE)) {
        return 0xFF;
    }

    if (BETWEEN(address, ADDR_IO_BASE, ADDR_IO_BASE + ADDR_IO_SIZE)) {
        return io_read(&bus->io, address); 
    }

    if (BETWEEN(address, ADDR_HRAM_BASE, ADDR_HRAM_BASE + ADDR_HRAM_SIZE)) {
        return bus->hram[address - ADDR_HRAM_BASE];
    }

    return bus->interrupt_enable;
}

/* Attempts to write a byte to a pre-defined block of memory pointed to by the specified address. */
void bus_write(bus_t* bus, bus_master_t master, uint16_t address, uint8_t value) {
    if (BETWEEN(address, ADDR_ROM0_BASE, ADDR_ROMX_BASE + ADDR_ROMX_SIZE) || BETWEEN(address, ADDR_EXRAM_BASE, ADDR_EXRAM_BASE + ADDR_EXRAM_SIZE)) {
        cartridge_write(bus->cartridge, address, value);
        return;
    }

    if (BETWEEN(address, ADDR_VRAM_BASE, ADDR_VRAM_BASE + ADDR_VRAM_SIZE)) {
        vram_write(bus, master, address, value);
        return;
    }

    if (BETWEEN(address, ADDR_WRAM0_BASE, ADDR_WRAMX_BASE + ADDR_WRAMX_SIZE)) {
        wram_write(bus, address, value);
        return;
    }

    if (BETWEEN(address, ADDR_ERAM_BASE, ADDR_ERAM_BASE + ADDR_ERAM_SIZE)) {
        wram_write(bus, address - (ADDR_ERAM_BASE - ADDR_WRAM0_BASE), value);
        return;
    }
 
    if (BETWEEN(address, ADDR_OAM_BASE, ADDR_OAM_BASE + ADDR_OAM_SIZE)) {
        bus->oam[address - ADDR_OAM_BASE] = value;
        return;
    }
 
    if (BETWEEN(address, ADDR_NU_BASE, ADDR_NU_BASE + ADDR_NU_SIZE)) {
        return;
    }

    if (BETWEEN(address, ADDR_IO_BASE, ADDR_IO_BASE + ADDR_IO_SIZE)) {
        io_write(&bus->io, address, value);
        return;
    }

    if (BETWEEN(address, ADDR_HRAM_BASE, ADDR_HRAM_BASE + ADDR_HRAM_SIZE)) {
        bus->hram[address - ADDR_HRAM_BASE] = value;
        return;
    }

    bus->interrupt_enable = value;
}

/* Returns the byte in VRAM pointed to by the specified address. */
static uint8_t vram_read(bus_t* bus, bus_master_t master, uint16_t address) {
    uint8_t ppu_mode = bus->io.stat & 2;
    if (master == BUS_MASTER_SM83 && ppu_mode == 3) {
        return 0xFF;
    }

    uint16_t physical_offset = (uint16_t)bus->vram_bank * ADDR_VRAM_SIZE + (address - ADDR_VRAM_BASE);

    return bus->vram[physical_offset];
}

/* Writes a byte into VRAM at the specified address. */
static void vram_write(bus_t* bus, bus_master_t master, uint16_t address, uint8_t value) {
    uint8_t ppu_mode = bus->io.stat & 2;
    if (master == BUS_MASTER_SM83 && ppu_mode == 3) {
        return;
    }

    uint16_t physical_offset = (uint16_t)bus->vram_bank * ADDR_VRAM_SIZE + (address - ADDR_VRAM_BASE);

    bus->vram[physical_offset] = value;
}

/* Returns the byte in WRAM pointed to by the specified address. */
static uint8_t wram_read(bus_t* bus, uint16_t address) {
    uint16_t physical_offset;

    if(address <= ADDR_WRAM0_BASE) {
        physical_offset = address - ADDR_WRAM0_BASE;
    } else {
        physical_offset = (uint16_t)bus->wram_bank * ADDR_WRAMX_SIZE + (address - ADDR_WRAMX_BASE);
    }

    return bus->wram[physical_offset];
}

/* Writes a byte into WRAM at the specified address. */
static void wram_write(bus_t* bus, uint16_t address, uint8_t value) {
    uint16_t physical_offset;

    if(address <= ADDR_WRAM0_BASE) {
        physical_offset = address - ADDR_WRAM0_BASE;
    } else {
        physical_offset = (uint16_t)bus->wram_bank * ADDR_WRAMX_SIZE + (address - ADDR_WRAMX_BASE);
    }

    bus->wram[physical_offset] = value;
}


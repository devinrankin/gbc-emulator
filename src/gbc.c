#include "gbc.h"

void gbc_init(gbc_t* gbc, const char* rom_path) {
    cartridge_t cartridge;
    sm83_t sm83;
    bus_t bus;

    cartridge_init(&cartridge, rom_path);
    sm83_init(&sm83);
    bus_init(&bus, &cartridge);
}


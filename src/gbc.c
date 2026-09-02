#include "gbc.h"

void gbc_init(gbc_t* gbc, const char* rom_path) {
    cartridge_init(&gbc->cartridge, rom_path);
    sm83_init(&gbc->sm83);
    bus_init(&gbc->bus, &gbc->cartridge);
}


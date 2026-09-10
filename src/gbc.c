#include "gbc.h"
#include "mem/cartridge.h"

void gbc_init(gbc_t* gbc, const char* rom_path) {
    cartridge_init(&gbc->cartridge, rom_path);
    bus_init(&gbc->bus, &gbc->cartridge);
    sm83_init(&gbc->sm83, &gbc->bus);
}

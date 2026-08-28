#include <stdio.h>
#include "cartridge.h"

bool cartridge_load_file(cartridge_t* cartridge, const char* path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    long file_size = ftell(file);

    if (file_size < 0 ||
        (unsigned long)file_size > CARTRIDGE_MAX_ROM_SIZE) {
        fclose(file);
        return false;
    }

    rewind(file);

    size_t size = (size_t)file_size;

    if (fread(cartridge->rom, 1, size, file) != size) {
        fclose(file);
        return false;
    }

    fclose(file);

    cartridge->rom_size = size;
    return true;
}

bool cartridge_parse_header(cartridge_t* cartridge) {
    cartridge->header.cgb_flag = cartridge->rom[CARTRIDGE_CGB_FLAG_ADDR];
    cartridge->header.cartridge_type = cartridge->rom[CARTRIDGE_TYPE_ADDR];
    cartridge->header.rom_size_code = cartridge->rom[CARTRIDGE_ROM_SIZE_ADDR];
    cartridge->header.ram_size_code = cartridge->rom[CARTRIDGE_RAM_SIZE_ADDR];
    
    int idx = 0;
    for(uint16_t addr = CARTRIDGE_TITLE_START_ADDR; addr < CARTRIDGE_TITLE_END_ADDR; addr++) {
        cartridge->header.title[idx++] = cartridge->rom[addr];
    }
    cartridge->header.title[idx] = '\0';

    return cartridge_identify_mapper(cartridge);
}

bool cartridge_identify_mapper(cartridge_t* cartridge) {
    uint8_t type = cartridge->header.cartridge_type;

    switch(type) {
        case 0x00:
        case 0x08:
        case 0x09:
            cartridge->map = CARTRIDGE_MAP_NONE;
            return true;
        case 0x01:
        case 0x02:
        case 0x03:
            cartridge->map = CARTRIDGE_MAP_MBC1;
            return true;
        case 0x05:
        case 0x06:
            cartridge->map = CARTRIDGE_MAP_MBC2;
            return true;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            cartridge->map = CARTRIDGE_MAP_MBC3;
            return true;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            cartridge->map = CARTRIDGE_MAP_MBC5;
            return true;

        default:
            cartridge->map = CARTRIDGE_MAP_UNKNOWN;
            return false; 
    }
}

void cartridge_display_info(cartridge_t* cartridge) {
    printf("Displaying cartridge header information: \n");
    printf("\tTitle       : %s\n", cartridge->header.title);
    printf("\tMapper Type : %2.2x\n", cartridge->header.cartridge_type);
    printf("\tROM Size    : %2.2x KiB\n", 32 << cartridge->header.rom_size_code);
    printf("\tRAM Size    : %2.2x\n", cartridge->header.ram_size_code);
}

uint8_t cartridge_read8(cartridge_t* cartridge, uint16_t addr) {
    if(addr < 0x4000) {
        return cartridge->rom[addr];
    }
    NO_IMPL;
}

void cartridge_write8(cartridge_t* cartridge, uint16_t addr, uint8_t value) {
    if(addr < 0x4000) {
        cartridge->rom[addr] = value;
    }
    NO_IMPL
}



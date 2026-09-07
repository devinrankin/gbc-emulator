#include <stdio.h>

#include "cartridge.h"

static bool cartridge_load_rom(uint8_t* rom, const char* path);

static void cartridge_parse_header(cartridge_header_t* header, const uint8_t* rom);

static uint8_t cartridge_header_get_title_len(const uint8_t* rom);
static void cartridge_header_parse_title(cartridge_header_t* header, const uint8_t* rom);

static size_t cartridge_get_ram_size(uint8_t header_value);
static mbc_type_t cartridge_identify_mbc_type(uint8_t type);

void cartridge_init(cartridge_t* cartridge, const char* path) {
    cartridge_load_rom(cartridge->rom, path);
    cartridge_parse_header(&cartridge->header, cartridge->rom);
   
    /* ROM size calculation provided here: https://gbdev.io/pandocs/The_Cartridge_Header.html#0148--rom-size */
    cartridge->rom_size = (32 * SIZE_KIB) * (1 << cartridge->header.rom_size);
    cartridge->ram_size = cartridge_get_ram_size(cartridge->header.ram_size);

    mbc_type_t type = cartridge_identify_mbc_type(cartridge->header.cartridge_type);
    mbc_init(&cartridge->mbc, type, cartridge->rom, cartridge->rom_size, cartridge->ram, cartridge->ram_size);
}

void cartridge_display_info(cartridge_t* cartridge) {
    printf("Displaying cartridge header information: \n");
    printf("\tTitle       : %s\n", cartridge->header.title);
    printf("\tMapper Type : %2.2x\n", cartridge->header.cartridge_type);
    printf("\tROM Size    : %lu B\n", cartridge->rom_size);
    printf("\tRAM Size    : %lu B\n", cartridge->ram_size);
}

/* Returns a byte from the cartridge using the member MBC's read. */
uint8_t cartridge_read(cartridge_t* cartridge, uint16_t address) {
   return cartridge->mbc.read(&cartridge->mbc, address); 
}

/* Writes a byte to the cartridge using the member MBC's write. */
void cartridge_write(cartridge_t* cartridge, uint16_t address, uint8_t value) {
    cartridge->mbc.write(&cartridge->mbc, address, value);
}

/* Attempts to load a ROM at the given filepath. Returns false on a failure. */
static bool cartridge_load_rom(uint8_t* rom, const char* path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    long file_size = ftell(file);

    if (file_size < 0 || (size_t)file_size > CART_MAX_ROM_SIZE) {
        fclose(file);
        return false;
    }

    rewind(file);

    size_t size = (size_t)file_size;

    if (fread(rom, 1, size, file) != size) {
        fclose(file);
        return false;
    }

    fclose(file);
    
    return true;
}

/* Parses ROM's header data and assigns values to the passed cartridge header's respective members. */
static void cartridge_parse_header(cartridge_header_t* header, const uint8_t* rom) {
    cartridge_header_parse_title(header, rom);
    header->cgb_flag = rom[CART_HDR_CGB_FLAG];
    header->new_licensee_code = rom[CART_HDR_NEW_LICENSEE_CODE];
    header->sgb_flag = rom[CART_HDR_SGB_FLAG];
    header->cartridge_type = rom[CART_HDR_CARTRIDGE_TYPE]; 
    header->rom_size = rom[CART_HDR_ROM_SIZE];
    header->ram_size = rom[CART_HDR_RAM_SIZE];
    header->dest_code = rom[CART_HDR_DEST_CODE];
    header->old_licensee_code = rom[CART_HDR_OLD_LICENSEE_CODE];
    header->rom_version = rom[CART_HDR_MASK_ROM_VERSION];
    header->checksum = rom[CART_HDR_HEADER_CHECKSUM];
    header->global_checksum = (uint16_t)((rom[CART_HDR_GLOBAL_CHECKSUM] << 8) | rom[CART_HDR_GLOBAL_CHECKSUM + 1]);
}

/* Since the CGB flag address is part of the title on DMG systems, we need to check its value
 * to determine the length of the title. */
static uint8_t cartridge_header_get_title_len(const uint8_t* rom) {
    uint8_t cgb_flag = rom[CART_HDR_CGB_FLAG];

    if (cgb_flag == CGB_FLAG_COMPATIBLE || cgb_flag == CGB_FLAG_REQUIRED) {
        return 11;
    }
    return 16;
}

/* Build the ROM title from the designated title header address space. */
static void cartridge_header_parse_title(cartridge_header_t* header, const uint8_t* rom) {
    uint8_t title_len = cartridge_header_get_title_len(rom);

    int idx = 0;
    for (uint16_t addr = CART_HDR_TITLE; addr < CART_HDR_TITLE + title_len - 1; addr++) {
        header->title[idx++] = rom[addr];
    }
    header->title[idx] = '\0';
}

static size_t cartridge_get_ram_size(uint8_t header_value) {
    switch (header_value) {
        case 0x00:
            return 0;
        case 0x02:
            return (8 * SIZE_KIB);
        case 0x03:
            return (32 * SIZE_KIB);
        case 0x04:
            return (128 * SIZE_KIB);
        case 0x05:
            return (64 * SIZE_KIB);
        default:
            NO_IMPL;
    }
}

static mbc_type_t cartridge_identify_mbc_type(uint8_t type) {
    switch (type) {
        case 0x00:
        case 0x08:
        case 0x09:
            return MBC_TYPE_NONE;
        case 0x01:
        case 0x02:
        case 0x03:
            return MBC_TYPE_MBC1;
        case 0x05:
        case 0x06:
            return MBC_TYPE_MBC2;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return MBC_TYPE_MBC3;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            return MBC_TYPE_MBC5;
        default:
            return MBC_TYPE_NONE;
    }
}

#include <stdio.h>

#include "cartridge.h"

static uint8_t cartridge_title_len(cartridge_t* cartridge);
static void cartridge_parse_title(cartridge_t* cartridge);
static void cartridge_parse_header(cartridge_t* cartridge);
static mbc_type_t cartridge_identify_mbc_type(uint8_t type);
static bool cartridge_load_file(cartridge_t* cartridge, const char* path);

void cartridge_init(cartridge_t* cartridge, const char* path) {
    cartridge_load_file(cartridge, path);
    cartridge_parse_header(cartridge);
    
    mbc_init(&cartridge->mbc, cartridge_identify_mbc_type(cartridge->header.cartridge_type), cartridge->rom, cartridge->rom_size, cartridge->ram, cartridge->ram_size);
}

void cartridge_display_info(cartridge_t* cartridge) {
    printf("Displaying cartridge header information: \n");
    printf("\tTitle       : %s\n", cartridge->header.title);
    printf("\tMapper Type : %2.2x\n", cartridge->header.cartridge_type);
    printf("\tROM Size    : %2.2x KiB\n", 32 << cartridge->header.rom_size);
    printf("\tRAM Size    : %2.2x\n", cartridge->header.ram_size);
}

uint8_t cartridge_read8(cartridge_t* cartridge, uint16_t address) {
   return cartridge->mbc.read(&cartridge->mbc, address); 
}

void cartridge_write8(cartridge_t* cartridge, uint16_t address, uint8_t value) {
    cartridge->mbc.write(&cartridge->mbc, address, value);
}

static uint8_t cartridge_title_len(cartridge_t* cartridge) {
    uint8_t cgb_flag = cartridge->rom[CART_HDR_CGB_FLAG];

    if(cgb_flag == CART_CGB_FLAG_CGB_SUPPORTED || cgb_flag == CART_CGB_FLAG_CGB_ONLY) {
        return 11;
    }
    return 16;
}

static void cartridge_parse_title(cartridge_t* cartridge) {
    uint8_t title_len = cartridge_title_len(cartridge);

    int idx = 0;
    for(uint16_t addr = CART_HDR_TITLE; addr < CART_HDR_TITLE + title_len - 1; addr++) {
        cartridge->header.title[idx++] = cartridge->rom[addr];
    }
    cartridge->header.title[idx] = '\0';
}

static mbc_type_t cartridge_identify_mbc_type(uint8_t type) {
    switch(type) {
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

static void cartridge_parse_header(cartridge_t* cartridge) {
    uint8_t* rom = cartridge->rom;

    cartridge_parse_title(cartridge);
    cartridge->header.cgb_flag = rom[CART_HDR_CGB_FLAG];
    cartridge->header.new_licensee_code = rom[CART_HDR_NEW_LICENSEE_CODE];
    cartridge->header.sgb_flag = rom[CART_HDR_SGB_FLAG];
    cartridge->header.cartridge_type = rom[CART_HDR_CARTRIDGE_TYPE]; 
    cartridge->header.rom_size = rom[CART_HDR_ROM_SIZE];
    cartridge->header.ram_size = rom[CART_HDR_RAM_SIZE];
    cartridge->header.dest_code = rom[CART_HDR_DEST_CODE];
    cartridge->header.old_licensee_code = rom[CART_HDR_OLD_LICENSEE_CODE];
    cartridge->header.rom_version = rom[CART_HDR_MASK_ROM_VERSION];
    cartridge->header.checksum = rom[CART_HDR_HEADER_CHECKSUM];
    cartridge->header.global_checksum = (uint16_t)((rom[CART_HDR_GLOBAL_CHECKSUM] << 8) | rom[CART_HDR_GLOBAL_CHECKSUM + 1]);
}

static bool cartridge_load_file(cartridge_t* cartridge, const char* path) {
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
        (unsigned long)file_size > CART_MAX_ROM_SIZE) {
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


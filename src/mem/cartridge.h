#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../common.h"
#include "mbc.h"

#define CART_MAX_ROM_SIZE (8 * SIZE_MIB)
#define CART_MAX_RAM_SIZE (32 * SIZE_KIB)

#define CART_MAX_TITLE_LEN 16

#define CART_HDR_ENTRY_POINT 0x0100
#define CART_HDR_TITLE 0x0134
#define CART_HDR_MANUFACTURER_CODE 0x013F
#define CART_HDR_CGB_FLAG 0x0143
#define CART_HDR_NEW_LICENSEE_CODE 0x0144
#define CART_HDR_SGB_FLAG 0x0146
#define CART_HDR_CARTRIDGE_TYPE 0x0147
#define CART_HDR_ROM_SIZE 0x0148
#define CART_HDR_RAM_SIZE 0x0149
#define CART_HDR_DEST_CODE 0x014A
#define CART_HDR_OLD_LICENSEE_CODE 0x014B
#define CART_HDR_MASK_ROM_VERSION 0x014C
#define CART_HDR_HEADER_CHECKSUM 0x014D
#define CART_HDR_GLOBAL_CHECKSUM 0x014E

#define CART_ROM_SIZE_32_KIB 0x00
#define CART_ROM_SIZE_64_KIB 0x01
#define CART_ROM_SIZE_128_KIB 0x02
#define CART_ROM_SIZE_256_KIB 0x03
#define CART_ROM_SIZE_512_KIB 0x04
#define CART_ROM_SIZE_1_MIB 0x05
#define CART_ROM_SIZE_2_MIB 0x06
#define CART_ROM_SIZE_4_MIB 0x07
#define CART_ROM_SIZE_8_MIB 0x08

#define CART_RAM_SIZE_NONE 0x00
#define CART_RAM_SIZE_8_KIB 0x02
#define CART_RAM_SIZE_32_KIB 0x03
#define CART_RAM_SIZE_128_KIB 0x04
#define CART_RAM_SIZE_64_KIB 0x05

#define CGB_FLAG_COMPATIBLE 0x80
#define CGB_FLAG_REQUIRED 0xC0

typedef enum {
    CARTRIDGE_DMG_ONLY,
    CARTRIDGE_CGB_COMPATIBLE,
    CARTRIDGE_CGB_ONLY
} cartridge_cgb_support_t;

typedef struct {
    char title[CART_MAX_TITLE_LEN];
    uint8_t cgb_flag;
    uint8_t new_licensee_code;
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t dest_code;
    uint8_t old_licensee_code;
    uint8_t rom_version;
    uint8_t checksum;
    uint16_t global_checksum;
} cartridge_header_t;

typedef struct cartridge {
    cartridge_header_t header;
    mbc_t mbc;

    uint8_t rom[CART_MAX_ROM_SIZE];
    uint8_t ram[CART_MAX_RAM_SIZE];
    
    size_t rom_size;
    size_t ram_size;

    cartridge_cgb_support_t mode;
} cartridge_t;

void cartridge_init(cartridge_t* cartridge, const char* path);
void cartridge_display_info(cartridge_t* cartridge);

uint8_t cartridge_read(cartridge_t* cartridge, uint16_t address);
void cartridge_write(cartridge_t* cartridge, uint16_t address, uint8_t value);

#endif

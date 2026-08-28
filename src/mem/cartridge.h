#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

#define CARTRIDGE_MAX_ROM_SIZE (8 * 1024 * 1024)
#define CARTRIDGE_MAX_RAM_SIZE (32 * 1024)

#define CARTRIDGE_TITLE_START_ADDR 0x134
#define CARTRIDGE_TITLE_END_ADDR 0x142
#define CARTRIDGE_CGB_FLAG_ADDR 0x143
#define CARTRIDGE_TYPE_ADDR 0x147
#define CARTRIDGE_ROM_SIZE_ADDR 0x148
#define CARTRIDGE_RAM_SIZE_ADDR 0x149

typedef enum {
    CARTRIDGE_MAP_NONE,
    CARTRIDGE_MAP_MBC1,
    CARTRIDGE_MAP_MBC2,
    CARTRIDGE_MAP_MBC3,
    CARTRIDGE_MAP_MBC5,
    CARTRIDGE_MAP_UNKNOWN
} cartridge_map_t;

typedef struct {
    char title[16];
    uint8_t cgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size_code;
    uint8_t ram_size_code;
    uint8_t dest_code;
} cartridge_header_t;

typedef struct {
    cartridge_header_t header;
    uint8_t rom[CARTRIDGE_MAX_ROM_SIZE];
    uint8_t ram[CARTRIDGE_MAX_RAM_SIZE];

    size_t rom_size;
    size_t ram_size;
    cartridge_map_t map;

    uint8_t rom_bank;
    uint8_t ram_bank;
    bool ram_enabled;
} cartridge_t;

bool cartridge_load_file(cartridge_t* cartridge, const char* path);
bool cartridge_parse_header(cartridge_t* cartridge);
bool cartridge_identify_mapper(cartridge_t* cartridge);
void cartridge_display_info(cartridge_t* cartridge);

uint8_t cartridge_read8(cartridge_t* cartridge, uint16_t addr);
void cartridge_write8(cartridge_t* cartridge, uint16_t addr, uint8_t value);
uint16_t cartridge_read16(cartridge_t* cartridge, uint16_t addr);
#endif

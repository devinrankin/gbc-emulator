#ifndef MBC_H
#define MBC_H

#include <stdint.h>
#include <stddef.h>

typedef enum mbc_type {
    MBC_TYPE_NONE,
    MBC_TYPE_MBC1,
    MBC_TYPE_MBC2,
    MBC_TYPE_MBC3,
    MBC_TYPE_MBC5,
    MBC_TYPE_MBC6,
    MBC_TYPE_MBC7,
    MBC_TYPE_MMM01,
    MBC_TYPE_M161,
    MBC_TYPE_HUC1,
    MBC_TYPE_HUC3,
    MBC_TYPE_OTHER
} mbc_type_t;

typedef struct mbc mbc_t;

struct mbc {
    mbc_type_t type;

    const uint8_t* rom;
    uint8_t* ram;
   
    size_t rom_size;
    size_t ram_size;
    
    uint8_t ram_enabled;
    uint16_t rom_bank;
    uint8_t ram_bank;

    uint8_t (*read)(mbc_t* mbc, uint16_t address);
    void (*write)(mbc_t* mbc, uint16_t address, uint8_t value);

    uint8_t banking_mode;
    uint8_t bank_low;
    uint8_t bank_high;
};

void mbc_init(mbc_t* mbc, mbc_type_t type, const uint8_t* rom, size_t rom_size, uint8_t* ram, size_t ram_size);
uint8_t mbc_read(mbc_t* mbc, uint16_t address);
void mbc_write(mbc_t* mbc, uint16_t address, uint8_t value);

#endif

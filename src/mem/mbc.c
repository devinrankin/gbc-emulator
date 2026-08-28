#include "mbc.h"

static uint8_t mbc0_read(mbc_t* mbc, uint16_t addr);
static void mbc0_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc1_read(mbc_t* mbc, uint16_t addr);
static void mbc1_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc2_read(mbc_t* mbc, uint16_t addr);
static void mbc2_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc3_read(mbc_t* mbc, uint16_t addr);
static void mbc3_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc5_read(mbc_t* mbc, uint16_t addr);
static void mbc5_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc6_read(mbc_t* mbc, uint16_t addr);
static void mbc6_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc7_read(mbc_t* mbc, uint16_t addr);
static void mbc7_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t read_rom(mbc_t* mbc, size_t offset) {
    return mbc->rom[offset];
}

static uint8_t read_ram(mbc_t* mbc, size_t offset) {
    return mbc->ram[offset];
}

static void write_ram(mbc_t* mbc, size_t offset, uint8_t value) {
    mbc->ram[offset] = value;
}

void mbc_init(mbc_t* mbc, mbc_type_t type, uint8_t* rom, size_t rom_size, uint8_t* ram, size_t ram_size) {
    mbc->type = type;
    mbc->rom = rom;
    mbc->rom_size = rom_size;
    mbc->ram = ram;
    mbc->ram_size = ram_size;

    mbc->rom_bank = 1;
    mbc->ram_bank = 0;
    mbc->ram_enabled = false;

    switch(type) {
        case MBC_TYPE_NONE:
            mbc->read = mbc0_read;
            mbc->write = mbc0_write;
            break;
        case MBC_TYPE_MBC1:
            mbc->read = mbc1_read;
            mbc->write = mbc1_write;
            break;
        case MBC_TYPE_MBC2:
            mbc->read = mbc2_read;
            mbc->write = mbc2_write;
            break;
        case MBC_TYPE_MBC3:
            mbc->read = mbc3_read;
            mbc->write = mbc3_write;
            break;
        case MBC_TYPE_MBC5:
            mbc->read = mbc5_read;
            mbc->write = mbc5_write;
            break;
        case MBC_TYPE_MBC6:
            mbc->read = mbc6_read;
            mbc->write = mbc6_write;
            break;
        case MBC_TYPE_MBC7:
            mbc->read = mbc7_read;
            mbc->write = mbc7_write;
            break;
        default:
            mbc->read = mbc0_read;
            mbc->write = mbc0_write;
            break;
    }
}

uint8_t mbc_read(mbc_t* mbc, uint16_t address) {
    return mbc->read(mbc, address);
}

void mbc_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    mbc->write(mbc, address, value);
}

static uint8_t mbc0_read(mbc_t* mbc, uint16_t address) {
    if(address < 0x8000) {
        return read_rom(mbc, address);  
    }

    if(address >= 0xA000 && address < 0xC000) {
        return read_ram(mbc, address - 0xA000);
    }

    return 0xFF;
}

static void mbc0_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    (void)mbc;
    (void)address;
    (void)value;
}




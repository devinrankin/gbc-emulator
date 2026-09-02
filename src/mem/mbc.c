#include "mbc.h"

#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

static uint8_t mbc0_read(mbc_t* mbc, uint16_t address);
static void mbc0_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc1_read(mbc_t* mbc, uint16_t address);
static void mbc1_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc2_read(mbc_t* mbc, uint16_t addr);
static void mbc2_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc3_read(mbc_t* mbc, uint16_t address);
static void mbc3_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc5_read(mbc_t* mbc, uint16_t address);
static void mbc5_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc6_read(mbc_t* mbc, uint16_t address);
static void mbc6_write(mbc_t* mbc, uint16_t address, uint8_t value);

static uint8_t mbc7_read(mbc_t* mbc, uint16_t address);
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

void mbc_init(mbc_t* mbc, mbc_type_t type, const uint8_t* rom, size_t rom_size, uint8_t* ram, size_t ram_size) {
    mbc->type = type;
    mbc->rom = rom;
    mbc->rom_size = rom_size;
    mbc->ram = ram;
    mbc->ram_size = ram_size;

    mbc->bank_low = 1;
    mbc->bank_high = 0;
    mbc->ram_enabled = 0;
    mbc->banking_mode = 0;

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

static uint8_t mbc1_read(mbc_t* mbc, uint16_t address) {
    size_t bank;
    size_t offset;

    if(address < 0x4000) {
        if(mbc->banking_mode == 0) 
            bank = 0;
        else
            bank = (size_t)(mbc->bank_high & 0x03) << 5;

        offset = bank * ROM_BANK_SIZE + address;
        return read_rom(mbc, offset);
    }

    if(address < 0x8000) {
        bank = ((size_t)(mbc->bank_high & 0x03) << 5) | (mbc->bank_low & 0x1F);

        if((bank & 0x1F) == 0)
            bank++;

        offset = bank * ROM_BANK_SIZE + (address - 0x4000);
        return read_rom(mbc, offset);
    }

    if(address >= 0xA000 && address < 0xC000) {
        if(!mbc->ram_enabled)
            return 0xFF;

        if(mbc->banking_mode == 0)
            bank = 0;
        else
            bank = mbc->bank_high & 0x03;
        
        offset = bank * RAM_BANK_SIZE + (address - 0xA000);
        return read_ram(mbc, offset);
    }

    return 0xFF;
}

static void mbc1_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    if(address < 0x2000) {
        mbc->ram_enabled = ((value & 0x0F) == 0x0A);
        return;
    }

    if(address < 0x4000) {
        mbc->bank_low = (value & 0x1F);
        return;
    }

    if(address < 0x6000) {
        mbc->bank_high = value & 0x03;
        return;
    }

    if (address < 0x8000) {
        mbc->banking_mode = value & 0x01;
        return;
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!mbc->ram_enabled)
            return;

        size_t bank = 0;

        if(mbc->banking_mode == 1)
            bank = mbc->bank_high & 0x03;

        size_t offset = bank * RAM_BANK_SIZE + (address - 0xA000);

        write_ram(mbc, offset, value);
        return;
    }
}

static uint8_t mbc2_read(mbc_t* mbc, uint16_t address) {
    size_t bank;
    size_t offset;

    if(address < 0x4000) {
        offset = address;
        return read_rom(mbc, offset);
    }

    if(address < 0x8000) {
        bank = mbc->rom_bank & 0x0F;

        if(bank == 0) {
            bank = 1;
        }

        offset = bank * ROM_BANK_SIZE + (address - 0x4000);
        return read_rom(mbc, offset);
    }

    if(address >= 0xA000 && address < 0xC000) {
        if(!mbc->ram_enabled) {
            return 0xFF;
        }

        offset = (address - 0xA000) & 0x01FF;

        return 0xF0 | read_ram(mbc, offset);
    }

    return 0xFF;
}

static void mbc2_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    if(address < 0x4000) {
        /* Masking the least significant bit of the upper address byte */
        if((address & 0x0100) == 0) {
            /* Ram is enabled if and only if the lower 4 bits of value are $A */
            mbc->ram_enabled = ((value & 0x0F) == 0x0A);
        } else {
            mbc->bank_low = value & 0x0F;
        }

        return;
    }

    if(address >= 0xA000 && address < 0xC000) {
        if(!mbc->ram_enabled) {
            return;
        }

        size_t offset = (address - 0xA000) & 0x01FF;
        write_ram(mbc, offset, value & 0x0F);
    }
}

static uint8_t mbc3_read(mbc_t* mbc, uint16_t address) {
    (void)mbc;
    (void)address;
   return 0;
}
static void mbc3_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    (void)mbc;
    (void)address;
    (void)value;
}


static uint8_t mbc5_read(mbc_t* mbc, uint16_t address) {
    (void)mbc;
    (void)address;
   return 0;
}
static void mbc5_write(mbc_t* mbc, uint16_t address, uint8_t value) {
     (void)mbc;
    (void)address;
    (void)value;   
}

static uint8_t mbc6_read(mbc_t* mbc, uint16_t address) {
    (void)mbc;
    (void)address;
   return 0;
}
static void mbc6_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    (void)mbc;
    (void)address;
    (void)value;
}

static uint8_t mbc7_read(mbc_t* mbc, uint16_t address) {
    (void)mbc;
    (void)address;
   return 0;
}
static void mbc7_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    (void)mbc;
    (void)address;
    (void)value;
}



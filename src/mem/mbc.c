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

static uint8_t rom_read(mbc_t* mbc, size_t offset);
static uint8_t ram_read(mbc_t* mbc, size_t offset);
static void ram_write(mbc_t* mbc, size_t offset, uint8_t value);

static uint8_t rtc_read(mbc_t* mbc, uint8_t register_id);
static void rtc_write(mbc_t* mbc, uint8_t value);
static void rtc_update(mbc_t* mbc, uint64_t now);

void mbc_init(mbc_t* mbc, mbc_type_t type, const uint8_t* rom, uint8_t* ram) {
    *mbc = (mbc_t){ 
        mbc->type = type,
        mbc->rom = rom,
        mbc->ram = ram
    };

    switch(type) {
        case MBC_TYPE_NONE:
            mbc->read = mbc0_read;
            mbc->write = mbc0_write;
            break;
        case MBC_TYPE_MBC1:
            mbc->read = mbc1_read;
            mbc->write = mbc1_write;

            mbc->state.mbc1.rom_bank_low = 1;
            mbc->state.mbc1.banking_mode = 0;
            break;
        case MBC_TYPE_MBC2:
            mbc->read = mbc2_read;
            mbc->write = mbc2_write;

            mbc->state.mbc2.rom_bank = 1;
            break;
        case MBC_TYPE_MBC3:
            mbc->read = mbc3_read;
            mbc->write = mbc3_write;

            mbc->state.mbc3.rom_bank = 1;
            break;
        case MBC_TYPE_MBC5:
            mbc->read = mbc5_read;
            mbc->write = mbc5_write;

            mbc->state.mbc5.rom_bank = 1;
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
    if (address < 0x8000) {
        return rom_read(mbc, address);  
    }

    if (address >= 0xA000 && address < 0xC000) {
        return ram_read(mbc, address - 0xA000);
    }

    return 0xFF;
}

static void mbc0_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    (void)mbc;
    (void)address;
    (void)value;
}

static uint8_t mbc1_read(mbc_t* mbc, uint16_t address) {
    mbc1_state_t *state = &mbc->state.mbc1;

    size_t bank;
    size_t offset;

    if (address < 0x4000) {
        if (state->banking_mode == 0) 
            bank = 0;
        else
            bank = (size_t)(state->bank_high & 0x03) << 5;

        offset = bank * ROM_BANK_SIZE + address;
        return rom_read(mbc, offset);
    }

    if (address < 0x8000) {
        bank = ((size_t)(state->bank_high & 0x03) << 5) | (state->rom_bank_low & 0x1F);

        if ((bank & 0x1F) == 0)
            bank++;

        offset = bank * ROM_BANK_SIZE + (address - 0x4000);
        return rom_read(mbc, offset);
    }

    if (address >= 0xA000 && address < 0xC000) {
        if (!state->ram_enabled)
            return 0xFF;

        if (state->banking_mode == 0)
            bank = 0;
        else
            bank = state->bank_high & 0x03;
        
        offset = bank * RAM_BANK_SIZE + (address - 0xA000);
        return ram_read(mbc, offset);
    }

    return 0xFF;
}

static void mbc1_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    mbc1_state_t *state = &mbc->state.mbc1;
    
    if (address < 0x2000) {
        state->ram_enabled = ((value & 0x0F) == 0x0A);
        return;
    }

    if (address < 0x4000) {
        state->rom_bank_low = (value & 0x1F);
        return;
    }

    if (address < 0x6000) {
        state->bank_high = value & 0x03;
        return;
    }

    if (address < 0x8000) {
        state->banking_mode = value & 0x01;
        return;
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!state->ram_enabled)
            return;

        size_t bank = 0;

        if(state->banking_mode == 1)
            bank = state->bank_high & 0x03;

        size_t offset = bank * RAM_BANK_SIZE + (address - 0xA000);

        ram_write(mbc, offset, value);
        return;
    }
}

static uint8_t mbc2_read(mbc_t* mbc, uint16_t address) {
    mbc2_state_t *state = &mbc->state.mbc2;

    size_t bank;
    size_t offset;

    if (address < 0x4000) {
        offset = address;
        return rom_read(mbc, offset);
    }

    if (address < 0x8000) {
        bank = state->rom_bank & 0x0F;

        if(bank == 0) {
            bank = 1;
        }

        offset = bank * ROM_BANK_SIZE + (address - 0x4000);
        return rom_read(mbc, offset);
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!state->ram_enabled) {
            return 0xFF;
        }

        offset = (address - 0xA000) & 0x01FF;

        return 0xF0 | (ram_read(mbc, offset) & 0x0F);
    }

    return 0xFF;
}

static void mbc2_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    mbc2_state_t *state = &mbc->state.mbc2;

    if (address < 0x4000) {
        /* Masking the least significant bit of the upper address byte */
        if ((address & 0x0100) == 0) {
            /* Ram is enabled if and only if the lower 4 bits of value are $A */
            state->ram_enabled = ((value & 0x0F) == 0x0A);
        } else {
            state->rom_bank = value & 0x0F;
        }

        return;
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!state->ram_enabled) {
            return;
        }

        size_t offset = (address - 0xA000) & 0x01FF;
        ram_write(mbc, offset, value & 0x0F);
    }
}

static uint8_t mbc3_read(mbc_t* mbc, uint16_t address) {
    mbc3_state_t *state = &mbc->state.mbc3;
    
    size_t bank;
    size_t offset;

    if (address < 0x4000) {
        return rom_read(mbc, address);
    }

    if (address < 0x8000) {
        bank = state->rom_bank & 0x7F;

        if(bank == 0) {
            bank = 1;
        }

        offset = bank * ROM_BANK_SIZE + (address - 0x4000);

        return rom_read(mbc, offset);
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!state->ram_enabled) {
            return 0xFF;
        }

        switch (state->ram_rtc_select) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
                offset = (size_t)state->ram_rtc_select * RAM_BANK_SIZE + (address - 0xA000);
                return ram_read(mbc, offset);
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
                return rtc_read(mbc, state->ram_rtc_select);
            default:
                return 0xFF;
        }
    }
    return 0xFF;
}

static void mbc3_write(mbc_t* mbc, uint16_t address, uint8_t value) {
    mbc3_state_t *state = &mbc->state.mbc3;
    
    if (address < 0x2000) {
        state->ram_enabled = ((value & 0x0F) == 0x0A);
        return;
    }

    if (address < 0x4000) {
        state->rom_bank = value & 0x7F;
        return;
    }

    if (address < 0x6000) {
        state->ram_rtc_select = value;
        return;
    }

    if (address < 0x8000) {
        /* TODO: implement RTC latch */
        return;
    }

    if (address >= 0xA000 && address < 0xC000) {
        if(!state->ram_enabled) {
            return;
        }
        
        switch(state->ram_rtc_select) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03: {
                size_t offset = (size_t)state->ram_rtc_select * RAM_BANK_SIZE + (address - 0xA000);
                ram_write(mbc, offset, value);
                return;
            }
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
                rtc_write(mbc, value); 
                return;
            default:
                return;
        }
    }
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

static uint8_t rom_read(mbc_t* mbc, size_t offset) {
    return mbc->rom[offset];
}

static uint8_t ram_read(mbc_t* mbc, size_t offset) {
    return mbc->ram[offset];
}

static void ram_write(mbc_t* mbc, size_t offset, uint8_t value) {
    mbc->ram[offset] = value;
}

static uint8_t rtc_read(mbc_t* mbc, uint8_t register_id) {
    mbc3_state_t *state = &mbc->state.mbc3;
    
    const struct rtc* rtc = state->rtc_latched ? &state->latched_rtc : &state->current_rtc;

    switch(register_id) {
        case 0x08: return rtc->seconds;
        case 0x09: return rtc->minutes;
        case 0x0A: return rtc->hours;
        case 0x0B: return rtc->days_low;
        case 0x0C: return rtc->days_high;
        default: return 0xFF;
    }
}

static void rtc_write(mbc_t* mbc, uint8_t value) {
    mbc3_state_t *state = &mbc->state.mbc3;

    if (state->rtc_latch_value == 0x00 && value == 0x01) {
        state->latched_rtc = state->current_rtc;
    }

    state->rtc_latch_value = value;
}

static void rtc_update(mbc_t* mbc, uint64_t now) {
    mbc3_state_t *state = &mbc->state.mbc3;

    uint64_t elapsed;
    uint16_t days;
    
    if (state->last_timestamp == 0) {
        state->last_timestamp = now;
        return;
    }

    if (state->current_rtc.days_high & 0x40) {
        state->last_timestamp = now;
        return;
    }

    elapsed = now - state->last_timestamp;
    if (elapsed <= 0) {
        return;
    }

    state->last_timestamp = now;

    while(elapsed >= 86400) {
        elapsed -= 86400;

        days = ((state->current_rtc.days_high & 1) << 8) | state->current_rtc.days_low;

        if (days == 511) {
            days = 0;
            state->current_rtc.days_high |= 0x80;
        } else {
            days++;
        }

        state->current_rtc.days_low = days & 0xFF;
        state->current_rtc.days_high = (state->current_rtc.days_high & 0xFE) | ((days >> 8) & 1);
    }

    while (elapsed >= 3600) {
        elapsed -= 3600;
        state->current_rtc.hours++;
    }

    while (state->current_rtc.hours >= 24) {
        state->current_rtc.hours -= 24;
        state->current_rtc.days_low++;
    
        uint16_t days = ((state->current_rtc.days_high & 1) << 8) | state->current_rtc.days_low;

        if(days > 511) {
            days = 0;
            state->current_rtc.days_high |= 0x80;
        }

        state->current_rtc.days_low = days & 0xFF;
        state->current_rtc.days_high = (state->current_rtc.days_high & 0xFE) | ((days >> 8) & 1);
    }

    state->current_rtc.minutes += elapsed / 60;
    elapsed %= 60;
    
    if(state->current_rtc.minutes >= 60) {
        state->current_rtc.minutes -= 60;
        state->current_rtc.hours++;
    }

    state->current_rtc.seconds += elapsed;

    if(state->current_rtc.seconds >= 60) {
        state->current_rtc.seconds -= 60;
        state->current_rtc.minutes++;
    }
}

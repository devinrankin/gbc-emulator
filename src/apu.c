#include "apu.h"

#define APU_ADDR_SOUND_START 0xFF10
#define APU_ADDR_SOUND_END 0xFF26

#define APU_ADDR_WAVERAM_START 0xFF30
#define APU_ADDR_WAVERAM_END 0xFF3F

uint8_t apu_read(apu_t* apu, uint16_t address) {
    if (RANGE(address, APU_ADDR_SOUND_START, APU_ADDR_SOUND_END)) {
        return apu->sound_registers[address - APU_ADDR_SOUND_START];
    } 

    if (RANGE(address, APU_ADDR_WAVERAM_START, APU_ADDR_WAVERAM_END)) {
        return apu->wave_ram[address - APU_ADDR_WAVERAM_START];
    }

    return 0xFF;
}

void apu_write(apu_t* apu, uint16_t address, uint8_t value) {
     if (RANGE(address, APU_ADDR_SOUND_START, APU_ADDR_SOUND_END)) {
        apu->sound_registers[address - APU_ADDR_SOUND_START] = value;
        return;
    } 

    if (RANGE(address, APU_ADDR_WAVERAM_START, APU_ADDR_WAVERAM_END)) {
        apu->wave_ram[address - APU_ADDR_WAVERAM_START] = value;
        return;
    } 
}

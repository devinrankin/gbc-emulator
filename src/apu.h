#ifndef APU_H
#define APU_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

#define APU_SOUND_REG_COUNT 0x15
#define APU_WAVE_RAM_SIZE 0x0F

typedef struct {
    uint8_t sound_registers[APU_SOUND_REG_COUNT];
    uint8_t wave_ram[APU_WAVE_RAM_SIZE];
} apu_t;

uint8_t apu_read(apu_t* apu, uint16_t address);
void apu_write(apu_t* apu, uint16_t address, uint8_t value);

#endif

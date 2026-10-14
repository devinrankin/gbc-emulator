#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "../common.h"
#include "ppu_fifo.h"
#include "../bus.h"

#define PPU_MODE2_DOTS 80
#define PPU_LINE_DOTS 456 

#define PPU_LY_VBLANK 144
#define PPU_LY_BOTTOM 154

#define PPU_XRES 160
#define PPU_YRES 144

typedef enum {
    PPU_MODE2,
    PPU_MODE3,
    PPU_MODE0,
    PPU_MODE1
} ppu_mode_t;

typedef enum {
    ADDR_MODE_8000,
    ADDR_MODE_8800
} ppu_tile_addr_mode_t;

typedef enum {
    FETCH_TILE,
    FETCH_LOW,
    FETCH_HIGH,
    FETCH_SLEEP,
    FETCH_PUSH,
} ppu_fetch_state_t;

typedef struct ppu {
    bus_t* bus;

    ppu_mode_t mode;

    ppu_pixel_t framebuffer[PPU_YRES * PPU_XRES];
    uint16_t line_dots;
    uint16_t xpos;
    uint8_t discard_pixels;

    bool y_cond;
    bool window_active;
    uint8_t window_line;

    ppu_fifo_t bg_fifo;
    ppu_fifo_t obj_fifo;

    ppu_fetch_state_t fetch_state;
    uint8_t fetch_dots;
    uint8_t fetch_tile;
    uint8_t fetch_tile_attr;
    uint8_t fetch_row;
    uint8_t fetch_low;
    uint8_t fetch_high;
    uint8_t fetch_tile_x;
} ppu_t;

void ppu_init(ppu_t* ppu, bus_t* bus);
void ppu_step(ppu_t* ppu);

uint8_t ppu_read(ppu_t* ppu, uint16_t address);
void ppu_write(ppu_t* ppu, uint16_t address, uint8_t value);

#endif

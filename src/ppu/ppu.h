#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "../common.h"
#include "ppu_fifo.h"
#include "../bus.h"

#define PPU_XRES 160
#define PPU_YRES 144

/* VRAM tile map & data locations */
#define ADDR_VRAM_TILEDATA_BLOCK0_BASE 0x8000
#define ADDR_VRAM_TILEDATA_BLOCK1_BASE 0x8800
#define ADDR_VRAM_TILEDATA_BLOCK2_BASE 0x9000
#define ADDR_VRAM_TILEDATA_BLOCK_SIZE 0x800

#define ADDR_VRAM_TILEMAP0_BASE 0x9800
#define ADDR_VRAM_TILEMAP1_BASE 0x9C00
#define ADDR_VRAM_TILEMAP_SIZE 0x0400

#define OBJ_BUF_CAPACITY 10

#define PPU_NUM_CGB_PALETTES 8u
#define PPU_BYTES_PER_PALETTE 8u

typedef enum {
    PPU_MODE_OAM,
    PPU_MODE_DRAW,
    PPU_MODE_HBLANK,
    PPU_MODE_VBLANK
} ppu_mode_t;

typedef enum {
    FETCH_TILE,
    FETCH_LOW,
    FETCH_HIGH,
    FETCH_SLEEP,
    FETCH_PUSH,
} ppu_fetch_state_t;

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attr;
    uint8_t oam_index;
} ppu_obj_t;

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

    ppu_fifo_t bgw_fifo;
    ppu_fifo_t obj_fifo;

    uint8_t oam_scan_index;
    uint8_t obj_count;
    uint8_t next_obj;
    ppu_obj_t obj_buffer[OBJ_BUF_CAPACITY];

    uint8_t bgw_palettes[PPU_NUM_CGB_PALETTES * PPU_BYTES_PER_PALETTE];
    uint8_t obj_palettes[PPU_NUM_CGB_PALETTES * PPU_BYTES_PER_PALETTE];

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
void dump_framebuffer(ppu_t* ppu);

#endif

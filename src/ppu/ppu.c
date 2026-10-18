#include "ppu.h"
#include "ppu_fifo.h"

#define VRAM_TILEMAP0_ADDR 0x9800u
#define VRAM_TILEMAP1_ADDR 0x9C00u
#define VRAM_TILEDATA_UNSIGNED_ADDR 0x8000u
#define VRAM_TILEDATA_SIGNED_ADDR 0x9000u

/* LCDC bits */
#define LCDC_BGW_ENPRI 0u
#define LCDC_OBJ_EN 1u
#define LCDC_OBJ_SIZE 2u
#define LCDC_BG_TILE_MAP_AREA 3u
#define LCDC_BGW_TILE_DATA_AREA 4u
#define LCDC_WIN_EN 5u
#define LCDC_WIN_TILE_MAP_AREA 6u
#define LCDC_LCD_PPU_EN 7u

/* VRAM tile map & data locations */
#define ADDR_VRAM_TILEDATA_BLOCK0_START
#define ADDR_VRAM_TILEDATA_BLOCK0_END
#define ADDR_VRAM_TILEDATA_BLOCK1_START
#define ADDR_VRAM_TILEDATA_BLOCK1_END
#define ADDR_VRAM_TILEDATA_BLOCK2_START
#define ADDR_VRAM_TILEDATA_BLOCK2_END
#define ADDR_VRAM_TILEMAP0_START 0x9800
#define ADDR_VRAM_TILEMAP0_END 0x9BFF
#define ADDR_VRAM_TILEMAP1_START 0x9C00
#define ADDR_VRAM_TILEMAP1_END 0x9FFF

#define MAP_ROWS 32u

#define DATA_TILE_SIZE 16u
#define DATA_ROW_SIZE 2u
#define DATA_ROW_MASK 0x7

static uint8_t ppu_read(ppu_t* ppu, uint16_t address);
static void ppu_write(ppu_t* ppu, uint16_t address, uint8_t value);
static uint16_t ppu_tile_get_map_address(const ppu_t* ppu);
static uint16_t ppu_tile_get_data_address(const ppu_t* ppu, uint8_t tile, uint8_t row);
static void ppu_fetch_tile(ppu_t* ppu);
static void ppu_fetch_tile_data_low(ppu_t* ppu);
static void ppu_fetch_tile_data_high(ppu_t* ppu);
static bool ppu_fetch_push(ppu_t* ppu);
static void ppu_fetcher_step(ppu_t* ppu);

void ppu_init(ppu_t* ppu, bus_t* bus) {
    ppu->bus = bus;

    ppu_fifo_clear(&ppu->bg_fifo);
    ppu_fifo_clear(&ppu->obj_fifo);
}

void ppu_step(ppu_t* ppu) {
    ppu->line_dots++;

    /* Entering PPU step. There are roughly 4 line_dots per M-cycle and 2 per Double-Speed M-cycle. */
    switch (ppu->mode) {
        /* OAM Scan Mode */
        case PPU_MODE2:
            /* TODO: OAM stuff */
            if (ppu->line_dots == PPU_MODE2_DOTS) {  
                ppu->xpos = 0;
            }
            break;

        /* Drawing Mode */
        case PPU_MODE3:
            if (ppu->xpos == 160) {
                ppu->mode = PPU_MODE0;
            }

            if (ppu_fifo_empty(&ppu->bg_fifo)) {
                ppu_fetcher_step(ppu);
            }
            
            ppu_pixel_t pixel;
            if (!ppu_fifo_pop(&ppu->bg_fifo, &pixel)) {
                break;
            }

            if (ppu->discard_pixels != 0) {
                ppu->discard_pixels--;
            } else {
                ppu->framebuffer[ppu->xpos * ppu->bus->io.ly] = pixel;
                ppu->xpos++;
            }

            break;
        /* H-Blank Mode */
        case PPU_MODE0: {
            if (ppu->line_dots != PPU_LINE_DOTS) {
                break;
            }

            ppu->line_dots = 0;
            ppu->bus->io.ly++;
           
            if (ppu->bus->io.ly == PPU_LY_VBLANK) {
                ppu->mode = PPU_MODE1;
            } else {
                ppu->mode = PPU_MODE2;
            }
        
            break;
        }

        /* V-Blank Mode */
        case PPU_MODE1:
            /* One scanline of V-Blank is equivalent to the duration of modes 2, 3, and 0 combined. */
            if (ppu->line_dots == PPU_LINE_DOTS) {
                ppu->line_dots = 0;
                ppu->bus->io.ly++;
            }

            if (ppu->bus->io.ly == PPU_LY_BOTTOM) {
                ppu->mode = PPU_MODE2;
            }
            break;
    }
}

static uint8_t ppu_read(ppu_t* ppu, uint16_t address) {
    return bus_read8(ppu->bus, BUS_MASTER_PPU, address);
}

static void ppu_write(ppu_t* ppu, uint16_t address, uint8_t value) {
    bus_write8(ppu->bus, BUS_MASTER_PPU, address, value);
}

static uint16_t ppu_tile_get_map_address(const ppu_t* ppu) {
    if (BIT(ppu->bus->io.lcdc, LCDC_BG_TILE_MAP_AREA) && !BETWEEN(ppu->xpos, 0, ppu->bus->io.wx)) {
        return ADDR_VRAM_TILEMAP1_START;
    }

    if (BIT(ppu->bus->io.lcdc, LCDC_WIN_TILE_MAP_AREA) && BETWEEN(ppu->xpos, 0, ppu->bus->io.wx)) {
        return ADDR_VRAM_TILEMAP1_START;
    }

    return ADDR_VRAM_TILEMAP0_START;
}

static uint16_t ppu_tile_get_data_address(const ppu_t* ppu, uint8_t tile, uint8_t row) {
    if (BIT(ppu->bus->io.lcdc, LCDC_BGW_TILE_DATA_AREA)) {
        return (uint16_t)(VRAM_TILEDATA_UNSIGNED_ADDR + (uint16_t)tile * DATA_TILE_SIZE + (uint16_t)row * DATA_ROW_SIZE);
    }
    return (uint16_t)(VRAM_TILEDATA_SIGNED_ADDR + (int16_t)(int8_t)tile * DATA_TILE_SIZE + (int16_t)row * DATA_ROW_SIZE);
}

static void ppu_fetch_tile(ppu_t* ppu) {
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t tile_row;
    uint8_t map_base;
    // TODO: refine
    bool window_enabled = ppu->y_cond && BIT(ppu->bus->io.lcdc, LCDC_WIN_EN);
    uint8_t window_left = ppu->bus->io.wx - 7; // subtract 7 for some reason
    
    /* window rendering begins; reset fifo and fetcher */
    if (window_enabled && ppu->xpos >= window_left) {
        ppu->fetch_tile = 0;
        ppu->fetch_tile_attr = 0;
        ppu->fetch_low = 0;
        ppu->fetch_high = 0;
        ppu->fetch_row = 0;
        ppu->fetch_tile_x = 0;
        ppu_fifo_clear(&ppu->bg_fifo); 
    }
    
    map_base = ppu_tile_get_map_address(ppu);

    if (ppu->window_active) {
        tile_x = ppu->fetch_tile_x & 0x1F;
        tile_y = (ppu->window_line >> 3) & 0x1F;
        tile_row = ppu->window_line & DATA_ROW_MASK;
    } else {    
        tile_x = ((ppu->bus->io.scx + ppu->xpos) >> 3) & 0x1F;
        tile_y = (ppu->bus->io.scy + ppu->bus->io.ly) & 0xFF;
        tile_row = tile_y & DATA_ROW_MASK;   
    }

    uint16_t map_address = map_base + (uint16_t)tile_y * MAP_ROWS + tile_x;
    ppu->fetch_tile = ppu_read(ppu, map_address);
    ppu->fetch_tile_attr = ppu_read(ppu, map_address + ADDR_VRAM_SIZE);
    ppu->fetch_row = tile_row;
}
static void ppu_fetch_tile_data_low(ppu_t* ppu) {
    if(ppu->fetch_tile_attr & 0x40) {
        ppu->fetch_row = DATA_ROW_MASK - ppu->fetch_row;
    }

    uint16_t data_address = ppu_tile_get_data_address(ppu, ppu->fetch_tile, ppu->fetch_row);
    
    ppu->fetch_low = ppu_read(ppu, data_address);
}

static void ppu_fetch_tile_data_high(ppu_t* ppu) {
    uint16_t data_address = ppu_tile_get_data_address(ppu, ppu->fetch_tile, ppu->fetch_row);

    ppu->fetch_high = ppu_read(ppu, data_address + 1);
}

static bool ppu_fetch_push(ppu_t* ppu) {
    if (ppu_fifo_count(&ppu->bg_fifo) > PPU_FIFO_CAPACITY - 8) {
        return false;
    }

    bool flip_x = (ppu->fetch_tile_attr & 0x20) != 0;
    for (unsigned x = 0; x < 8; x++) {
        unsigned bit = flip_x ? x : 7 - x;

        ppu_pixel_t pixel = {
            .color = (uint8_t)((((ppu->fetch_high >> bit) & 1) << 1) | ((ppu->fetch_low >> bit) & 1)),
            .palette = 0,
            .priority = ppu->fetch_tile_attr & 0x80,
        };

        if (!ppu_fifo_push(&ppu->bg_fifo, pixel)) {
            return false;
        }
    }

    ppu->fetch_tile_x++;
    return true;
}

static void ppu_fetcher_step(ppu_t* ppu) {
    if (ppu->fetch_state == FETCH_PUSH) {
        ppu->fetch_state = ppu_fetch_push(ppu) ? FETCH_TILE : FETCH_PUSH;
        return;
    }

    ppu->fetch_dots++;
    if (ppu->fetch_dots < 2u) {
        return;
    }
    
    switch (ppu->fetch_state) {
        case FETCH_TILE:
            ppu_fetch_tile(ppu);
            ppu->fetch_state = FETCH_LOW;
            break;
        case FETCH_LOW:
            ppu_fetch_tile_data_low(ppu);
            ppu->fetch_state = FETCH_HIGH;
            break;
        case FETCH_HIGH:
            ppu_fetch_tile_data_high(ppu);
            ppu->fetch_state = FETCH_SLEEP;
            break;
        case FETCH_SLEEP:
            ppu->fetch_state = FETCH_PUSH;
            break;
        case FETCH_PUSH:
            break;
    }
    
    ppu->fetch_dots = 0;
}

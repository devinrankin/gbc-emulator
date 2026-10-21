#include "ppu.h"
#include "ppu_fifo.h"

#define PPU_MODE_DOTS_OAM 80
#define PPU_DOTS_PER_LINE 456 

#define PPU_LY_VBLANK 144
#define PPU_LY_BOTTOM 154

/* LCDC bits */
#define PPU_LCDC_BGW_ENPRI 0u
#define PPU_LCDC_OBJ_EN 1u
#define PPU_LCDC_OBJ_SIZE 2u
#define PPU_LCDC_BG_TILE_MAP_AREA 3u
#define PPU_LCDC_BGW_TILE_DATA_AREA 4u
#define PPU_LCDC_WIN_EN 5u
#define PPU_LCDC_WIN_TILE_MAP_AREA 6u
#define PPU_LCDC_LCD_PPU_EN 7u

/* STAT bits */
#define PPU_STAT_PPU_MODE_MASK 3u // this is 2 bits, probably won't use this
#define PPU_STAT_LYC 2u
#define PPU_STAT_MODE0_INTERRUPT 3u
#define PPU_STAT_MODE1_INTERRUPT 4u
#define PPU_STAT_MODE2_INTERRUPT 5u
#define PPU_STAT_LYC_INTERRUPT 6u

#define PPU_NUM_PX_PER_FETCH 8u
#define PPU_TILES_PER_MAP_ROW 32u
#define PPU_BYTES_PER_TILE 16u
#define PPU_BYTES_PER_OBJ 4u
#define PPU_BYTES_PER_COLOR 2u

#define PPU_TILE_DATA_ROW_MASK 0x7

/* BG Map Attribute bits */
#define PPU_BG_MAP_ATTR_PALETTE_MASK 7u
#define PPU_BG_MAP_ATTR_BANK 3u
#define PPU_BG_MAP_ATTR_X_FLIP 5u
#define PPU_BG_MAP_ATTR_Y_FLIP 6u
#define PPU_BG_MAP_ATTR_PRIO 7u

/* OAM Attribute bits */
#define PPU_OBJ_ATTR_PALETTE_MASK 7u
#define PPU_OBJ_ATTR_BANK 3u
#define PPU_OBJ_ATTR_DMG_PALETTE 4u
#define PPU_OBJ_ATTR_X_FLIP 5u
#define PPU_OBJ_ATTR_Y_FLIP 6u
#define PPU_OBJ_ATTR_PRIO 7u

#define PPU_OBJ_PER_SCREEN 40u
#define PPU_OBJ_HEIGHT0 8u
#define PPU_OBJ_HEIGHT1 16u
#define PPU_OBJ_OFFSET_Y 16u
#define PPU_OBJ_OFFSET_X 8u

static const uint32_t dmg_colors[4] = {
    0xFFFFFFFF,
    0xFFAAAAAA,
    0xFF555555,
    0xFF000000,
};

/* Local function declarations */
static void ppu_oam_scan_step(ppu_t* ppu);

static uint16_t ppu_tile_map_base(const ppu_t* ppu, bool window);
static uint16_t ppu_tile_data_address(const ppu_t* ppu, uint8_t tile, uint8_t row);
static uint8_t ppu_tile_data_bank(const ppu_t* ppu);

static void ppu_fetch_tile(ppu_t* ppu);
static void ppu_fetch_tile_data_low(ppu_t* ppu);
static void ppu_fetch_tile_data_high(ppu_t* ppu);
static bool ppu_fetch_push(ppu_t* ppu);
static void ppu_fetcher_step(ppu_t* ppu);

static void ppu_mode_transition_oam(ppu_t* ppu);
static void ppu_mode_transition_draw(ppu_t* ppu);
static bool ppu_is_cgb(const ppu_t* ppu);
static uint8_t ppu_dmg_shade(const ppu_t* ppu, uint8_t color);

static uint8_t ppu_read_vram(ppu_t* ppu, uint8_t bank, uint16_t address); 
static uint8_t ppu_read_oam(ppu_t* ppu, uint16_t address);

static uint16_t (*ppu_get_pixel_color)(ppu_pixel_t*, ppu_pixel_t*);

void dump_framebuffer(ppu_t *ppu) {
    (void)ppu;
/* TODO: part of debug mode */
}

void ppu_init(ppu_t* ppu, bus_t* bus) {
    ppu->bus = bus;
    ppu->mode = PPU_MODE_OAM;
    ppu->line_dots = 0;

    ppu_fifo_clear(&ppu->bgw_fifo);
    ppu_fifo_clear(&ppu->obj_fifo);
}

void ppu_step(ppu_t* ppu) {
    /* Request STAT interrupt if LYC = LY */
    if (ppu->bus->io.lyc == ppu->bus->io.ly) {
        BIT_SET(ppu->bus->io.stat, 2);
        io_request_interrupt(&ppu->bus->io, INTERRUPT_BIT_STAT);
    }

    ppu->line_dots++;
    switch (ppu->mode) {
    case PPU_MODE_OAM:
        ppu_oam_scan_step(ppu);
        if (ppu->line_dots == PPU_MODE_DOTS_OAM) { 
            ppu->mode = PPU_MODE_DRAW;
            ppu_mode_transition_draw(ppu);
        }
        break;
    case PPU_MODE_DRAW: {
        const bool window_enabled = ppu->y_cond && BIT(ppu->bus->io.lcdc, PPU_LCDC_WIN_EN);
        int16_t window_left = (int16_t)ppu->bus->io.wx - 7;

        if (!ppu->window_active && window_enabled && window_left < PPU_XRES && (int16_t)ppu->xpos >= (window_left < 0 ? 0 : window_left)) {
            ppu->window_active = true;
            ppu->fetch_tile_x = 0;
            ppu->fetch_dots = 0;
            ppu->fetch_state = FETCH_TILE;
            ppu_fifo_clear(&ppu->bgw_fifo);
        }

        ppu_fetcher_step(ppu);

        ppu_pixel_t pixel;
        if (!ppu_fifo_pop(&ppu->bgw_fifo, &pixel)) {
            break;
        }

        if (ppu->discard_pixels != 0) {
            ppu->discard_pixels--;
            break;
        }

        if (ppu->xpos < PPU_XRES) {
            if (!ppu_is_cgb(ppu)) {
                pixel.color = ppu_dmg_shade(ppu, pixel.color);
            }

            ppu->framebuffer[ppu->bus->io.ly * PPU_XRES + ppu->xpos] = pixel;
            ppu->xpos++;
        }
        
        if (ppu->xpos >= PPU_XRES) {
            ppu->mode = PPU_MODE_HBLANK;
        }
        break;
    }
    case PPU_MODE_HBLANK: 
        if (ppu->line_dots != PPU_DOTS_PER_LINE) {
            break;
        }

        ppu->line_dots = 0;
        ppu->bus->io.ly++;
       
        if (ppu->bus->io.ly == PPU_LY_VBLANK) {
            ppu->mode = PPU_MODE_VBLANK;
            io_request_interrupt(&ppu->bus->io, INTERRUPT_BIT_VBLANK);
        } else {
            ppu->mode = PPU_MODE_OAM;
            ppu_mode_transition_oam(ppu);
        }
        break;
    /* V-Blank Mode */
    case PPU_MODE_VBLANK:
        /* One scanline of V-Blank is equivalent to the duration of modes 2, 3, and 0 combined. */
        if (ppu->line_dots == PPU_DOTS_PER_LINE) {
            ppu->line_dots = 0;
            ppu->bus->io.ly++;
            if (ppu->bus->io.ly >= PPU_LY_BOTTOM) {
                ppu->bus->io.ly = 0;
                ppu->mode = PPU_MODE_DRAW;
                ppu_mode_transition_oam(ppu);
            }
        }
        break;
    }
}

static void ppu_oam_scan_step(ppu_t* ppu) {
    if ((ppu->line_dots & 1u) == 0) {
        return;
    }

    if (ppu->oam_scan_index >= PPU_OBJ_PER_SCREEN) {
        return;
    }

    uint8_t oam_index = ppu->oam_scan_index++;
    uint16_t address = (uint16_t)(ADDR_OAM_BASE + PPU_BYTES_PER_OBJ * oam_index);

    uint8_t y = ppu_read_oam(ppu, address);
    uint8_t x = ppu_read_oam(ppu, address + 1);
    uint8_t tile = ppu_read_oam(ppu, address + 2);
    uint8_t attr = ppu_read_oam(ppu, address + 3);

    uint8_t height = BIT(ppu->bus->io.lcdc, PPU_LCDC_OBJ_SIZE) ? PPU_OBJ_HEIGHT1 : PPU_OBJ_HEIGHT0;
    
    if (ppu->bus->io.ly + PPU_OBJ_OFFSET_Y < y || ppu->bus->io.ly + PPU_OBJ_OFFSET_Y >= y + height) {
        return;
    }

    if (ppu->obj_count < OBJ_BUF_CAPACITY) {
        ppu->obj_buffer[ppu->obj_count++] = (ppu_obj_t) {
            .y = y,
            .x = x,
            .tile = tile,
            .attr = attr,
            .oam_index = oam_index
        };
    }
}

static void ppu_fetch_tile(ppu_t* ppu) {
    const bool window = ppu->window_active;
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t tile_row;
   
    /* A tile is 8x8 pixels */
    if (window) {
        tile_x = ppu->fetch_tile_x & 0x1F;
        tile_y = (ppu->window_line >> 3) & 0x1F;
        tile_row = ppu->window_line & 7; 
    } else {   
        uint8_t bg_y = (ppu->bus->io.scy + ppu->bus->io.ly) & 0xFF;

        tile_x = ((ppu->bus->io.scx >> 3) + ppu->fetch_tile_x) & 0x1F;
        tile_y = (bg_y >> 3) & 0x1F;
        tile_row = bg_y & 7;   
    }

    uint16_t map_address = ppu_tile_map_base(ppu, window) + (uint16_t)tile_y * PPU_TILES_PER_MAP_ROW + tile_x;
    ppu->fetch_tile = ppu_read_vram(ppu, 0, map_address);
    ppu->fetch_tile_attr = ppu_is_cgb(ppu) ? ppu_read_vram(ppu, 1, map_address) : 0;
    ppu->fetch_row = tile_row;
}

static void ppu_fetch_tile_data_low(ppu_t* ppu) {
    if (BIT(ppu->fetch_tile_attr, PPU_BG_MAP_ATTR_Y_FLIP)) {
        ppu->fetch_row = 7 - ppu->fetch_row;
    }

    uint16_t data_address = ppu_tile_data_address(ppu, ppu->fetch_tile, ppu->fetch_row);
    
    ppu->fetch_low = ppu_read_vram(ppu, ppu_tile_data_bank(ppu), data_address);

}

static void ppu_fetch_tile_data_high(ppu_t* ppu) {
    uint16_t data_address = ppu_tile_data_address(ppu, ppu->fetch_tile, ppu->fetch_row);

    ppu->fetch_high = ppu_read_vram(ppu, ppu_tile_data_bank(ppu), data_address + 1);
}

/* Pushes a pixel into the pixel FIFO */
static bool ppu_fetch_push(ppu_t* ppu) {
    if (ppu_fifo_count(&ppu->bgw_fifo) > PPU_FIFO_FETCHER_CAPACITY - PPU_NUM_PX_PER_FETCH) {
        return false;
    }
    
    bool flip_x = BIT(ppu->fetch_tile_attr, PPU_BG_MAP_ATTR_X_FLIP);
    for (uint8_t x = 0; x < PPU_NUM_PX_PER_FETCH; x++) {
        uint8_t bit = flip_x ? x : 7 - x;

        ppu_pixel_t pixel = {
            .color = (uint8_t)((((ppu->fetch_high >> bit) & 1) << 1) | ((ppu->fetch_low >> bit) & 1)),
            .palette = ppu->fetch_tile_attr & PPU_BG_MAP_ATTR_PALETTE_MASK,
            .priority = BIT(ppu->fetch_tile_attr, PPU_BG_MAP_ATTR_PRIO),
        };

        if (!ppu_fifo_push(&ppu->bgw_fifo, pixel)) {
            return false;
        }
    }

    ppu->fetch_tile_x++;
    return true;
}

static void ppu_fetcher_step(ppu_t* ppu) {
    if (ppu->fetch_state == FETCH_PUSH) {
        if (ppu_fetch_push(ppu)) {
            ppu->fetch_state = FETCH_TILE;
        }
        return;
    }

    if (++ppu->fetch_dots < 2u) {
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
        /* Handled above */
        break;
    }
    
    ppu->fetch_dots = 0;
}

/* Check if Cartridge is CGB compatible */
static bool ppu_is_cgb(const ppu_t* ppu) {
    return ppu->bus->cartridge->mode != CARTRIDGE_DMG_ONLY;
}

/* Probably temporary; gets DMG color from BGP register */
static uint8_t ppu_dmg_shade(const ppu_t* ppu, uint8_t color) {
    return (ppu->bus->io.bgp >> (color * 2)) & 0x03;
}

/* Returns the base address of the tile map used for tile data fetching */
static uint16_t ppu_tile_map_base(const ppu_t* ppu, bool window) {
    unsigned lcdc_bit = window ? PPU_LCDC_WIN_TILE_MAP_AREA : PPU_LCDC_BG_TILE_MAP_AREA;
    
    return BIT(ppu->bus->io.lcdc, lcdc_bit) ? ADDR_VRAM_TILEMAP1_BASE : ADDR_VRAM_TILEMAP0_BASE;
}

/* Returns the base address used for tile data fetching */
static uint16_t ppu_tile_data_address(const ppu_t* ppu, uint8_t tile, uint8_t row) {
    if (BIT(ppu->bus->io.lcdc, PPU_LCDC_BGW_TILE_DATA_AREA)) {
        return (uint16_t)(ADDR_VRAM_TILEDATA_BLOCK0_BASE + (uint16_t)tile * PPU_BYTES_PER_TILE + (uint16_t)row * 2u);
    }
    return (uint16_t)(ADDR_VRAM_TILEDATA_BLOCK2_BASE + (int16_t)(int8_t)tile * PPU_BYTES_PER_TILE + (int16_t)row * 2u);
}

/* Returns the bank used for tile data fetching */
static uint8_t ppu_tile_data_bank(const ppu_t* ppu) {
    return ppu_is_cgb(ppu) ? (uint8_t)((ppu->fetch_tile_attr >> 3) & 1) : 0;
}

/* Sets PPU state for OAM scan mode */
static void ppu_mode_transition_oam(ppu_t* ppu) {
    ppu->xpos = 0;
    ppu->window_active = false;
    ppu->fetch_tile_x = 0;
    ppu->fetch_dots = 0;
    ppu->fetch_state = FETCH_TILE;
    ppu_fifo_clear(&ppu->bgw_fifo);

    ppu->obj_count = 0;
    ppu->oam_scan_index = 0;
}

/* Sets PPU state for draw mode */
static void ppu_mode_transition_draw(ppu_t* ppu) {
    ppu->xpos = 0;
    ppu->discard_pixels = ppu->bus->io.scx & 7;
    ppu->window_active = false;

    ppu->fetch_tile_x = 0;
    ppu->fetch_dots = 0;
    ppu->fetch_state = FETCH_TILE;

    ppu_fifo_clear(&ppu->bgw_fifo);
}

/* Reads directly from VRAM. Maybe temporary; a bit hacky since PPU needs access to both banks of VRAM whenever it wants */
static uint8_t ppu_read_vram(ppu_t* ppu, uint8_t bank, uint16_t address) {
    if (!BETWEEN(address, ADDR_VRAM_BASE, ADDR_VRAM_BASE + ADDR_VRAM_SIZE)) {
        return 0xFF;
    }

    size_t offset = (size_t)(address - ADDR_VRAM_BASE);
    return ppu->bus->vram[(BIT(bank, 0u)) * ADDR_VRAM_SIZE + offset];
}

/* Same as VRAM reads, except OAM doesn't have banks; just accessing the data the same way for consistency */
static uint8_t ppu_read_oam(ppu_t* ppu, uint16_t address) {
    if (!BETWEEN(address, ADDR_OAM_BASE, ADDR_OAM_BASE + ADDR_OAM_SIZE)) {
        return 0xFF;
    }

    size_t offset = (size_t)(address - ADDR_OAM_BASE);
    return ppu->bus->oam[offset];
}

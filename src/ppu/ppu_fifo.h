#ifndef PPU_FIFO_H
#define PPU_FIFO_H

#include <stdint.h>
#include <stdbool.h>

#define PPU_FIFO_FETCHER_CAPACITY 16u

typedef struct {
    uint8_t color;
    uint8_t palette;
    uint8_t priority;
} ppu_pixel_t;

typedef struct {
    ppu_pixel_t pixels[PPU_FIFO_FETCHER_CAPACITY];
    uint8_t head;
    uint8_t count;
} ppu_fifo_t;

void ppu_fifo_clear(ppu_fifo_t* fifo);
bool ppu_fifo_push(ppu_fifo_t* fifo, ppu_pixel_t pixel);
bool ppu_fifo_pop(ppu_fifo_t* fifo, ppu_pixel_t* pixel);
bool ppu_fifo_empty(const ppu_fifo_t*);
uint8_t ppu_fifo_count(const ppu_fifo_t* fifo);

#endif

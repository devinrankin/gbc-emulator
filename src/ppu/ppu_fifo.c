#include "ppu_fifo.h"

void ppu_fifo_clear(ppu_fifo_t* fifo) {
    fifo->head = 0;
    fifo->count = 0;
}

bool ppu_fifo_push(ppu_fifo_t* fifo, ppu_pixel_t pixel) {
    if (fifo->count == PPU_FIFO_FETCHER_CAPACITY) {
        return false;
    }

    unsigned tail = (fifo->head + fifo->count) % PPU_FIFO_FETCHER_CAPACITY;
    fifo->pixels[tail] = pixel;
    fifo->count++;
    
    return true;
}

bool ppu_fifo_pop(ppu_fifo_t* fifo, ppu_pixel_t* pixel) {
    if (fifo->count == 0) {
        return false;
    }

    *pixel = fifo->pixels[fifo->head];
    fifo->head = (fifo->head + 1) % PPU_FIFO_FETCHER_CAPACITY;
    fifo->count--;

    return true;
}

bool ppu_fifo_empty(const ppu_fifo_t *fifo) {
    return fifo->count == 0;
}

uint8_t ppu_fifo_count(const ppu_fifo_t *fifo) {
    return fifo->count;
}

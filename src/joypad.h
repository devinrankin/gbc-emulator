#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdint.h>
#include <stdbool.h>

#define JOYPAD_SELECT_DPAD_MASK (1u << 4)
#define JOYPAD_SELECT_BUTTONS_MASK (1u << 5)

/* P14 D-pad Selected */
#define JOYPAD_RIGHT_MASK (1u << 0)
#define JOYPAD_LEFT_MASK (1u << 1)
#define JOYPAD_UP_MASK (1u << 2)
#define JOYPAD_DOWN_MASK (1u << 3)

/* P15 Buttons Selected */
#define JOYPAD_A_MASK (1u << 0)
#define JOYPAD_B_MASK (1u << 1)
#define JOYPAD_SELECT_MASK (1u << 2)
#define JOYPAD_START_MASK (1u << 3)

#define JOYPAD_BUTTON_COUNT 8

typedef enum {
    JOYPAD_RIGHT,
    JOYPAD_LEFT,
    JOYPAD_UP,
    JOYPAD_DOWN,
    JOYPAD_A,
    JOYPAD_B,
    JOYPAD_SELECT,
    JOYPAD_START
} joypad_button_t;

typedef struct joypad_state {
    uint8_t select;
    bool pressed[JOYPAD_BUTTON_COUNT]; 
} joypad_t;

uint8_t joypad_read(const joypad_t* joypad);
void joypad_write(joypad_t* joypad, uint8_t value);
void joypad_set_pressed(joypad_t* joypad, joypad_button_t button, bool pressed);

#endif

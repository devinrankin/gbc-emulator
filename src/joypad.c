#include "joypad.h"

uint8_t joypad_read(const joypad_t* joypad) {
    uint8_t low = 0x0F;

    bool dpad_selected = !(joypad->select & JOYPAD_SELECT_DPAD_MASK);
    bool buttons_selected = !(joypad->select & JOYPAD_SELECT_BUTTONS_MASK);

    if (dpad_selected) {
        if (joypad->pressed[JOYPAD_RIGHT]) {
            low &= ~JOYPAD_RIGHT_MASK;
        }
        if (joypad->pressed[JOYPAD_LEFT]) {
            low &= ~JOYPAD_LEFT_MASK;
        }
        if (joypad->pressed[JOYPAD_UP]) {
            low &= ~JOYPAD_UP_MASK;
        }
        if (joypad->pressed[JOYPAD_DOWN]) {
            low &= ~JOYPAD_DOWN_MASK;
        }
    }

    if (buttons_selected) {
        if (joypad->pressed[JOYPAD_A]) {
            low &= ~JOYPAD_A_MASK;
        }
        if (joypad->pressed[JOYPAD_B]) {
            low &= ~JOYPAD_B_MASK;
        }
        if (joypad->pressed[JOYPAD_SELECT]) {
            low &= ~JOYPAD_SELECT_MASK;
        }
        if (joypad->pressed[JOYPAD_START]) {
            low &= ~JOYPAD_START_MASK;
        }
    }

    return 0xC0 | (joypad->select & (JOYPAD_SELECT_BUTTONS_MASK | JOYPAD_SELECT_DPAD_MASK)) | low;
}

void joypad_write(joypad_t* joypad, uint8_t value) {
    joypad->select = value & (JOYPAD_SELECT_BUTTONS_MASK | JOYPAD_SELECT_DPAD_MASK);
}

void joypad_set_pressed(joypad_t* joypad, joypad_button_t button, bool pressed) {
    joypad->pressed[button] = pressed;
}

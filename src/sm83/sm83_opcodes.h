#ifndef SM83_OPCODES_H
#define SM83_OPCODES_H

#include <stdint.h>

typedef struct sm83 sm83_t;

typedef unsigned (*sm83_opcode_handler_t)(sm83_t* sm83, uint8_t opcode);

typedef struct {
    sm83_opcode_handler_t handler;
    uint8_t length;
    uint8_t cycles;
    const char* name; 
} sm83_instruction_t;

extern const sm83_instruction_t sm83_opcode_table[256];
extern const sm83_instruction_t sm83_cb_opcode_table[256];

#endif


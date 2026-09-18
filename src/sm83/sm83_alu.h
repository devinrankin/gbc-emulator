#ifndef SM83_ALU_H
#define SM83_ALU_H

#include <stdint.h>

typedef struct sm83 sm83_t;

typedef enum {
    SM83_ALU_ADD,
    SM83_ALU_ADC,
    SM83_ALU_SUB,
    SM83_ALU_SBC,
    SM83_ALU_CP,
    SM83_ALU_AND,
    SM83_ALU_OR,
    SM83_ALU_XOR
} sm83_alu_op_t;

uint8_t alu_execute(sm83_t* sm83, sm83_alu_op_t operation, uint8_t lhs, uint8_t rhs);

uint16_t alu_add16(sm83_t* sm83, uint16_t lhs, uint16_t rhs);
uint16_t alu_add_sp_i8(sm83_t* sm83, uint16_t sp, int8_t offset);

#endif

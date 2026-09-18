#include "sm83_alu.h"
#include "sm83.h"

static uint8_t alu_add(sm83_t* sm83, uint8_t lhs, uint8_t rhs, uint8_t carry_in);
static uint8_t alu_sub(sm83_t* sm83, uint8_t lhs, uint8_t rhs, uint8_t carry_in);
static uint8_t alu_and(sm83_t* sm83, uint8_t lhs, uint8_t rhs);
static uint8_t alu_or(sm83_t* sm83, uint8_t lhs, uint8_t rhs);
static uint8_t alu_xor(sm83_t* sm83, uint8_t lhs, uint8_t rhs);

uint8_t alu_execute(sm83_t *sm83, sm83_alu_op_t operation, uint8_t lhs, uint8_t rhs) {
    switch (operation) {
        case SM83_ALU_ADD:
            return alu_add(sm83, lhs, rhs, 0);
        case SM83_ALU_ADC:
            return alu_add(sm83, lhs, rhs, 1);
        case SM83_ALU_SUB:
            return alu_sub(sm83, lhs, rhs, 0);
        case SM83_ALU_SBC:
            return alu_sub(sm83, lhs, rhs, 1);
        case SM83_ALU_CP:
            return alu_sub(sm83, lhs, rhs, 0);
        case SM83_ALU_AND:
            return alu_and(sm83, lhs, rhs);
        case SM83_ALU_OR:
            return alu_or(sm83, lhs, rhs);
        case SM83_ALU_XOR:
            return alu_xor(sm83, lhs, rhs);
    }
}

static uint8_t alu_add(sm83_t* sm83, uint8_t lhs, uint8_t rhs, uint8_t carry_in) {
    uint16_t result = lhs + rhs + carry_in;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    if((lhs & 0x0F) + (rhs & 0x0F) + carry_in > 0x0F) flags |= SM83_FLAG_H;
    if(result + carry_in > 0xFF) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
    return (uint8_t)result;
}

static uint8_t alu_sub(sm83_t* sm83, uint8_t lhs, uint8_t rhs, uint8_t carry_in) {
    uint16_t result = lhs - rhs - carry_in;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_N;
    if((lhs & 0x0F) < ((rhs & 0x0F) + carry_in)) flags |= SM83_FLAG_H;
    if(lhs < (uint16_t)rhs + carry_in) flags |= SM83_FLAG_C;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_N | SM83_FLAG_H | SM83_FLAG_C, flags);
    return (uint8_t)result;
}

static uint8_t alu_and(sm83_t* sm83, uint8_t lhs, uint8_t rhs) {
    uint8_t result = lhs & rhs;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;
    flags |= SM83_FLAG_H;

    sm83_update_flags(sm83, SM83_FLAG_Z | SM83_FLAG_H, flags);
    return result;
}

static uint8_t alu_or(sm83_t* sm83, uint8_t lhs, uint8_t rhs) {
    uint8_t result = lhs | rhs;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;

    sm83_update_flags(sm83, SM83_FLAG_Z, flags);
    return result;
}

static uint8_t alu_xor(sm83_t* sm83, uint8_t lhs, uint8_t rhs) {
    uint8_t result = lhs ^ rhs;

    uint8_t flags = 0;

    if(result == 0) flags |= SM83_FLAG_Z;

    sm83_update_flags(sm83, SM83_FLAG_Z, flags);
    return result;
}


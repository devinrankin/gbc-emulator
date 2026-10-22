#ifndef SM83_OPCODES_H
#define SM83_OPCODES_H

#include <stdio.h>
#include <stdint.h>

#define OPCODE_X(op) (((op) >> 6) & 0x03u)
#define OPCODE_Y(op) (((op) >> 3) & 0x07u)
#define OPCODE_Z(op) ((op) & 0x07u)
#define OPCODE_P(op) (((op) >> 4) & 0x03u)
#define OPCODE_Q(op) (((op) >> 3) & 0x01u)

#define OPCODE_COND(op) (OPCODE_Y(op) & 0x03);

typedef struct sm83 sm83_t;

typedef enum {
    SM83_OP_NOP,
    SM83_OP_LD,
    SM83_OP_LDH,

    SM83_OP_INC,
    SM83_OP_DEC,

    SM83_OP_ADD,
    SM83_OP_ADC,
    SM83_OP_SUB,
    SM83_OP_SBC,
    SM83_OP_AND,
    SM83_OP_XOR,
    SM83_OP_OR,
    SM83_OP_CP,

    SM83_OP_DAA,
    SM83_OP_CPL,
    SM83_OP_SCF,
    SM83_OP_CCF,

    SM83_OP_RLCA,
    SM83_OP_RRCA,
    SM83_OP_RLA,
    SM83_OP_RRA,

    SM83_OP_RLC,
    SM83_OP_RRC,
    SM83_OP_RL,
    SM83_OP_RR,
    SM83_OP_SLA,
    SM83_OP_SRA,
    SM83_OP_SWAP,
    SM83_OP_SRL,

    SM83_OP_BIT,
    SM83_OP_RES,
    SM83_OP_SET,

    SM83_OP_JP,
    SM83_OP_JR,
    SM83_OP_CALL,
    SM83_OP_RET,
    SM83_OP_RETI,
    SM83_OP_RST,

    SM83_OP_PUSH,
    SM83_OP_POP,

    SM83_OP_HALT,
    SM83_OP_STOP,
    SM83_OP_DI,
    SM83_OP_EI,
} sm83_operation_t;

typedef enum {
    SM83_OPERAND_NONE,
    SM83_OPERAND_R8,
    SM83_OPERAND_R16,
    SM83_OPERAND_MEM_R8,
    SM83_OPERAND_MEM_R16,
    SM83_OPERAND_HLI,
    SM83_OPERAND_HLD,
    SM83_OPERAND_IMM8,
    SM83_OPERAND_IMM16,
    SM83_OPERAND_MEM_HIGH_IMM8,
    SM83_OPERAND_MEM_IMM16,
    SM83_OPERAND_MEM_HIGH_C,
    SM83_OPERAND_SIGNED_IMM8,
    SM83_OPERAND_COND,
    SM83_OPERAND_BIT
} sm83_operand_kind_t;

typedef enum {
    SM83_REG_NONE,
    SM83_REG_A,
    SM83_REG_B,
    SM83_REG_C,
    SM83_REG_D,
    SM83_REG_E,
    SM83_REG_H,
    SM83_REG_L,
    SM83_REG_AF,
    SM83_REG_BC,
    SM83_REG_DE,
    SM83_REG_HL,
    SM83_REG_SP,
    SM83_REG_PC
} sm83_register_t;

typedef enum {
    SM83_COND_NZ,
    SM83_COND_Z,
    SM83_COND_NC,
    SM83_COND_C
} sm83_cond_t;

typedef struct {
    sm83_operand_kind_t kind;
    uint8_t value;
} sm83_operand_t;

typedef unsigned (*sm83_opcode_handler_t)(sm83_t* sm83, uint8_t opcode);

typedef struct {
    sm83_opcode_handler_t handler;
    uint8_t length;
    sm83_operation_t operation;
    sm83_operand_t dst;
    sm83_operand_t src;
} sm83_instruction_t;

extern const sm83_instruction_t sm83_opcode_table[256];
extern const sm83_instruction_t sm83_opcode_table_cb[256];

void sm83_disassemble(sm83_t* sm83, uint8_t opcode, const sm83_instruction_t* table, uint16_t pc, char* out, size_t out_size);
#endif

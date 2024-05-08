#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "../semaphore/semaphore.h"

typedef enum Opcode {
    EXEC,
    READ,
    WRITE,
    PRINT,
    SEM_P,
    SEM_V
} opcode_t;


typedef struct Instruction {
    opcode_t op;
    int value;
    char* sem;
} instruction;

void instr_parse(instruction* instr, const char* line, semaphore_table_t* sem_table);

#endif // SO_PROJECT_INSTRUCTION_H

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "semaforo.h"

char EXEC = '1';
char READ = '2';
char WRITE = '3';
char PRINT = '4';
char SEM_P = '5';
char SEM_V = '6';


typedef struct Instruction {
    char op;
    int value;
    char* sem;
} instruction;

void instr_parse(instruction* instr, const char* line, semaphore_table_t* sem_table);

#endif // SO_PROJECT_INSTRUCTION_H

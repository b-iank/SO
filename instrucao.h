#ifndef INSTRUCAO_H
#define INSTRUCAO_H

#include "semaforo.h"

const char EXEC = '1';
const char READ = '2';
const char WRITE = '3';
const char PRINT = '4';
const char SEM_P = '5';
const char SEM_V = '6';


typedef struct instrucao {
    char op;
    int value;
    char *sem;
} INSTRUCAO;

void instr_parse(INSTRUCAO *instr, const char *line, TABELA_SEMAFORO *sem_table);

#endif // INSTRUCAO_H

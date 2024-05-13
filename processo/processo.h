#ifndef PROCESSO_H
#define PROCESSO_H

#include <stdio.h>
#include <time.h>
#include "../semaforo/semaforo.h"

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

// ESTADOS DO PROCESSO
#define NOVO '0'
#define BLOQUEADO '1'
#define PRONTO '2'
#define EXECUTANDO '3'
#define CONCLUIDO '4'

// INSTRUÇÃO
#define EXEC '1'
#define READ '2'
#define WRITE '3'
#define PRINT '4'
#define SEM_P '5'
#define SEM_V '6'

typedef struct processo PROCESSO;
typedef struct pcb PCB;
typedef struct instrucao INSTRUCAO;

struct processo {
    // Cabecalho
    char nome[MAX_PROCESS_NAME];
    int idSegmento;
    int prioridade;
    int tamanhoSegmento;
    char semaforos[MAX_SEMAFOROS];
    int quantidadeSemaforos;

    // Legado
    int id;
    clock_t tempoRestante;
    clock_t tempoChegada;

    char estado;
    int pc;
    int numComandos;

    struct processo *prox; // Lista de processos
};

struct pcb {
    PROCESSO *head;
    PROCESSO *tail;
    PROCESSO *atual;
};

struct instrucao {
    char op;
    int value;
    char sem;
};

// Processo
PCB iniciaPCB();
PCB *add_process(PCB *lista, PROCESSO *processo);
void readSyntheticProgram(FILE *, PROCESSO **, INSTRUCAO **);
void processCreate(char *fileName);
void processInterrupt(PCB *lista);
void processFinish(PCB *lista);

// Instrucao
void instr_parse(INSTRUCAO *instr, const char *line, TABELA_SEMAFORO *sem_table);

#endif //PROCESSO_H

#ifndef PROCESS_H
#define PROCESS_H

#include <time.h>
#include "instruction.h"

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

char NOVO = '0';
char BLOQUEADO = '1';
char PRONTO = '2';
char EXECUTANDO = '3';
char CONCLUIDO = '4';


typedef struct processo{
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
} PROCESSO;

typedef struct pcb {
    PROCESSO *head;
    PROCESSO *tail;
    PROCESSO *atual;
} PCB;

PCB iniciaPCB();
PCB *add_process(PCB *lista, PROCESSO *processo);

void readSyntheticProgram(FILE *, PROCESSO**, instruction**);
void processCreate(char *fileName);
void processInterrupt(PCB *lista);
void processFinish(PCB *lista);

#endif //PROCESS_H

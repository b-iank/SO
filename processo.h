#ifndef PROCESSO_H
#define PROCESSO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "instrucao.h"
#include "semaforo.h"
#include "kernel.h"
#include "memoria.h"

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

const char NOVO = '0';
const char BLOQUEADO = '1';
const char PRONTO = '2';
const char EXECUTANDO = '3';
const char CONCLUIDO = '4';


typedef struct processo {
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
void readSyntheticProgram(FILE *, PROCESSO **, INSTRUCAO **);
void processCreate(char *fileName);
void processInterrupt(PCB *lista);
void processFinish(PCB *lista);

#endif //PROCESSO_H

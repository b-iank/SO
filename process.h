#ifndef PROCESS_H
#define PROCESS_H

#include "instruction.h"

#define MAX_PROCESS_NAME 50
#define MAX_SEMAFOROS 10


typedef struct Process{
    // Cabecalho
    char name[MAX_PROCESS_NAME];
    int idSegmento;
    int priority;
    int tamanhoSegmento;
    int semaforos[MAX_SEMAFOROS];
    int numSemaforos;
    int numComandos;
    // Legado
    int id;
    int remainingTime;
    int arrivalTime;
    struct Process *next; // Lista de processos
} process_t;

typedef struct lista_circular_processos {
    process_t *head;
    process_t *tail;
    process_t *current;
} pcb;

pcb *add_process(pcb *lista, process_t *processo);
void readSyntacticProgram(FILE *, process_t**, instruction**);
process_t* processCreate(int id, const char *name);
void processInterrupt(pcb *lista);
void processFinish(pcb *lista);

#endif //PROCESS_H

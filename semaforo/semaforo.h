#ifndef SEMAFORO_H
#define SEMAFORO_H

#include <semaphore.h>

#include "terminal/terminal.h"
#include "processo/processo.h"

#define MAX_SEMAFOROS 10 //TODO: defiir quantos semaforos sao

typedef struct semaforo {
    char nome;
    int S;
    PROCESSO *aguardando;
    sem_t mutex;
} SEMAFORO;

typedef struct tabela_semaforo {
    SEMAFORO semaforo[MAX_SEMAFOROS];
    int quantidadeSemaforos;
} TABELA_SEMAFORO;

TABELA_SEMAFORO inciaTabelaSemaforo();

void novoSemaforo(char nome);

SEMAFORO *buscaSemaforo(char semaforo);

int existeSemaforoProcesso(char semaforo, PROCESSO *process);

int adicionaTabelaSemaforo(SEMAFORO *semaforo);

#endif

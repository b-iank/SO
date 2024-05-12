#ifndef SEMAFORO_H
#define SEMAFORO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include "processo.h"
#include "terminal.c"
#include "kernel.h"
#include "memoria.h"

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

int adicionaTabelaSemaforo(SEMAFORO *semaforo);

#endif

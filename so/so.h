#ifndef SO_H
#define SO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include "../terminal/terminal.h"

#define MAX_SEMAFOROS 10 //TODO: defiir quantos semaforos sao

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

// FUNÇÕES DO KERNEL
#define PROCESS_INTERRUPT '1'
#define PROCESS_CREATE '2'
#define PROCESS_FINISH '3'
#define DISK_REQUEST '4'
#define DISK_FINISH '5'
#define MEMORY_LOAD_REQUEST '6'
#define MEMORY_LOAD_FINISH '7'
#define FILE_SYSTEM_REQUEST '8'
#define FILE_SYSTEM_FINISH '9'
#define SEMAPHORE_P 'A' // 10
#define SEMAPHORE_V 'B' // 11
#define PRINT_REQUEST 'E' // 14
#define PRINT_FINISH 'F' // 15

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

typedef struct semaforo SEMAFORO;
typedef struct tabela_semaforo TABELA_SEMAFORO;

typedef struct processo PROCESSO;
typedef struct pcb PCB;

typedef struct instrucao INSTRUCAO;

typedef struct memoria MEMORIA;

typedef struct kernel KERNEL;

struct semaforo {
    char nome;
    int S;
    PROCESSO *aguardando;
    sem_t mutex;
};

struct tabela_semaforo {
    SEMAFORO semaforo[MAX_SEMAFOROS];
    int quantidadeSemaforos;
};

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

struct memoria {
    PROCESSO *process;
    INSTRUCAO *code;
};


struct kernel {
    PCB pcb; // <- Bloco de controle de processos
    int proxId; // <- Guarda o próximo id de processo

    /* Segment Table Information */
    // segment_table_t seg_table;

    /* Scheduler Information */
    // scheduler_t scheduler;

    /* Disk Scheduler Information */
    // disk_scheduler_t disk_scheduler;

    /* Semaforo Table Information */
    TABELA_SEMAFORO tabelaSemaforo; // <- Guarda a tabela de semáforo

    /* File Table Information */
    // file_table_t file_table;

    int pc; // <- Program Counter
};

extern KERNEL *kernel;

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
TABELA_SEMAFORO inciaTabelaSemaforo();
void novoSemaforo(char nome);
SEMAFORO *buscaSemaforo(char semaforo);
int existeSemaforoProcesso(char semaforo, PROCESSO *process);
int adicionaTabelaSemaforo(SEMAFORO *semaforo);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESSO --------------------------------------
PCB iniciaPCB();
PCB *add_process(PCB *lista, PROCESSO *processo);
void readSyntheticProgram(FILE *, PROCESSO **, INSTRUCAO **);
void processCreate(char *fileName);
void processInterrupt(PCB *lista);
void processFinish(PCB *lista);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES INSTRUÇÃO -------------------------------------
void instr_parse(INSTRUCAO *instr, const char *line, TABELA_SEMAFORO *sem_table);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES KERNEL ----------------------------------------
KERNEL *iniciaKernel();
void sysCall(char function, void *arg);
// ---------------------------------------------------------------------------------------------




#endif //SO_H

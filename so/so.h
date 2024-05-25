#ifndef SO_H
#define SO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../terminal/terminal.h"

#define MAX_SEMAFOROS 10

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

#define K 1024
#define TAMANHO_PAGINA 8 * K
#define TAMANHO_MAX_MEMORIA K *K *K // 1 GB = 1 * 1024 * 1024 * 1024

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

// SCHEDULER
#define NONE 0x0
#define IO_REQUESTED 0x1
#define QUANTUM_COMPLETED 0x2
#define SEMAPHORE_BLOCKED 0x4

#define ONE_SECOND_NS (1000000000L)
#define FETCH_INSTR_ADDR(x) ((x)->pc++)

typedef struct semaforo SEMAFORO;
typedef struct tabela_semaforo TABELA_SEMAFORO;

typedef struct processo PROCESSO;
typedef struct pcb PCB;

typedef struct instrucao INSTRUCAO;

typedef struct memoria MEMORIA;
typedef struct segmento SEGMENTO;
typedef struct tabela_segmento TABELA_SEGMENTO;

typedef struct kernel KERNEL;

struct semaforo {
    char nome;
    int S;
    int *idAguardando;
    int aguardando;
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
    clock_t tempo;
    clock_t tempoChegada;

    char estado;
    int pc;
    int numComandos;
    INSTRUCAO* codigo;

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

struct segmento {
    int id;
    int paginaQuant;
    int paginaQuantMemoria;
    int segundaChance; // 0 -> 1; 1 sai da memoria
};

struct tabela_segmento {
    SEGMENTO *segmentos;
    int quantSegmentos;
    int memoriaRestante;
    int atual;
};

struct kernel {
    PCB pcb; // <- Bloco de controle de processos
    int proxId; // <- Guarda o próximo id de processo

    /* Tabela de Segmentos */
    TABELA_SEGMENTO seg_table;

    /* Tabela de Semaforos */
    TABELA_SEMAFORO tabelaSemaforo; // <- Guarda a tabela de semáforo

    int pc; // <- Program Counter
};

extern KERNEL *kernel;

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
TABELA_SEMAFORO iniciaTabelaSemaforo();
void novoSemaforo(char nome);
SEMAFORO *buscaSemaforo(char semaforo);
int existeSemaforoProcesso(char semaforo, PROCESSO *process);
int adicionaTabelaSemaforo(SEMAFORO *semaforo);
void P(SEMAFORO *semaforo, PROCESSO *processo, void (*sleep)(void));
void V(SEMAFORO *semaforo, void (*wakeup)(PROCESSO *));
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESSO --------------------------------------
PCB iniciaPCB();
PCB *add_process(PCB *lista, PROCESSO *processo);
void readSyntheticProgram(FILE *, PROCESSO **, INSTRUCAO **);
void processCreate(char *fileName);
void processInterrupt(PCB *lista);
void processFinish(PCB *lista);
PROCESSO *buscaProcessoID(PCB pcb, int id);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES LOG -------------------------------------------

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
TABELA_SEGMENTO iniciaTabelaSegmentos();
MEMORIA *memoriaRequest(PROCESSO *processo, INSTRUCAO *codigo);
void memoriaLoadRequest(PROCESSO *processo);
void trocaPaginas(SEGMENTO *segmento, int requisicao);
void adicionaTabelaSegmentos(SEGMENTO *segmento);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES KERNEL ----------------------------------------
KERNEL *iniciaKernel();
void sysCall(char function, void *arg);
void interruptControl(char function, void *arg);
// ---------------------------------------------------------------------------------------------

// ----------------------------------- FUNÇÕES CPU --------------------------------------------
void cpu_init();
void cpu();
// ---------------------------------------------------------------------------------------------


#endif // SO_H

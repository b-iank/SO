#ifndef SO_H
#define SO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../terminal/terminal.h"

#define CLEAR system("clear");

#define MAX_SEMAPHORE 10

#define MAX_PROCESS_NAME 50
#define QUANTUM_TIME 5000

#define K 1024
#define MEMORY_PAGE_SIZE 8 // em bytes
#define MAX_MEMORY_SIZE (K * K * K) // 1 GB = 1 * 1024 * 1024 * 1024

// FUNÇÕES DO KERNEL
#define PROCESS_INTERRUPT '1'
#define PROCESS_CREATE '2'
#define PROCESS_FINISH '3'
#define MEMORY_LOAD_REQUEST '6'
#define MEMORY_LOAD_FINISH '7'
#define SEMAPHORE_P 'A' // 10
#define SEMAPHORE_V 'B' // 11

// ESTADOS DO PROCESSO
#define NEW '0'
#define BLOCKED '1'
#define READY '2'
#define RUNNING '3'
#define DONE '4'

// INSTRUÇÃO
#define EXEC '1'
#define READ '2'
#define WRITE '3'
#define PRINT '4'
#define SEM_P '5'
#define SEM_V '6'

// SCHEDULER
#define NONE 0x0
#define QUANTUM_COMPLETED 0x2
#define SEMAPHORE_BLOCKED 0x4
#define FINISHED 0x6

#define ONE_SECOND_NS (1000000000)
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) >= (b) ? (b) : (a))

typedef struct semaphore SEMAPHORE;
typedef struct semaphore_table SEMAPHORE_TABLE;

typedef struct process PROCESS;
typedef struct pcb PCB;

typedef struct code CODE;

typedef struct segment SEGMENT;
typedef struct segment_table SEGMENT_TABLE;

typedef struct scheduler SCHEDULER;
typedef struct process_scheduler PROCESS_SCHEDULER;

typedef struct kernel KERNEL;

struct semaphore {
    char name;
    int S;
    int *id_blocked;
    int qnt_blocked;
    sem_t mutex;
};

struct semaphore_table {
    SEMAPHORE *semaphores;
    int qnt_semaphore;
};

struct process {
    // Cabecalho
    char name[MAX_PROCESS_NAME];
    int segment_id;
    int priority;
    int segment_size;
    char semaphore[MAX_SEMAPHORE];
    int qnt_semaphore;

    int id;
    int max_time;
    int remaining_time;
    int arrival_time;

    char state;
    int pc;
    int qnt_code;
    CODE *code;

    PROCESS *next; // Lista de processos (PCB)
};

struct pcb {
    PROCESS *head;
    PROCESS *tail;
};

struct code {
    char op;
    int value;
    char sem;
};

struct segment {
    int id;
    int qnt_page; // Quantidade de paginas do process
    int qnt_page_memory; // Quantidade de paginas que estao na memoria
    int second_chance; // 1 -> 0 -> sai da memoria
};

struct segment_table {
    SEGMENT *segments;
    int qnt_segments;
    int remaining_memory;
    int curr; // Indice do segmento atual no algoritmo de segunda chance
};

struct process_scheduler {
    PROCESS *process;
    PROCESS_SCHEDULER *next;
};

struct scheduler {
    PROCESS_SCHEDULER *head;
    PROCESS_SCHEDULER *tail;
    PROCESS_SCHEDULER *scheduled;
};

struct kernel {
    PCB pcb; // <- Bloco de controle de processos
    int next_id; // <- Guarda o próximo id de process
    SEGMENT_TABLE segment_table; // <- Tabela de Segmentos
    SEMAPHORE_TABLE semaphore_table; // <- Guarda a tabela de semáforo
    SCHEDULER scheduler;
    int time;
};

extern KERNEL *kernel;
extern pthread_mutex_t mutex_scheduler;

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
SEMAPHORE_TABLE semaphore_table_init();
void new_semaphore(char name);
SEMAPHORE *find_semaphore(char semaphore_name);
int semaphore_process_exists(char semaphore_name, PROCESS *process);
void add_semaphore_table(SEMAPHORE *semaphore);
void P(SEMAPHORE *semaphore, PROCESS *process);
void V(SEMAPHORE *semaphore);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESSO --------------------------------------
PCB PCB_init();
void process_create(char *file_name);
PCB add_process(PROCESS *process);
PROCESS * read_synthetic_program(FILE *fp);
int count_codes(FILE *fp);
void process_finish(PROCESS *process);
PROCESS *find_process(int id);
void run_process(PROCESS *process);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
SEGMENT_TABLE segment_table_init();
void load_memory_page(SEGMENT *segment, int request);
void memory_load_request(PROCESS *process);
void page_request(PROCESS *process, SEGMENT *segment, int request);
void page_swap(int request);
void add_segment_table(SEGMENT *segment);
void segment_free(int segment_id, int process_id);
int find_segment(int segment_id);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES SCHEDULER --------------------------------------
SCHEDULER scheduler_init();
SCHEDULER add_process_scheduler(PROCESS *process);
SCHEDULER schedule_process(int flag);
void remove_scheduler(PROCESS *process);
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES LOG -------------------------------------------
void print_pcb_processes();
void print_running_process();
void print_code(char name[50], char op);
void print_segment_table();
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES KERNEL ----------------------------------------
KERNEL *kernel_init();
void sys_call(char function, void *arg);
void interrupt_control(char function, void *arg);
void process_sleep();
void process_wakeup(PROCESS *process);
// ---------------------------------------------------------------------------------------------

// ----------------------------------- FUNÇÕES CPU --------------------------------------------
void cpu_init();
void cpu();
// ---------------------------------------------------------------------------------------------


#endif // SO_H

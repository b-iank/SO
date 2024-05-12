#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include "processo.h"
#include "semaforo.h"

// Funções do KERNEL
const char PROCESS_INTERRUPT = '1';
const char PROCESS_CREATE = '2';
const char PROCESS_FINISH = '3';
const char DISK_REQUEST = '4';
const char DISK_FINISH = '5';
const char MEMORY_LOAD_REQUEST = '6';
const char MEMORY_LOAD_FINISH = '7';
const char FILE_SYSTEM_REQUEST = '8';
const char FILE_SYSTEM_FINISH = '9';
const char SEMAPHORE_P = 'A'; // 10
const char SEMAPHORE_V = 'B'; // 11
const char PRINT_REQUEST = 'E'; // 14
const char PRINT_FINISH = 'F'; // 15

typedef struct kernel {
    /* processo Table Information (aka PCB) */
    PCB pcb;
    int proxId;

    /* Segment Table Information */
    //segment_table_t seg_table;

    /* Scheduler Information */
    //scheduler_t scheduler;

    /* Disk Scheduler Information */
    //disk_scheduler_t disk_scheduler;

    /* Semaforo Table Information */
    TABELA_SEMAFORO tabelaSemaforo;

    /* File Table Information */
    //file_table_t file_table;

    int pc; /* Program Counter */
} KERNEL;

/* kernel Variables */

/**
 * A pointer to the KERNEL structure. This
 * variable must be initialized and will be
 * when the kernel_init function is called.
 */
extern KERNEL* kernel;

void iniciaKernel();
void sysCall(char function, void* arg);


#endif //KERNEL_H

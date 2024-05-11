#ifndef KERNEL_H
#define KERNEL_H
#include "process.h"
#include "semaphore.h"
// Funções do kernel
char PROCESS_INTERRUPT = '1';
char PROCESS_CREATE = '2';
char PROCESS_FINISH = '3';
char DISK_REQUEST = '4';
char DISK_FINISH = '5';
char MEMORY_LOAD_REQUEST = '6';
char MEMORY_LOAD_FINISH = '7';
char FILE_SYSTEM_REQUEST = '8';
char FILE_SYSTEM_FINISH = '9';
char SEMAPHORE_P = 'A'; // 10
char SEMAPHORE_V = 'B'; // 11
char PRINT_REQUEST = 'E'; // 14
char PRINT_FINISH = 'F'; // 15

typedef struct Kernel {
    /* Process Table Information (aka PCB) */
    pcb pcb;
    int next_proc_id;

    /* Segment Table Information */
    //segment_table_t seg_table;

    /* Scheduler Information */
    //scheduler_t scheduler;

    /* Disk Scheduler Information */
    //disk_scheduler_t disk_scheduler;

    /* Semaphore Table Information */
    semaphore_table_t sem_table;

    /* File Table Information */
    //file_table_t file_table;

    int pc; /* Program Counter */
} kernel_t;

/* Kernel Variables */

/**
 * A pointer to the kernel structure. This
 * variable must be initialized and will be
 * when the kernel_init function is called.
 */
extern kernel_t* kernel;


void sysCall(char function, void* arg);


#endif //KERNEL_H

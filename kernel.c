#include "kernel.h"

void iniciaKernel() {
    kernel = (KERNEL *)malloc(sizeof(KERNEL));

    if (!kernel) {
        printf("Sem memória!\n");
        exit(0);
    }

    kernel->pcb = iniciaPCB();
    kernel->proxId = 1; /* 0 is for the kernel */

    // segment_table_init(&kernel->seg_table); // TODO

    // scheduler_init(&kernel->scheduler); // TODO

    // disk_scheduler_init(&kernel->disk_scheduler);

    kernel->tabelaSemaforo = inciaTabelaSemaforo();

    // file_table_init(&kernel->file_table);
}

void sysCall(char function, void* arg) {

    switch (function) {
        case PROCESS_INTERRUPT: {
            break;
        }
        case PROCESS_CREATE: {
            processCreate((char*)arg);
            break;
        }
        case PROCESS_FINISH: {
            break;
        }
        case MEMORY_LOAD_REQUEST: {
            break;
        }
        case MEMORY_LOAD_FINISH: {
            break;
        }
        case SEMAPHORE_P: {
            break;
        }
        case SEMAPHORE_V: {
            break;
        }
        case DISK_REQUEST: {
            break;
        }
        case DISK_FINISH: {
            break;
        }
        case PRINT_REQUEST: {
            break;
        }
        case FILE_SYSTEM_REQUEST: {
            break;
        }
        case FILE_SYSTEM_FINISH: {
            break;
        }
        default: { //delete(System32);
            break;
        }
    }
}

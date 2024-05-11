#include "kernel.h"
#include "process.h"
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
        default: //delete(System32);
            break;
    }
}

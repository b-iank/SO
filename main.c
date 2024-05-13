#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "so/so.h"

clock_t inicio;

KERNEL *kernel;

int main() {
    int op;
    char fileName[255];

    inicio = clock();

    //process_log_init();
    // memory_log_init();

     kernel = iniciaKernel();


    // disk_init();
    // cpu_init();

    do {
        op = main_menu();
        if (op == 1) {
            scanf("%s", fileName);
            processInterrupt(&kernel->pcb);
            sysCall(PROCESS_CREATE, fileName);
        } else if (op == 2) {
            // a
        } else if (op == 3) {
            //
        } else {
            printf("Adeus Aleardo :D");
        }
    } while (op != 0);
    return 0;
}

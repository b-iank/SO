#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "so/so.h"

clock_t inicio;

KERNEL *kernel;

void tela();

int main() {
    inicio = clock();

    // process_log_init();
    // memory_log_init();

    kernel = iniciaKernel();

    cpu_init();

    pthread_t menu;

    pthread_create(&menu, NULL, (void*) tela, NULL);

    return 0;
}

void tela() {
    int op;
    char fileName[255];
    do {
        op = main_menu();
        if (op == 1) {
            scanf("%s", fileName);
            processInterrupt();
            sysCall(PROCESS_CREATE, fileName);
        } else if (op == 2) {
            // a
        } else if (op == 3) {
            //
        } else {
            printf("Adeus Aleardo :D");
        }
    } while (op != 0);
}
#include <pthread.h>
#include <stdio.h>

#include "so/so.h"

KERNEL *kernel;

void tela();

int main() {
    kernel = iniciaKernel();

    //processCreate("synt2");
    cpu_init();

    tela();
    return 0;
}

void tela() {
    printaNome();
    int op;
    char fileName[255];
    do {
        op = main_menu();
        if (op == 1) {
            scanf("%s", fileName);
            sysCall(PROCESS_CREATE, fileName);
        } else if (op == 2) {
            printaProcessos();
        } else if (op == 3) {
            if (kernel->scheduler.scheduled == NULL)
                schedule_process(NONE);
            else
                printf("%s %d\n", kernel->scheduler.scheduled->processo->nome, kernel->scheduler.scheduled->processo->id);
            // printaMemoria();
        } else {
            printf("Adeus Aleardo :D");
        }
    } while (op != 0);
}

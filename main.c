#include <stdio.h>
#include <unistd.h>

#include "so/so.h"

KERNEL *kernel;

void tela();

int main() {
    print_name();
    sleep(5);

    kernel = kernel_init();
    cpu_init();

    tela();
    return 0;
}

void tela() {
    int op;
    char file_name[255];
    do {
        op = main_menu();
        if (op == 1) {
            printf("Digite o nome do arquivo: ");
            scanf("%s", file_name);
            sys_call(PROCESS_CREATE, file_name);
        } else if (op == 2)
            print_running_process();
        else if (op == 3)
            print_pcb_processes();
        else if (op == 4)
            print_segment_table();
        else
            printf("Adeus Aleardo :D");
    } while (op != 0);
}

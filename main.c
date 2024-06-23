#include <stdio.h>
#include <unistd.h>

#include "so/so.h"

KERNEL *kernel;

void screen();

int main() {
    print_name();
    sleep(3);

    kernel = kernel_init();
    disk_init();
    cpu_init();


    screen();
    return 0;
}

void screen() {
    int op;
    char file_name[255], next;
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
        else if (op == 5)
            print_disk_usage();
        else
            so_sucess("Adeus Aleardo :D");
    } while (op != 0);
}

#include <stdio.h>
#include <stdlib.h>
#include "process.h"
#include "list.h"

pcb *BCP;

sem_t log_mutex;
sem_t mem_mutex;
sem_t disk_mutex;
sem_t refresh_sem;
sem_t io_mutex;
sem_t res_acq_mutex;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t refresh_mutex = PTHREAD_MUTEX_INITIALIZER;

list_t* process_log_list;
list_t* disk_log_list;
list_t* io_log_list;
list_t* res_acq_log_list;


void main_menu() {
    char file_name[31];
    BCP = malloc(sizeof(pcb));
    BCP->head = NULL;
    BCP->tail = NULL;
    BCP->current = NULL;

    int op;
    printf("Menu\n");
    printf("1 - Executar processo\n");
    printf("2 - Ver processos\n");
    printf("3 - Ver memória\n");
    scanf("%d", &op);

    if (op == 1) {
        scanf("%s", file_name);
        processInterrupt(BCP);
        sysCall(PROCESS_CREATE, fileName);


        // interromper processo
        // continuar execução
    } else if (op == 2) {
        // mostrar
    } else if (op == 3) {
        // mostrar
    } else {
        printf("Operação não suportada\n");
    }
}

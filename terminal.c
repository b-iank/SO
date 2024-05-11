#include "terminal.h"

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
    char fileName[31];

    int op;
    printf("Menu\n");
    printf("1 - Executar PROCESSO\n");
    printf("2 - Ver processos\n");
    printf("3 - Ver memória\n");
    scanf("%d", &op);

    if (op == 1) {
        scanf("%s", fileName);
        processInterrupt(&kernel->pcb);
        sysCall(PROCESS_CREATE, fileName);
    } else if (op == 2) {
        // mostrar
    } else if (op == 3) {
        // mostrar
    } else {
        printf("Operação não suportada\n");
    }
}

void alerta (char * mensagem) {
    printf("%s", mensagem);
}

void erro(char *mensagem) {
    printf("%s", mensagem);
}

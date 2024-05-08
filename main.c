#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "terminal.c"
sem_t semaphore;

void *processInterruptHandler(void *arg);
void *semaphorePHandler(void *arg);

// processInterrupt (1): interrupção de sistema (processCreate), término de operações de E/S.
// semaphoreP (10): bloqueio de um processo que está tentando acessar um recurso protegido por um semáforo.
// semaphoreV (11): desbloqueio de um processo que estava aguardando a liberação de um semáforo.
// DiskRequest (4): operação de entrada/saída para disco.
// DiskFinish (5): o término de uma operação de E/S de disco.
// PrintRequest (14): operação de entrada/saída para impressão.
// PrintFinish (15): sinalizar o término de uma operação de E/S de impressão.
// memLoadReq (6): chamada de operação de carregamento na memória.
// memLoadFinish (7): término de uma operação de carregamento na memória.
// fRequest (8): chamada de operação no sistema de arquivos.
// fsFinish (9): término de uma operação no sistema de arquivos.
// processCreate (2): iniciar a criação de processo.
// processFinish (3): terminar a existência de processo.

int main(){
    process_log_init();
    memory_log_init();

    kernel_init();
    disk_init();
    cpu_init();

    main_menu();

    return 0;
    // pthread_t interruptThread, semaphorePThread;
    // // Declare as variáveis para as outras threads aqui
    //
    // // Criação das threads para cada tipo de evento
    // pthread_create(&interruptThread, NULL, processInterruptHandler, NULL);
    // pthread_create(&semaphorePThread, NULL, semaphorePHandler, NULL);
}


void *processInterruptHandler(void *arg) {
    printf("Thread para tratar interrupções de processo iniciada\n");

    pthread_exit(NULL);
    return NULL;
}

void *semaphorePHandler(void *arg) {
    printf("Thread para tratar bloqueio de semáforo iniciada\n");
    pthread_exit(NULL);
    return NULL;
}
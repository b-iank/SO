#include <pthread.h>
#include <semaforo.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "terminal.h"

clock_t inicio;

int main() {

    inicio = clock();

    process_log_init();
    memory_log_init();

    kernel_init();
    disk_init();
    cpu_init();

    main_menu();

    return 0;
}

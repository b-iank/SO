#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "terminal.c"

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

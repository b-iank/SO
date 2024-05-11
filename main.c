#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "log.h"
#include "terminal.c"

int main(){
    process_log_init();
    memory_log_init();

    kernel_init();
    disk_init();
    cpu_init();

    main_menu();

    return 0;
}

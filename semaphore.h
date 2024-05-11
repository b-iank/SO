#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef struct Semaphore {
    char *name;
    int S;
    list_t *waiters;
    sem_t mutex;
} semaphore;

typedef struct SemaphoreTable {
    semaphore *table;
    int len;
} semaphore_table;

#endif

#ifndef MEMORY_H
#define MEMORY_H

#include "process.h"

typedef struct {
    process_t* process;
    instruction* code;
} memoryRequest;


#endif //MEMORY_H

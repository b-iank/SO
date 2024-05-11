#ifndef MEMORY_H
#define MEMORY_H

#include "process.h"

typedef struct memoria {
    PROCESSO* process;
    instruction* code;
} MEMORIA;


#endif //MEMORY_H

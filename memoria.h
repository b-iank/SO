#ifndef MEMORY_H
#define MEMORY_H

#include "processo.h"

typedef struct memoria {
    PROCESSO* process;
    instruction* code;
} MEMORIA;


#endif //MEMORY_H

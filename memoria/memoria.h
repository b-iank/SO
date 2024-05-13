#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../processo/processo.h"

typedef struct memoria MEMORIA;

struct memoria {
    PROCESSO *process;
    INSTRUCAO *code;
};


#endif //MEMORIA_H

#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "processo.h"
#include "instrucao.h"

typedef struct memoria {
    PROCESSO *process;
    INSTRUCAO *code;
} MEMORIA;


#endif //MEMORIA_H

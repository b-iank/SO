#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "processo/processo.h"
#include "instrucao/instrucao.h"

typedef struct memoria {
    PROCESSO *process;
    INSTRUCAO *code;
} MEMORIA;


#endif //MEMORIA_H

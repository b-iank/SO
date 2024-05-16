#include "../so/so.h"

int ID_SEGMENTOS = 0;

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
TABELA_SEMAFORO inciaTabelaSemaforo() {
    TABELA_SEMAFORO tabelaSemaforo;
    tabelaSemaforo.quantidadeSemaforos = 0;

    return tabelaSemaforo;
}

void novoSemaforo(char nome) {
    SEMAFORO *semaforo = buscaSemaforo(nome);
    if (semaforo)
        return;
    semaforo = malloc(sizeof(SEMAFORO));
    if (!semaforo) {
        erro("Sem memoria");
        exit(EXIT_FAILURE);
    }

    semaforo->nome = nome;
    semaforo->S = 1; //TODO ?
    semaforo->aguardando = NULL;
    sem_init(&semaforo->mutex, 0, 1);

    if (!adicionaTabelaSemaforo(semaforo)) {
        char *mensagem = malloc(255);
        sprintf(mensagem, "Nao foi possivel adicionar o semaforo %c", nome);
        erro(mensagem);
        exit(EXIT_FAILURE);
    }
}

int adicionaTabelaSemaforo(SEMAFORO *semaforo) {
    if (kernel->tabelaSemaforo.quantidadeSemaforos == MAX_SEMAFOROS)
        return 0;

    kernel->tabelaSemaforo.semaforo[++kernel->tabelaSemaforo.quantidadeSemaforos] = *semaforo;
    return 1;
}

SEMAFORO *buscaSemaforo(char semaforo) {
    TABELA_SEMAFORO tabela = kernel->tabelaSemaforo;
    SEMAFORO *retorno = NULL;
    for (int i = 0; i < MAX_SEMAFOROS && !retorno; i++) {
        if (tabela.semaforo[i].nome == semaforo)
            retorno = &tabela.semaforo[i];
    }

    return retorno;
}

int existeSemaforoProcesso(char semaforo, PROCESSO *process) {
    for (int i = 0; i < process->quantidadeSemaforos; i++) {
        if (process->semaforos[i] == semaforo) {
            return 1;
        }
    }
    return 0;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESSO --------------------------------------
void processCreate(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    PROCESSO *process = NULL;
    INSTRUCAO *code = NULL;

    if (!(fp = fopen(fileName, "r"))) {
        exit(EXIT_FAILURE);
    }

    readSyntheticProgram(fp, &process, &code);

    MEMORIA *memReq = memoriaRequest(process, code);

    sysCall(MEMORY_LOAD_REQUEST, (void *) &memReq);
}

PCB iniciaPCB() {
    PCB pcb;
    pcb.head = NULL;
    pcb.tail = NULL;
    pcb.atual = NULL;

    return pcb;
}

PCB *add_process(PCB *TCP, PROCESSO *processo) {
    PCB *lista_aux = TCP;
    PROCESSO *head = lista_aux->head, *aux = head, *anterior = lista_aux->tail;

    if (head == NULL) {
        lista_aux->head = processo;
        lista_aux->tail = processo;
        processo->prox = processo;
        return lista_aux;
    }

    while (processo->prioridade > aux->prioridade && aux->prox != head) {
        anterior = aux;
        aux = aux->prox;
    }

    processo->prox = aux;
    anterior->prox = processo;

    if (processo->prioridade < head->prioridade)
        lista_aux->head = processo;
    if (processo->prox == lista_aux->head)
        lista_aux->tail = processo;

    return lista_aux;
}

void readSyntheticProgram(FILE *arquivo, PROCESSO **process, INSTRUCAO **code) {
    long int code_section;
    int countCode;
    long tamanhoArquivo;

    (*process) = malloc(sizeof(PROCESSO));

    // Le cabecalho
    fscanf(arquivo, "%s %d %d %d", (*process)->nome, &(*process)->idSegmento, &(*process)->prioridade,
           &(*process)->tamanhoSegmento);

    (*process)->id = kernel->proxId++; // Pega o próximo id de processo e soma a variável
    (*process)->pc = 0;
    (*process)->estado = NOVO;
    (*process)->tempoRestante = (QUANTUM_TIME / (*process)->prioridade);
    (*process)->tempoChegada = clock();

    (*process)->quantidadeSemaforos = 0;
    while (1) {
        char semaforo;
        if (fscanf(arquivo, "%c", &semaforo) != 1)
            break;

        if (existeSemaforoProcesso(semaforo, *process)) {
            char *mensagem = malloc(21);
            sprintf(mensagem, "Semaforo %c ja existe", semaforo);
            alerta(mensagem);
            continue;
        }

        (*process)->semaforos[(*process)->quantidadeSemaforos++] = semaforo;
        novoSemaforo(semaforo);
    }

    // Le codigo
    code_section = ftell(arquivo);
    countCode = 0;
    fseek(arquivo, 0, SEEK_END); // Move o ponteiro para o final do arquivo
    tamanhoArquivo = ftell(arquivo); // Obtém o tamanho total do arquivo

    // Conta a quantidade de linhas restantes
    while (ftell(arquivo) < tamanhoArquivo) {
        if (fgetc(arquivo) == '\n') {
            countCode++;
        }
    }

    fseek(arquivo, code_section, SEEK_SET);

    (*code) = (INSTRUCAO *) malloc(sizeof(INSTRUCAO) * (countCode));

    if (!(*code)) {
        alerta("Sem memoria");
        exit(EXIT_FAILURE);
    }

    (*process)->numComandos = countCode;

    char comando[51];
    int i = 0;
    while (fgets(comando, 51, arquivo) != NULL) {
        if (comando[0] == 'P' || comando[0] == 'V') {
            if (!existeSemaforoProcesso(comando[2], *process)) {
                char mensagem[255];
                sprintf(mensagem, "O semaforo %c nao existe.", comando[2]);
                erro(mensagem);
                exit(EXIT_FAILURE);
            }
            (*code)[i].op = comando[0] == 'P' ? SEM_P : SEM_V;
            (*code)[i].sem = comando[2];
        } else { // exec 1000
            char *left_op = malloc(sizeof(char) * 6);
            int right_op;

            sscanf(comando, "%s %d", left_op, &right_op);

            if (strcmp(left_op, "exec") == 0)
                (*code)[i].op = EXEC;
            else if (strcmp(left_op, "read") == 0)
                (*code)[i].op = READ;
            else if (strcmp(left_op, "write") == 0)
                (*code)[i].op = WRITE;
            else if (strcmp(left_op, "print") == 0)
                (*code)[i].op = PRINT;
            else {
                erro("Operacao invalida");
                exit(EXIT_FAILURE);
            }
            (*code)[i].value = right_op;
            (*code)[i].sem = '#'; // TODO: podemos usar # pra 'não semaforo'?
        }
        i++;
    }

    fclose(arquivo);
}


void processInterrupt(PCB *BCP) {
    printf("Interrupcao do PROCESSO %d\n", BCP->atual->id);
    // Realizar as ações necessárias para tratar a interrupção
    // troca de contexto etc
    if (BCP->atual->tempoRestante <= 0) {
        processFinish(BCP);
    }
}

void processFinish(PCB *BCP) {
    printf("Finalizando o PROCESSO %d\n", BCP->atual->id);
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES INSTRUÇÃO -------------------------------------

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
MEMORIA * memoriaRequest(PROCESSO * processo, INSTRUCAO * codigo) {
    MEMORIA *memoria = malloc(sizeof(MEMORIA));
    if (!memoria) {
        erro("Sem memoria.");
        exit(EXIT_FAILURE);
    }
    memoria->process = processo;
    memoria->code = codigo;
    return memoria;
}

void memoriaLoadRequest(MEMORIA *memReq) {
    TABELA_SEGMENTO tabelaSegmentos = kernel->seg_table;
    SEGMENTO *segmento = malloc(sizeof(SEGMENTO));
    int i = tabelaSegmentos.quantSegmentos;
    tabelaSegmentos.quantSegmentos++;

    segmento->id = ID_SEGMENTOS++;
    segmento->pageQuant = (int) (memReq->process->tamanhoSegmento)/(TAMANHO_PAGINA); // TODO: truncate

    const int restante = tabelaSegmentos.memoriaRestante - memReq->process->tamanhoSegmento;

    tabelaSegmentos.memoriaRestante = restante;

    if (restante < 0) {
        tabelaSegmentos.memoriaRestante += trocarPaginas(segmento);
    }

    adicionaTabelaSegmentos(segmento);
}

int trocarPaginas(SEGMENTO segmento) {

    return 0;
}

void adicionaTabelaSegmentos(SEGMENTO *segmento) {
    return;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES KERNEL ----------------------------------------
KERNEL *iniciaKernel() {
    kernel = (KERNEL *) malloc(sizeof(KERNEL));

    if (!kernel) {
        erro("Sem memoria");
        exit(EXIT_FAILURE);
    }

    kernel->pcb = iniciaPCB();
    kernel->proxId = 1; /* 0 is for the kernel */
    kernel->pc = 0; // TODO ?

    // segment_table_init(&kernel->seg_table); // TODO

    // scheduler_init(&kernel->scheduler); // TODO

    // disk_scheduler_init(&kernel->disk_scheduler);

    kernel->tabelaSemaforo = inciaTabelaSemaforo();

    // file_table_init(&kernel->file_table);

    return kernel;
}

void sysCall(char function, void *arg) {

    switch (function) {
        case PROCESS_INTERRUPT: {
            break;
        }
        case PROCESS_CREATE: {
            processCreate((char *) arg);
            break;
        }
        case PROCESS_FINISH: {
            break;
        }
        case MEMORY_LOAD_REQUEST: {
            memoriaLoadRequest((MEMORIA *)arg);
            //interruptControl(MEM_LOAD_FINISH, (MEMORIA *)arg);
            break;
        }
        case MEMORY_LOAD_FINISH: {
            break;
        }
        case SEMAPHORE_P: {
            break;
        }
        case SEMAPHORE_V: {
            break;
        }
        // case DISK_REQUEST: {
        //     break;
        // }
        // case DISK_FINISH: {
        //     break;
        // }
        // case PRINT_REQUEST: {
        //     break;
        // }
        // case FILE_SYSTEM_REQUEST: {
        //     break;
        // }
        // case FILE_SYSTEM_FINISH: {
        //     break;
        // }
        default: { //delete(System32);
            break;
        }
    }
}
// ---------------------------------------------------------------------------------------------


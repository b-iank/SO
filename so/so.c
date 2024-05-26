#include "../so/so.h"

#include <math.h>

int ID_SEGMENTOS = 0;

static void sleep();

static void wakeup(PROCESSO *proc);

// ------------------------------------- FUNÇÕES SEMÁFOROS -------------------------------------
TABELA_SEMAFORO iniciaTabelaSemaforo() {
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
    semaforo->S = 1;
    semaforo->aguardando = 0;
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

void P(SEMAFORO *semaforo, PROCESSO *processo, void (*sleep)(void)) {
    sem_wait(&semaforo->mutex);
    semaforo->S--;
    if (semaforo->S < 0) {
        semaforo->idAguardando = realloc(semaforo->idAguardando, ++semaforo->aguardando * sizeof(int));
        if (!semaforo->idAguardando) {
            erro("Sem memoria");
            exit(EXIT_FAILURE);
        }
        semaforo->idAguardando[semaforo->aguardando - 1] = processo->id;
        sleep();
    }
    sem_post(&semaforo->mutex);
}

void V(SEMAFORO *semaforo, void (*wakeup)(PROCESSO *)) {
    sem_wait(&semaforo->mutex);
    semaforo->S++;
    if (semaforo->S <= 0 && semaforo->aguardando > 0) {
        semaforo->aguardando--;
        int id = semaforo->idAguardando[0], aux;
        for (int i = 0; i < semaforo->aguardando; i++)
            semaforo->idAguardando[i] = semaforo->idAguardando[i + 1];
        PROCESSO *aguardando = buscaProcessoID(id);
        wakeup(aguardando);
    }
    sem_post(&semaforo->mutex);
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

    process->codigo = code;

    sysCall(MEMORY_LOAD_REQUEST, (void *) &process);
}

PCB iniciaPCB() {
    PCB pcb;
    pcb.head = NULL;
    pcb.tail = NULL;
    pcb.atual = NULL;

    return pcb;
}

PCB add_process(PROCESSO *processo) {
    PCB lista_aux = kernel->pcb;
    PROCESSO *head = lista_aux.head, *aux = head, *anterior = lista_aux.tail;

    if (head == NULL) {
        lista_aux.head = processo;
        lista_aux.tail = processo;
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
        lista_aux.head = processo;
    if (processo->prox == lista_aux.head)
        lista_aux.tail = processo;

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
    (*process)->tempoMaximo = (QUANTUM_TIME / (*process)->prioridade);
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
            (*code)[i].sem = '#';
        }
        i++;
    }

    fclose(arquivo);
}


void processInterrupt() {
    PCB *BCP = &kernel->pcb;
    printf("Interrupcao do PROCESSO %d\n", BCP->atual->id);

    if (BCP->atual->tempoMaximo <= 0) {
        processFinish(BCP);
    }
}

void processFinish() {
    PCB *BCP = &kernel->pcb;
    printf("Finalizando o PROCESSO %d\n", BCP->atual->id);
}

PROCESSO *buscaProcessoID(int id) {
    PROCESSO *busca = kernel->pcb.head;
    while (busca) {
        if (busca->id == id)
            return busca;
    }
    char mensagem[255];
    sprintf(mensagem, "Processo %d nao encontrado", id);
    erro(mensagem);
    exit(EXIT_FAILURE);
}

void avalia(PROCESSO *processo) {
    INSTRUCAO *instr = &processo->codigo[processo->pc];
    switch (instr->op) {
        case EXEC: {
            if (processo->tempoMaximo < instr->value)
                processo->codigo->value = instr->value - processo->tempoMaximo;
            else {
                processo->tempoRestante = processo->tempoRestante - instr->value;
                processo->pc++;
            }
            break;
        }
        case SEM_P: {
            sysCall(SEMAPHORE_P, buscaSemaforo(instr->sem));

            if (processo->estado != BLOQUEADO)
                processo->tempoRestante = MAX(0, processo->tempoRestante - 200);
            break;
        }
        case SEM_V: {
            sysCall(SEMAPHORE_V, buscaSemaforo(instr->sem));
            processo->tempoRestante = MAX(0, processo->tempoRestante - 200);
            break;
        }
        case READ: {
            // sysCall(DISK_READ_REQUEST, instr->value);
            break;
        }
        case WRITE: {
            // sysCall(DISK_WRITE_REQUEST, instr->value);
            break;
        }
        case PRINT: {
            // sysCall(PRINT_REQUEST, (void*) instr->value);
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES INSTRUÇÃO -------------------------------------

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES MEMÓRIA ---------------------------------------
TABELA_SEGMENTO iniciaTabelaSegmentos() {
    TABELA_SEGMENTO tabelaSegmento;
    tabelaSegmento.segmentos = NULL;
    tabelaSegmento.quantSegmentos = 0;
    tabelaSegmento.memoriaRestante = 1073741824; // (1024^3);

    return tabelaSegmento;
}

MEMORIA *memoriaRequest(PROCESSO *processo, INSTRUCAO *codigo) {
    MEMORIA *memoria = malloc(sizeof(MEMORIA));
    if (!memoria) {
        erro("Sem memoria.");
        exit(EXIT_FAILURE);
    }
    memoria->process = processo;
    memoria->code = codigo;
    return memoria;
}

void memoriaLoadRequest(PROCESSO *processo) {
    TABELA_SEGMENTO *tabelaSegmentos = &kernel->seg_table;
    SEGMENTO *segmento = malloc(sizeof(SEGMENTO));

    segmento->id = ++ID_SEGMENTOS;

    segmento->paginaQuant = (int) ceil((double) (processo->tamanhoSegmento) / (TAMANHO_PAGINA));
    processo->idSegmento = segmento->id;

    const int restante = tabelaSegmentos->memoriaRestante - processo->tamanhoSegmento;

    tabelaSegmentos->memoriaRestante = restante;

    if (restante < 0)
        trocaPaginas(segmento, restante);
    else {
        adicionaTabelaSegmentos(segmento);
    }
}

void trocaPaginas(SEGMENTO *novoSegmento, int requisicao) {
    requisicao = requisicao * -1;
    TABELA_SEGMENTO *tabelaSegmentos = &kernel->seg_table;
    while (tabelaSegmentos->memoriaRestante < 0) {
        SEGMENTO segmentoAtual = tabelaSegmentos->segmentos[tabelaSegmentos->atual];
        if (segmentoAtual.segundaChance && segmentoAtual.paginaQuantMemoria > 0) {
            segmentoAtual.segundaChance = 0;
        } else {
            while (tabelaSegmentos->memoriaRestante < requisicao && segmentoAtual.paginaQuantMemoria > 0) {
                segmentoAtual.paginaQuantMemoria--;
                tabelaSegmentos->memoriaRestante += 8 * K;
            }
            segmentoAtual.segundaChance = 1;
        }
        tabelaSegmentos->atual = (tabelaSegmentos->atual + 1) % tabelaSegmentos->quantSegmentos;
    }
    adicionaTabelaSegmentos(novoSegmento);
}

void adicionaTabelaSegmentos(SEGMENTO *segmento) {
    TABELA_SEGMENTO *tabelaSegmentos = &kernel->seg_table;
    int i = ++tabelaSegmentos->quantSegmentos;

    tabelaSegmentos->segmentos = (SEGMENTO *) realloc(tabelaSegmentos->segmentos, sizeof(SEGMENTO) * i);

    if (!tabelaSegmentos->segmentos) {
        erro("Sem memoria");
        exit(EXIT_FAILURE);
    }

    tabelaSegmentos->segmentos[i - 1] = *segmento;
}

// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES SCHEDULER -------------------------------------
SCHEDULER add_process_scheduler(PROCESSO *processo) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESSO_SCHEDULER *head = scheduler.head, *anterior = scheduler.tail;

    PROCESSO_SCHEDULER *novo = malloc(sizeof (PROCESSO_SCHEDULER));
    if (!novo) {
        erro("Sem memoria");
        exit(EXIT_FAILURE);
    }
    novo->processo = processo;
    novo->prox = NULL;

    if (head == NULL) {
        scheduler.head = novo;
        scheduler.tail = novo;
        scheduler.head->prox = scheduler.tail;
        scheduler.tail->prox = scheduler.head;
        return scheduler;
    }

    PROCESSO_SCHEDULER *aux = scheduler.head;
    PROCESSO *aux_processo = aux->processo;
    while (processo->prioridade > aux_processo->prioridade && aux->prox != head) {
        anterior = aux;
        aux = aux->prox;
        aux_processo = aux->processo;
    }

    novo->prox = aux;
    anterior->prox = novo;

    if (processo->prioridade < head->processo->prioridade)
        scheduler.head = novo;
    if (novo->prox == scheduler.head)
        scheduler.tail = novo;

    return scheduler;
}

void schedule_process(int flag) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESSO_SCHEDULER *atual = scheduler.scheduled, *novo = NULL;

    PROCESSO_SCHEDULER *aux = atual;
    while (aux && aux->prox->processo->estado == BLOQUEADO && aux->prox != atual)
        aux = aux->prox;

    if (aux && aux->prox != atual) {
        novo = atual->prox;
        novo->processo->tempoRestante = novo->processo->tempoMaximo;
    }

    if (atual) {
        if (flag == IO_REQUESTED || flag == SEMAPHORE_BLOCKED) {
            atual->processo->estado = BLOQUEADO;
        } else if (flag == QUANTUM_COMPLETED) {
            atual->processo->estado = PRONTO;
        }
    }

    if (novo)
        novo->processo->estado = EXECUTANDO;

    scheduler.scheduled = novo;
}

SCHEDULER iniciaScheduler() {
    SCHEDULER scheduler;
    scheduler.scheduled = NULL;
    scheduler.head = NULL;
    scheduler.tail = NULL;
    scheduler.bloqueados = NULL;

    return scheduler;
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
    kernel->pc = 0;

    kernel->seg_table = iniciaTabelaSegmentos();

    kernel->scheduler = iniciaScheduler();

    kernel->tabelaSemaforo = iniciaTabelaSemaforo();

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
            memoriaLoadRequest((PROCESSO *) arg);
            interruptControl(MEMORY_LOAD_FINISH, (PROCESSO *) arg);
            break;
        }
        case SEMAPHORE_P: {
            P((SEMAFORO *) arg, kernel->pcb.atual, sleep);
            break;
        }
        case SEMAPHORE_V: {
            V((SEMAFORO *) arg, wakeup);
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
        default: { // delete(System32);
            break;
        }
    }
}

static void sleep() { sysCall(PROCESS_INTERRUPT, (void *) 0x4); }

static void wakeup(PROCESSO *processo) {
    processo->estado = PRONTO;
}

void interruptControl(char function, void *arg) {
    switch (function) {
        case MEMORY_LOAD_FINISH: {
            PROCESSO *processo = (PROCESSO *) arg;

            kernel->pcb = add_process(processo); // Adiciona o processo na PCB
            processo->estado = PRONTO;

            add_process_scheduler(processo);
            pthread_mutex_unlock(&criacao);
            break;
        }
        default: {
            char mensagem[255];
            sprintf(mensagem, "O processo %c nao esta definido.", function);
            alerta(mensagem);
        }
    }
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES CPU -------------------------------------------
void cpu_init() {
    pthread_t cpu_id;
    pthread_attr_t cpu_attr;

    pthread_mutex_init(&criacao, NULL);

    pthread_attr_init(&cpu_attr);
    pthread_attr_setscope(&cpu_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&cpu_id, NULL, (void*)cpu, NULL);
}

_Noreturn void cpu() {
    while (!kernel)
        ;

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start);

    int no_process = 0;
    while (1) {
        pthread_mutex_lock(&criacao);
        if (!kernel->scheduler.scheduled) {
            if (!no_process) {
                //sem_post(&refresh_sem);
                no_process = 1;
            }

            schedule_process(NONE);
        }
        else {
            no_process = 0;
            do {
                clock_gettime(CLOCK_REALTIME, &end);
                const int elapsed = (end.tv_sec - start.tv_sec) * ONE_SECOND_NS
                                    + (end.tv_nsec - start.tv_nsec);

                if (elapsed >= ONE_SECOND_NS) {
                    start = end;
//                    if (!page->used)
//                        page->used = 1;

//                    process_log(kernel->scheduler.scheduled_proc->name,
//                                kernel->scheduler.scheduled_proc->remaining,
//                                pc,
//                                seg->id,
//                                kernel->scheduler.scheduled_proc->o_files->size);
//                    sem_post(&log_mutex);
//                    sem_post(&refresh_sem);

                    avalia(kernel->scheduler.scheduled->processo);
                }
            } while (kernel->scheduler.scheduled != NULL
                     && kernel->scheduler.scheduled->processo->tempoRestante > 0
                     && kernel->scheduler.scheduled->processo->pc
                            < kernel->scheduler.scheduled->processo->numComandos);

            if (kernel->scheduler.scheduled == NULL)
                continue;

            if (kernel->scheduler.scheduled->processo->pc
                >= kernel->scheduler.scheduled->processo->numComandos)
                sysCall(PROCESS_FINISH, kernel->scheduler.scheduled);
            else
                sysCall(PROCESS_INTERRUPT, (void*)QUANTUM_COMPLETED);
        }
        pthread_mutex_unlock(&criacao);
    }
}

// ---------------------------------------------------------------------------------------------

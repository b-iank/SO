#include "../so/so.h"

#include <math.h>

int ID_SEGMENTOS = 0;

pthread_mutex_t criacao;

static void sleep();

static void wakeup(PROCESSO *proc);

void printaProcesso(int id, char nome[50], char estado, int prioridade);
void printaSegmento(int id, int quantidade);

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

int adicionaTabelaSemaforo(SEMAFORO *semaforo) { // TODO: realocar
    if (kernel->tabelaSemaforo.quantidadeSemaforos == MAX_SEMAFOROS)
        return 0;

    kernel->tabelaSemaforo.semaforo[kernel->tabelaSemaforo.quantidadeSemaforos++] = *semaforo;
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
        if (!aguardando)
            return;
        wakeup(aguardando);
    }
    sem_post(&semaforo->mutex);
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES PROCESSO --------------------------------------
void processCreate(char *fileName) {
    FILE *fp;
    PROCESSO *process = NULL;

    if (!(fp = fopen(fileName, "r"))) {
        alerta("Arquivo nao encontrado");
        return;
    }

    readSyntheticProgram(fp, &process);

    sysCall(MEMORY_LOAD_REQUEST, process);
    sysCall(PROCESS_INTERRUPT, NONE);
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

int findnum_lines(FILE* fp){

    int num_lines = 0;
    int line; // changed char to int
    int size;

    size = ftell(fp);
    line = getc(fp);
    if(size != 0){
        while(line != EOF){
            if(line == '\n'){
                num_lines = num_lines + 1;
            }
            line = getc(fp);
        }
    } else {
        num_lines = 0;
    }
    return num_lines;
}

void readSyntheticProgram(FILE *arquivo, PROCESSO **process) {
    long int code_section;
    int countCode;
    long tamanhoArquivo;

    (*process) = malloc(sizeof(PROCESSO));

    // Le cabecalho
    fscanf(arquivo, "%s %d %d %d\n", (*process)->nome, &(*process)->idSegmento, &(*process)->prioridade,
           &(*process)->tamanhoSegmento);

    (*process)->id = kernel->proxId++; // Pega o próximo id de processo e soma a variável
    (*process)->pc = 0;
    (*process)->estado = NOVO;
    (*process)->tempoMaximo = (QUANTUM_TIME / (*process)->prioridade);
    (*process)->quantidadeSemaforos = 0;
    while (1) {
        char semaforo;
        fscanf(arquivo, "%c", &semaforo);
        if (semaforo == '\n')
            break;

        if ((*process)->quantidadeSemaforos == MAX_SEMAFOROS) {
            erro("Numero maximo de semaforos alcancado.");
            return;
        }
        if (semaforo == ' ')
            continue;

        if (existeSemaforoProcesso(semaforo, *process)) {
            char *mensagem = malloc(21);
            sprintf(mensagem, "Semaforo %c ja existe", semaforo);
            alerta(mensagem);
            continue;
        }

        (*process)->semaforos[(*process)->quantidadeSemaforos++] = semaforo;
        novoSemaforo(semaforo);
    }
    fgetc(arquivo);
    code_section = ftell(arquivo);
    countCode = findnum_lines(arquivo) + 1;

    fseek(arquivo, code_section, SEEK_SET);


    INSTRUCAO *code = (INSTRUCAO *) malloc(sizeof(INSTRUCAO) * (countCode));

    if (!code) {
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
                return; // TODO: mata processo
            }
            code[i].op = comando[0] == 'P' ? SEM_P : SEM_V;
            code[i].sem = comando[2];
        } else {
            char *left_op = malloc(sizeof(char) * 6);
            int right_op;

            sscanf(comando, "%s %d", left_op, &right_op);

            if (strcmp(left_op, "exec") == 0)
                code[i].op = EXEC;
            else if (strcmp(left_op, "read") == 0)
                code[i].op = READ;
            else if (strcmp(left_op, "write") == 0)
                code[i].op = WRITE;
            else if (strcmp(left_op, "print") == 0)
                code[i].op = PRINT;
            else {
                erro("Operacao invalida");
                return; // TODO: matar processo
            }
            code[i].value = right_op;
            code[i].sem = '#';
        }
        i++;
    }

    (*process)->codigo = code;
    fclose(arquivo);
}

void processFinish(PROCESSO *processo) {
    if (processo) {
        PROCESSO *busca = kernel->pcb.head;
        while (busca->id != processo->id)
            busca = busca->prox;
        if (kernel->scheduler.scheduled->processo->id == processo->id)
            sysCall(PROCESS_INTERRUPT, NONE);

        freeSegmento(processo->idSegmento);
        removeScheduler(processo);

        free(processo->codigo);
        free(processo);
    }
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
    return NULL;
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

    segmento->id = processo->idSegmento;

    segmento->paginaQuant = (int) ceil((double) ((processo->tamanhoSegmento) / (TAMANHO_PAGINA)));
    segmento->segundaChance = 1;

    const int restante = tabelaSegmentos->memoriaRestante - processo->tamanhoSegmento;

    if (restante < 0)
        trocaPaginas(segmento, restante);
    else {
        tabelaSegmentos->memoriaRestante = restante;
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

void freeSegmento(int idSegmento) {
    TABELA_SEGMENTO tabelaSegmento = kernel->seg_table;
    tabelaSegmento.quantSegmentos--;

    int qtd = tabelaSegmento.quantSegmentos, indice = buscaSegmento(idSegmento);

    SEGMENTO segmento = tabelaSegmento.segmentos[indice];
    tabelaSegmento.memoriaRestante = MIN(TAMANHO_MAX_MEMORIA, tabelaSegmento.memoriaRestante + segmento.paginaQuantMemoria);

    for (; indice < qtd-1; indice++)
        tabelaSegmento.segmentos[indice] = tabelaSegmento.segmentos[indice+1];

    // Realoca a tabela de segmentos para quantidade - 1
    tabelaSegmento.segmentos = (SEGMENTO *) realloc(tabelaSegmento.segmentos, sizeof(SEGMENTO) * (qtd-1));
    if (!tabelaSegmento.segmentos) {
        erro("Sem memoria");
        exit(EXIT_FAILURE);
    }
}

int buscaSegmento(int idSegmento) {
    TABELA_SEGMENTO tabelaSegmento = kernel->seg_table;

    int qtd = tabelaSegmento.quantSegmentos, indice;
    for (indice = 0; indice < qtd && tabelaSegmento.segmentos[indice].id != idSegmento; indice++);

    return indice;
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES SCHEDULER -------------------------------------
SCHEDULER iniciaScheduler() {
    SCHEDULER scheduler;
    scheduler.scheduled = NULL;
    scheduler.head = NULL;
    scheduler.tail = NULL;
    scheduler.bloqueados = NULL;

    return scheduler;
}

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

SCHEDULER schedule_process(int flag) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESSO_SCHEDULER *atual = scheduler.scheduled, *novo = NULL;

    PROCESSO_SCHEDULER *aux = atual;
    while (aux && aux->prox->processo->estado == BLOQUEADO && aux->prox != atual)
        aux = aux->prox;

    if (aux && aux->prox != atual) {
        novo = aux->prox;
        novo->processo->tempoRestante = novo->processo->tempoMaximo;
    }

    if (atual) {
        if (flag == IO_REQUESTED || flag == SEMAPHORE_BLOCKED) {
            atual->processo->estado = BLOQUEADO;
        } else if (flag == QUANTUM_COMPLETED) {
            atual->processo->estado = PRONTO;
        }
    }
    else {
        novo = scheduler.head;
        novo->processo->tempoRestante = novo->processo->tempoMaximo;
    }

    if (novo)
        novo->processo->estado = EXECUTANDO;

    scheduler.scheduled = novo;
    return scheduler;
}

void removeScheduler(PROCESSO *processo) {
    SCHEDULER scheduler = kernel->scheduler;
    PROCESSO_SCHEDULER *removido = scheduler.head, *prev = scheduler.tail;
    while (removido->processo->id != processo->id) {
        prev = prev->prox;
        removido = removido->prox;
    }
    prev->prox = removido->prox;
    free(removido);
}
// ---------------------------------------------------------------------------------------------

// ------------------------------------- FUNÇÕES LOG -------------------------------------------
void printaProcessos() {
    PCB pcb = kernel->pcb;
    PROCESSO *processo = pcb.head;
    if (processo) {
        printf("┌─────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│%-5s │ %-50s │ %-10s │ %-10s│\n", "ID", "Nome", "Estado", "Prioridade");
        printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
        printaProcesso(processo->id, processo->nome, processo->estado, processo->prioridade);
        while (processo->prox != pcb.head) {
            processo = processo->prox;
            printaProcesso(processo->id, processo->nome, processo->estado, processo->prioridade);
        }
    }
    printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
    while (!getchar())
        ;
}

void printaMemoria() {
    TABELA_SEGMENTO table = kernel->seg_table;

    if (table.quantSegmentos > 0){
        printf("┌───────────────────────────┐\n");
        printf("│%-27s│\n", "SEGMENTOS");
        printf("└───────────────────────────┘\n");
        printf("│%-5s │ %-10s │\n", "ID", "Quantidade Páginas");
        for (int i = 0; i < table.quantSegmentos; i++) {
            printaSegmento(table.segmentos[i].id, table.segmentos[i].paginaQuantMemoria);
        }

        printf("PRESSIONE ENTER PARA PROSSEGUIR\n");
        while (!getchar())
            ;
    }
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
            pthread_mutex_lock(&criacao);
            kernel->scheduler = schedule_process((int) arg);
            break;
        }
        case PROCESS_CREATE: {
            processCreate((char *) arg);
            break;
        }
        case PROCESS_FINISH: {
            processFinish((PROCESSO *) arg);
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
        default: { // delete(System32);
            break;
        }
    }
}

static void sleep() { sysCall(PROCESS_INTERRUPT, (void*)SEMAPHORE_BLOCKED); }

static void wakeup(PROCESSO *processo) {
    processo->estado = PRONTO;
}

void interruptControl(char function, void *arg) {
    switch (function) {
        case MEMORY_LOAD_FINISH: {
            PROCESSO *processo = (PROCESSO *) arg;

            kernel->pcb = add_process(processo); // Adiciona o processo na PCB
            processo->estado = PRONTO;

            kernel->scheduler = add_process_scheduler(processo);
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

            kernel->scheduler = schedule_process(NONE);
        }
        else {
            no_process = 0;
            do {
                clock_gettime(CLOCK_REALTIME, &end);
                const int elapsed = (end.tv_sec - start.tv_sec) * ONE_SECOND_NS
                                    + (end.tv_nsec - start.tv_nsec);

                if (elapsed >= ONE_SECOND_NS) {
                    start = end;

                    // kernel->seg_table.segmentos[buscaSegmento(kernel->scheduler.scheduled->processo->idSegmento)].used = 1;

                    avalia(kernel->scheduler.scheduled->processo);
                }
            } while (kernel->scheduler.scheduled != NULL
                     && kernel->scheduler.scheduled->processo->tempoRestante > 0
                     && kernel->scheduler.scheduled->processo->pc
                            < kernel->scheduler.scheduled->processo->numComandos);

            if (kernel->scheduler.scheduled == NULL)
                continue;

            if (kernel->scheduler.scheduled->processo->pc
                > kernel->scheduler.scheduled->processo->numComandos)
                sysCall(PROCESS_FINISH, kernel->scheduler.scheduled); // TODO: :3
            else
                sysCall(PROCESS_INTERRUPT, (void*)QUANTUM_COMPLETED);
        }
        pthread_mutex_unlock(&criacao);
    }
}

// ---------------------------------------------------------------------------------------------

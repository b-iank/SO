#include "processo.h"


void processCreate(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    PROCESSO *process = NULL;
    INSTRUCAO *code = NULL;

    if (!(fp = fopen(fileName, "r"))) {
        exit(EXIT_FAILURE);
    }

    readSyntheticProgram(fp, &process, &code);

    memoryRequest *memReq = malloc(sizeof(memoryRequest));
    memReq->process = process;
    memReq->code = code;
    mem_req_init(&memReq, process, code);

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

        int existe = 0;
        for (int i = 0; i < (*process)->quantidadeSemaforos; i++) {
            if ((*process)->semaforos[i] == semaforo) {
                existe = 1;
            }
        }

        if (existe) {
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

    (*code) = (INSTRUCAO*) malloc(sizeof(INSTRUCAO) * (countCode));

    if (!(*code)) {
        alerta("Sem memoria");
        exit(0);
    }

    (*process)->numComandos = countCode;

    char comando[51];
    int i = 0;
    while (fgets(comando, 51, arquivo) != NULL) {
        if (comando[0] == 'P' || comando[0] == 'V') {
            // TODO: ler semáforo
            printf("aaa");
        } else { // exec 1000
            char *left_op = malloc(sizeof(char) * 6);
            int right_op;

            sscanf(comando, "%s %d", left_op, &right_op);
            // TODO: validar espaço
            if (strcmp(left_op, "exec") == 0)
                (*code)[i].op = EXEC;
            else if (strcmp(left_op, "read") == 0)
                (*code)[i].op = READ;
            else if (strcmp(left_op, "write") == 0)
                (*code)[i].op = WRITE;
            else if (strcmp(left_op, "print") == 0)
                (*code)[i].op = PRINT;

            (*code)[i].value = right_op;
            (*code)[i].sem = NULL;
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"
#include "memory.h"

void processCreate(char* fileName) {
    FILE *fp = fopen(fileName, "r");
    process_t* process = NULL;
    instruction *code = NULL;

    if (!(fp = fopen(fileName, "r"))) {
        exit(EXIT_FAILURE);
    }

    readSyntacticProgram(fp, &process, &code);

    memoryRequest *memReq = malloc(sizeof(memoryRequest));
    memReq->process = process;
    memReq->code = code;
    mem_req_init(&memReq, process, code);

    sysCall(MEM_LOAD_REQ, (void *) &memReq);
}


pcb * add_process(pcb *TCP, process_t *processo) {
    pcb *lista_aux = TCP;
    process_t *head = lista_aux->head, *aux = head, *anterior = lista_aux->tail;

    if (head == NULL) {
        lista_aux->head = processo;
        lista_aux->tail = processo;
        processo->next = processo;
        return lista_aux;
    }

    while (processo->priority > aux->priority && aux->next != head) {
        anterior = aux;
        aux = aux->next;
    }

    processo->next = aux;
    anterior->next = processo;

    if (processo->priority < head->priority)
        lista_aux->head = processo;
    if (processo->next == lista_aux->head)
        lista_aux->tail = processo;

    return lista_aux;
}

void readSyntacticProgram(FILE *arquivo, process_t **process, instruction **code)
{
    long int code_section;
    int i, countCode;
    long tamanhoArquivo;

    (*process) = malloc(sizeof(process_t));

    fscanf(arquivo, "%s %d %d %d", (*process)->name, &(*process)->idSegmento, &(*process)->priority, &(*process)->tamanhoSegmento);

    (*process)->numSemaforos = 0;
    while (1)
    {
        int semaforo;
        if (fscanf(arquivo, "%d", &semaforo) != 1)
            break;
        (*process)->semaforos[(*process)->numSemaforos++] = semaforo;
    }

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

    (*code) = (instruction *)malloc(sizeof(instruction) * (countCode));

    if (!code) {
        printf("Sem memória!\n");
        exit(0);
    }

    (*process)->numComandos = countCode;

    char comando[51];
    i = 0;
    while (fgets(comando, 51, arquivo) != NULL)
    {
        if (comando[0] == 'P' || comando[0] == 'V') // TODO: ler semáforo
            printf("aaa");
        else {
            char* dupline = strdup(comando);
            char* left_op = strtok(dupline, " ");
            int right_op = atoi(strtok(NULL, " "));

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

process_t *processCreate(int id, const char *name)
{
    process_t *new_process = (process_t *)malloc(sizeof(process_t));
    if (new_process == NULL)
    {
        printf("Erro\n");
        return NULL;
    }

    new_process->id = id;
    strncpy(new_process->name, name, MAX_PROCESS_NAME - 1);
    new_process->name[MAX_PROCESS_NAME - 1] = '\0';

    return new_process;
}

void processInterrupt(pcb *BCP)
{
    process_t *current = BCP->current;
    printf("Interrupcao do processo %d\n", BCP->current->id);
    // Realizar as ações necessárias para tratar a interrupção
    // troca de contexto etc
    if (current->remainingTime <= 0) {
        processFinish(BCP);
        return;
    }

}

void processFinish(pcb *BCP)
{
    printf("Finalizando o processo %d\n", BCP->current->id);
}
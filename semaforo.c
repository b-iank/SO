#include "semaforo.h"

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
        exit(0);
    }

    semaforo->nome = nome;
    semaforo->S = 1; //TODO ?
    semaforo->aguardando = NULL;
    sem_init(&semaforo->mutex, 0, 1);

    if (!adicionaTabelaSemaforo(semaforo)) {
        char *mensagem = malloc(255);
        sprintf(mensagem, "Nao foi possivel adicionar o semaforo %c", nome);
        erro(mensagem);
        exit(0);
    }
}

int adicionaTabelaSemaforo(SEMAFORO *semaforo) {
    if (kernel->tabelaSemaforo.quantidadeSemaforos == MAX_SEMAFOROS)
        return 0;

    kernel->tabelaSemaforo.semaforo[++kernel->tabelaSemaforo.quantidadeSemaforos] = semaforo;
    return 1;
}

SEMAFORO *buscaSemaforo(char semaforo) {
    TABELA_SEMAFORO tabela = kernel->tabelaSemaforo;
    for (int i = 0; i < MAX_SEMAFOROS; i++) {
        if (tabela.semaforo[i].nome == semaforo)
            return &tabela.semaforo[i];
    }

    return NULL;
}
#include "terminal.h"

// RESETA COR DO TERMINAL
#define LIMPA "\x1B[0m"

//ALERTAS
#define ERRO "\x1b[38;5;1m"
#define SUCESSO "\x1b[38;5;10m"
#define ALERTA "\x1b[38;5;11m"

//ESTILOS
#define NEGRITO "\x1B[1m"
#define ITALICO "\x1B[3m"
#define SUBLINHADO "\x1B[4m"

//FILME
#define CIANO "\x1b[36m"
#define LARANJA "\x1b[38;5;214m"
#define ROXO "\x1b[38;5;126m"

#define CLEAR system("clear");

int main_menu() {
    char fileName[31];

    int op = -1, ret;

    CLEAR
    printf(LARANJA NEGRITO SUBLINHADO"\n------------ Zz SISTEMA OPERACIONAL zZ ------------" LIMPA);
    do {
        printf(ROXO NEGRITO SUBLINHADO"\n\tMENU\n\n" LIMPA);
        printf("1 - Executar processo\n");
        printf("2 - Ver processos\n");
        printf("3 - Ver memoria\n");
        printf("0 - Sair\n");
        ret = scanf("%d", &op);

        if (ret != 1 || op < 0 || op > 3) {
            printf(ERRO NEGRITO "OPCAO INVALIDA!\n" LIMPA);
            while (getchar() != '\n'); //Limpa o buffer do teclado para evitar comportamentos inesperados
        }
    } while (ret != 1 || op < 0 || op > 3);

    return op;
}


void printaProcesso(int id, char nome[50], char estado, int prioridade) {
    char estadoStr[11];
    if (estado == '0')
        strcpy(estadoStr, "NOVO");
    else if (estado == '1')
        strcpy(estadoStr, "BLOQUEADO");
    else if (estado == '2')
        strcpy(estadoStr, "PRONTO");
    else if (estado == '3')
        strcpy(estadoStr, "EXECUTANDO");
    else if (estado == '4')
        strcpy(estadoStr, "CONCLUIDO");
    printf("│%04d │ %-50s │ %-10s │ %02d│\n", id, nome, estadoStr, prioridade);
    printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
}

void printaSegmento(int id, int quantidade) {
    printf("│%05d │ %05d      │\n", id, quantidade);
    printf("└───────────────────────────┘\n");
}

void sucesso(char *mensagem) { printf(SUCESSO "%s\n" LIMPA, mensagem); }

void alerta(char *mensagem) { printf(ALERTA "%s\n" LIMPA, mensagem); }

void erro(char *mensagem) { printf(ERRO NEGRITO"%s\n" LIMPA, mensagem); }
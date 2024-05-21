#include "terminal.h"

int main_menu() {
    char fileName[31];

    int op;
    do {
        printf("Menu\n");
        printf("1 - Executar processo\n");
        printf("2 - Ver processos\n");
        printf("3 - Ver memoria\n");
        printf("0 - Sair\n");
        scanf("%d", &op);

        if (op <= 3 && op >= 0)
            return op;
        erro("Operação não suportada");
    } while (op > 3 || op < 0);
}

void sucesso(char *mensagem) { printf("%s\n", mensagem); }

void alerta(char *mensagem) { printf("%s\n", mensagem); }

void erro(char *mensagem) { printf("%s\n", mensagem); }

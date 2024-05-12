#include "terminal.h"

void main_menu() {
    char fileName[31];

    int op;
    printf("Menu\n");
    printf("1 - Executar PROCESSO\n");
    printf("2 - Ver processos\n");
    printf("3 - Ver memória\n");
    scanf("%d", &op);

    if (op == 1) {
        scanf("%s", fileName);
        processInterrupt(&kernel->pcb);
        sysCall(PROCESS_CREATE, fileName);
    } else if (op == 2) {
        // mostrar
    } else if (op == 3) {
        // mostrar
    } else {
        erro("Operação não suportada");
    }
}

void sucesso(char *mensagem) {
    printf("%s\n", mensagem);
}

void alerta(char *mensagem) {
    printf("%s\n", mensagem);
}

void erro(char *mensagem) {
    printf("%s\n", mensagem);
}

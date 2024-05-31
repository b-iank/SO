#include "terminal.h"

int main_menu() {
    int op = -1, ret;

    CLEAR
    printf(ORANGE BOLD UNDERLINE "\n------------ Zz SISTEMA OPERACIONAL zZ ------------\n" LIMPA);
    do {
        printf("1 - Adicionar processo\n");
        printf("2 - Ver processos em execucao\n");
        printf("3 - Ver estado dos processos\n");
        printf("4 - Ver memoria\n");
        printf("0 - Sair\n");
        printf("\nOpcao: ");
        ret = scanf("%d", &op);

        if (ret != 1 || op < 0 || op > 4) {
            printf(ERROR BOLD "OPCAO INVALIDA!\n" LIMPA);
            while (getchar() != '\n')
                ; // Limpa o buffer do teclado para evitar comportamentos inesperados
        }
    } while (ret != 1 || op < 0 || op > 4);

    return op;
}

void print_name() {
    printf(ORANGE BOLD"\n\n__________.__                           .__ \n");
    printf("\\______   \\__|____    __________________|__|\n");
    printf(" |    |  _/  \\__  \\  /    \\_  __ \\_  __ \\  |\n");
    printf(" |    |   \\  |/ __ \\|   |  \\  | \\/|  | \\/  |\n");
    printf(" |______  /__(____  /___|  /__|   |__|  |__|\n");
    printf("        \\/        \\/     \\/                 \n\n" LIMPA);


}


void print_process(int id, char name[50], char state, int priority, int arrival) {
    char state_string[11];
    if (state == '0')
        strcpy(state_string, "NOVO");
    else if (state == '1')
        strcpy(state_string, "BLOQUEADO");
    else if (state == '2')
        strcpy(state_string, "PRONTO");
    else if (state == '3')
        strcpy(state_string, "EXECUTANDO");
    else if (state == '4')
        strcpy(state_string, "DONE");
    printf("│ %04d │ %-50s │ %-10s │ %02d │ %016d │\n", id, name, state_string, priority, arrival);
    printf("└────────────────────────────────────────────────────────────────────────────────────────────────────┘\n");
}

void print_segment(int id, int pages) {
    printf("│ %05d │ %05d      │\n", id, pages);
    printf("└───────────────────────────┘\n");
}

void so_sucess(char *message) { printf(SUCESS "%s\n" LIMPA, message); }

void so_alert(char *message) { printf(ALERT "%s\n" LIMPA, message); }

void so_error(char *message) { printf(ERROR BOLD "%s\n" LIMPA, message); }

#include "terminal.h"

// RESETA COR DO TERMINAL
#define LIMPA "\x1B[0m"

// ALERTAS
#define ERROR "\x1b[38;5;1m"
#define SUCESS "\x1b[38;5;10m"
#define ALERT "\x1b[38;5;11m"

// ESTILOS
#define BOLD "\x1B[1m"
#define ITALIC "\x1B[3m"
#define UNDERLINE "\x1B[4m"
#define ORANGE "\x1b[38;5;214m"

#define CLEAR system("clear");

int main_menu() {
    int op = -1, ret;

    CLEAR
    printf(ORANGE BOLD UNDERLINE "\n------------ Zz SISTEMA OPERACIONAL zZ ------------\n" LIMPA);
    do {
        printf("1 - Adicionar processo\n");
        printf("2 - Ver processos\n");
        printf("3 - Ver memoria\n");
        printf("0 - Sair\n");
        ret = scanf("%d", &op);

        if (ret != 1 || op < 0 || op > 3) {
            printf(ERROR BOLD "OPCAO INVALIDA!\n" LIMPA);
            while (getchar() != '\n')
                ; // Limpa o buffer do teclado para evitar comportamentos inesperados
        }
    } while (ret != 1 || op < 0 || op > 3);

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


void print_process(int id, char name[50], char state, int priority) {
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
    printf("│ %04d │ %-50s │ %-10s │ %02d│\n", id, name, state_string, priority);
    printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
}

void print_segment(int id, int pages) {
    printf("│ %05d │ %05d      │\n", id, pages);
    printf("└───────────────────────────┘\n");
}

void so_sucess(char *message) { printf(SUCESS "%s\n" LIMPA, message); }

void so_alert(char *message) { printf(ALERT "%s\n" LIMPA, message); }

void so_error(char *message) { printf(ERROR BOLD "%s\n" LIMPA, message); }

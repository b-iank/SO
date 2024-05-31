#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

int main_menu();
void print_name();
void print_process(int id, char name[50], char state, int priority, int arrival);
void print_segment(int id, int pages);
void so_define(int status, char *message);
void so_sucess(char *message);
void so_alert(char *message);
void so_error(char *message);

#endif

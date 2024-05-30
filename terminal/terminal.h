#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main_menu();
void print_name();
void print_process(int id, char name[50], char state, int priority);
void print_segment(int id, int pages);
void so_sucess(char *message);
void so_alert(char *message);
void so_error(char *message);

#endif

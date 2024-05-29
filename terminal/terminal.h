#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main_menu();
void printaNome();
void printaProcesso(int id, char nome[50], char estado, int prioridade);
void sucesso(char *mensagem);
void alerta(char *mensagem);
void erro(char *mensagem);

#endif

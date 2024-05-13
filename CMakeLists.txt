cmake_minimum_required(VERSION 3.25)
project(SO C)

include_directories(.)
set(CMAKE_C_STANDARD 11)

add_executable(SO main.c
        processo/processo.c
        terminal/terminal.c
        ribabari/log.c
        instrucao/instrucao.c
        kernel/kernel.c
        semaforo/semaforo.c
        memoria/memoria.c)

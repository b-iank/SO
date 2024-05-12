cmake_minimum_required(VERSION 3.25)
project(SO C)

include_directories(.)
set(CMAKE_C_STANDARD 11)

add_executable(SO main.c
        processo.c
        terminal.c
        log.c
        instruction.c
        kernel.c
        semaforo.c)

cmake_minimum_required(VERSION 3.28)
project(SO C)

set(CMAKE_C_STANDARD 11)

add_executable(SO main.c
        process.c
        a/memory.h
        a/escalonamento.h
        a/semaphore.h
        a/escalonamento.c
        terminal.c
        log.h
        log.c
        list.h
        instruction.c
        instruction.h
        memory.h)

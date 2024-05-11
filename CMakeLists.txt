cmake_minimum_required(VERSION 3.28)
project(SO C)

set(CMAKE_C_STANDARD 11)

add_executable(SO main.c
        process.c
        terminal.c
        log.h
        log.c
        list.h
        instruction.c
        instruction.h
        memory.h
        kernel.h
        kernel.c
        semaphore.c
        semaphore.h)

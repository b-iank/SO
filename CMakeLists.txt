cmake_minimum_required(VERSION 3.28)
project(SO C)

set(CMAKE_C_STANDARD 11)

add_executable(SO main.c
        process.c
        terminal.c
        log.c
        instruction.c
        kernel.c
        semaphore.c)

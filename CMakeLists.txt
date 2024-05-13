cmake_minimum_required(VERSION 3.25)
project(SO C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(kernel memoria processo ribabari semaforo terminal)

add_executable(SO main.c
        processo/processo.c
        terminal/terminal.c
        ribabari/log.c
        kernel/kernel.c
        semaforo/semaforo.c
        memoria/memoria.c)


set(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(SO PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIRECTORY}")


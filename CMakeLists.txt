cmake_minimum_required(VERSION 3.25)
project(SO C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(terminal so)

add_executable(SO main.c so/so.c terminal/terminal.c)


set(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(SO PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIRECTORY}")


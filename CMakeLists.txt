cmake_minimum_required(VERSION 3.29)
project(redit C)

set(CMAKE_C_STANDARD 23)

add_executable(redit
        redit.c
        file_utils.c
        file_utils.h
        file_operations.h
        file_operations.c
        main.c)

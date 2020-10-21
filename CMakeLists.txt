cmake_minimum_required(VERSION 3.17)
project(k23a_project C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(k23a_project
        main.c
        lists.c
        lists.h
        hash.c
        hash.h
        )

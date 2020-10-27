cmake_minimum_required(VERSION 3.17)
project(k23a_project C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(k23a_project
        src/main.c
        src/lists.c
        include/lists.h
        src/hash.c
        include/hash.h
        )

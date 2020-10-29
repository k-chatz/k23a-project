cmake_minimum_required(VERSION 3.17)
project(k23a_project C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(k23a_project
        src/main.c
        src/lists.c
        src/hash.c
        src/json_parser.c
        src/spec_ids.c
        src/spec_to_specs.c
        src/read_csv.c
        include/lists.h
        include/hash.h
        include/json_parser.h
        include/read_csv.h
        include/spec_ids.h
        include/spec_to_specs.h
        )

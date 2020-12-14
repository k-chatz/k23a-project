cmake_minimum_required(VERSION 3.17)
project(k23a_project C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

#json_parser_tests
#add_executable(k23a_project
#        tests/json_parser_tests.c
#        src/json_parser.c
#        src/lists.c
#        src/hash.c
#        src/spec_ids.c
#        src/spec_to_specs.c
#        include/lists.h
#        include/hash.h
#        include/json_parser.h
#        )

#lists_tests
#add_executable(k23a_project
#        tests/lists_tests.c
#        src/lists.c
#        )

#hash tests
#add_executable(k23a_project
#        tests/hash_tests.c
#        src/lists.c
#        src/json_parser.c
#        src/hash.c
#        )

#spec_to_specs_tests
#add_executable(k23a_project
#        tests/spec_to_specs_tests.c
#        src/spec_to_specs.c
#        src/lists.c
#        src/hash.c
#        )

#ml tests
#add_executable(k23a_project
#        src/lists.c
#        src/hash.c
#        src/ml.c
#        tests/ml_tests.c
#        )

#main
add_executable(k23a_project
        src/main.c
        src/lists.c
        src/hash.c
        src/json_parser.c
        src/spec_ids.c
        src/spec_to_specs.c
        src/ml.c
        src/tokenizer.c
        )

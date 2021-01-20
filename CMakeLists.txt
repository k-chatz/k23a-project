cmake_minimum_required(VERSION 3.17)
project(k23a_project C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
find_package (Threads REQUIRED)

include_directories(.)

#general_test
add_executable(k23a_project_general_tests
        tests/general_tests.c
        )

#json_parser_tests
add_executable(k23a_project_json_parser_tests
        tests/json_parser_tests.c
        src/tokenizer.c
        src/json_parser.c
        src/lists.c
        src/hash.c
        src/spec_to_specs.c
        include/lists.h
        include/hash.h
        include/json_parser.h
        )

#lists_tests
add_executable(k23a_project_lists_tests
        tests/lists_tests.c
        src/lists.c
        )

#hash tests
add_executable(k23a_project_hash_tests
        tests/hash_tests.c
        src/lists.c
        src/tokenizer.c
        src/json_parser.c
        src/hash.c
        )

#hset tests
add_executable(k23a_project_hset_tests
        tests/hset_tests.c
        src/hash.c
        src/hset.c
        src/lists.c
        )

#spec to specs tests
add_executable(k23a_project_spec_to_specs_tests
        tests/spec_to_specs_tests.c
        src/spec_to_specs.c
        src/lists.c
        src/hash.c
        )

#ml tests
add_executable(k23a_project_ml_tests
        src/lists.c
        src/hash.c
        src/ml.c
        src/json_parser.c
        src/tokenizer.c
        tests/ml_tests.c
        )

#queue tests
add_executable(k23a_project_queue_tests
        src/queue.c
        tests/queue_tests.c
        )

#job scheduler tests
add_executable(k23a_project_job_scheduler_tests
        src/job_scheduler.c
        src/queue.c
        src/semaphore.c
        tests/job_scheduler_tests.c
        )

#logreg tests
add_executable(k23a_project_logreg_tests
        src/logreg.c
        tests/logreg_tests.c
        )

#tokenizer tests
add_executable(k23a_project_tokenizer_tests
        tests/tokenizer_tests.c
        src/json_parser.c
        src/hash.c
        src/hset.c
        src/lists.c
        src/tokenizer.c
        )

#main
add_executable(k23a_project
        src/main.c
        src/lists.c
        src/queue.c
        src/hash.c
        src/hset.c
        src/unique_rand.c
        src/json_parser.c
        src/spec_to_specs.c
        src/ml.c
        src/tokenizer.c
        src/logreg.c
        src/job_scheduler.c
        src/queue.c
        src/semaphore.c
        )

#user
add_executable(k23a_project_user
        src/user.c
        src/spec_to_specs.c
        src/hash.c
        src/json_parser.c
        src/lists.c
        src/ml.c
        src/tokenizer.c
        src/logreg.c
        src/unique_rand.c
        src/job_scheduler.c
        src/queue.c
        src/semaphore.c
        )

target_link_libraries(k23a_project_general_tests m)
target_link_libraries(k23a_project_json_parser_tests m)
target_link_libraries(k23a_project_lists_tests m)
target_link_libraries(k23a_project_hash_tests m)
target_link_libraries(k23a_project_hset_tests m)
target_link_libraries(k23a_project_spec_to_specs_tests m)
target_link_libraries(k23a_project_ml_tests m)
target_link_libraries(k23a_project_job_scheduler_tests m)
target_link_libraries(k23a_project_queue_tests m)
target_link_libraries(k23a_project_logreg_tests m)
target_link_libraries(k23a_project_tokenizer_tests m)
target_link_libraries(k23a_project_user m)
target_link_libraries(k23a_project m)

target_link_libraries(k23a_project_general_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_json_parser_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_lists_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_hash_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_hset_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_spec_to_specs_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_ml_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_queue_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_job_scheduler_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_logreg_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_tokenizer_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(k23a_project_user ${CMAKE_THREAD_LIBS_INIT})

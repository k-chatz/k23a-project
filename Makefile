vpath	 %.c     src
vpath	 %.c     tests
vpath	 %.h     include
vpath	 %.o     objs
vpath	 %_tests tests-bin

CC	= gcc
CFLAGS	= -g3 -Wall
LFLAGS	=

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o objs/$@

.PHONY: tests all clean githooks docs phony

all: tests part1

##################################################
#                                                #
#                                                #
#             RULES FOR EXECUTABLES              #
#                                                #
#                                                #
##################################################

part1: main.o lists.o spec_to_specs.o spec_ids.o hash.o json_parser.o


##################################################
#                                                #
#                                                #
#                RULES FOR TESTS                 #
#                                                #
#                                                #
##################################################

tests: phony hash_tests spec_to_specs_tests lists_tests json_parser_tests
	for test in tests-bin/*; do if [ -x $$test ]; then ./$$test || exit 1; fi done

tests-bin/hash_tests: hash_tests.o hash.o
tests-bin/spec_to_specs_tests: spec_to_specs_tests.o spec_to_specs.o lists.o hash.o
tests-bin/lists_tests: lists_tests.o lists.o
tests-bin/json_parser_tests: json_parser_tests.o json_parser.o spec_ids.o spec_to_specs.o hash.o lists.o


##################################################
#                                                #
#                                                #
#                  OTHER RULES                   #
#                                                #
#                                                #
##################################################


githooks:
	git config --local core.hooksPath ".githooks/"

docs:
	doxygen Doxyfile

clean:
	-rm -rf deps $(OUT)
	-rm -f tests-bin/*
	-rm -f objs/*.o
	-rm -f src/*~
	-rm -f tests/*~

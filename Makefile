vpath	 %.c       src
vpath	 %_tests.c tests
vpath	 %.h       include
vpath	 %_tests   tests-bin

CC	= gcc
CFLAGS	= -g3 -Wall
CFLAGS_OPT = -O3 -ftree-vectorize -msse2 -ftree-vectorizer-verbose=1 -ffast-math
LFLAGS	=


.PHONY: tests all clean githooks docs phony

all: tests part1

objs/%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@


##################################################
#                                                #
#                                                #
#             RULES FOR EXECUTABLES              #
#                                                #
#                                                #
##################################################

part1: $(addprefix objs/, main.o lists.o spec_to_specs.o spec_ids.o hash.o json_parser.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

##################################################
#                                                #
#                                                #
#                RULES FOR TESTS                 #
#                                                #
#                                                #
##################################################

tests: $(addprefix tests-bin/, hash_tests spec_to_specs_tests lists_tests json_parser_tests)
	for test in tests-bin/*; do if [ -x $$test ]; then ./$$test || exit 1; fi done

tests-bin/hash_tests: $(addprefix objs/, hash_tests.o hash.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/spec_to_specs_tests: $(addprefix objs/, spec_to_specs_tests.o spec_to_specs.o lists.o hash.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/lists_tests: $(addprefix objs/, lists_tests.o lists.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/json_parser_tests: $(addprefix objs/, json_parser_tests.o json_parser.o hash.o lists.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)


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
	-rm part1
	-rm -rf deps $(OUT)
	-rm -f tests-bin/*
	-rm -f objs/*.o
	-rm -f src/*~
	-rm -f tests/*~

vpath	 %.c       src
vpath	 %_tests.c tests
vpath	 %.h       include
vpath	 %_tests   tests-bin

CC	= gcc
CFLAGS	= -g3 -Wall -DMAKEFILE
LFLAGS	= -lm


.PHONY: tests all clean githooks docs phony

all: tests project user

objs/%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@


##################################################
#                                                #
#                                                #
#             RULES FOR EXECUTABLES              #
#                                                #
#                                                #
##################################################

project: $(addprefix objs/, main.o lists.o spec_to_specs.o hash.o tokenizer.o json_parser.o ml.o logreg.o unique_rand.o hset.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

user: $(addprefix objs/, user.o lists.o spec_to_specs.o hash.o tokenizer.o json_parser.o ml.o logreg.o unique_rand.o hset.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

##################################################
#                                                #
#                                                #
#                RULES FOR TESTS                 #
#                                                #
#                                                #
##################################################

tests: $(addprefix tests-bin/, hash_tests spec_to_specs_tests lists_tests json_parser_tests general_tests ml_tests logreg_tests tokenizer_tests hset_tests)
	for test in tests-bin/*; do if [ -x $$test ]; then printf "\n\nrunning $$test...\n"; ./$$test || exit 1; fi done

tests-bin/logreg_tests: $(addprefix objs/, logreg.o logreg_tests.o )
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/hash_tests: $(addprefix objs/, hash_tests.o hash.o json_parser.o lists.o tokenizer.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/spec_to_specs_tests: $(addprefix objs/, spec_to_specs_tests.o spec_to_specs.o lists.o hash.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/lists_tests: $(addprefix objs/, lists_tests.o lists.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/json_parser_tests: $(addprefix objs/, json_parser_tests.o json_parser.o hash.o lists.o tokenizer.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/general_tests: $(addprefix objs/, general_tests.o lists.o spec_to_specs.o hash.o tokenizer.o json_parser.o ml.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/ml_tests: $(addprefix objs/, ml_tests.o json_parser.o ml.o tokenizer.o hash.o lists.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/tokenizer_tests: $(addprefix objs/, tokenizer_tests.o tokenizer.o hset.o hash.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

tests-bin/hset_tests: $(addprefix objs/, hset_tests.o hset.o hash.o)
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
	-rm user
	-rm project
	-rm -rf deps $(OUT)
	-rm -f tests-bin/*
	-rm -f objs/*.o
	-rm -f src/*~
	-rm -f tests/*~

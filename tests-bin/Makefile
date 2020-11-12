CC	 = gcc
CFLAGS	 = -g -Wall
LFLAGS	 = 

CSRC := $(wildcard *.c)
DEPS := $(patsubst %.c, ../deps/%.d, $(CSRC))

SPEC_TO_SPECS_OBJS = $(patsubst %.o, ../objs/%.o,             \
					spec_to_specs_tests.o \
					spec_to_specs.o	      \
					lists.o		      \
					hash.o)		      

LISTS_OBJS = $(patsubst %.o, ../objs/%.o,     \
			lists_tests.o         \
			lists.o)

JSON_OBJS = $(patsubst %.o, ../objs/%.o,     \
			json_parser_tests.o  \
			json_parser.o	     \
			spec_ids.o	     \
			spec_to_specs.o		\
			hash.o		     \
			lists.o)

HASH_OBJS = $(patsubst %.o, ../objs/%.o,     \
			hash_tests.o         \
			hash.o)


.PHONY: phony

all:     phony		     \
	 hash_tests	     \
	 spec_to_specs_tests \
	 lists_tests	     \
	 json_parser_tests

hash_tests: $(HASH_OBJS)
	$(CC) $(CFLAGS) -ohash_tests $(HASH_OBJS) $(LFLAGS)

spec_to_specs_tests: $(SPEC_TO_SPECS_OBJS)
	$(CC) $(CFLAGS) -ospec_to_specs_tests $(SPEC_TO_SPECS_OBJS) $(LFLAGS)

lists_tests: $(LISTS_OBJS)
	$(CC) $(CFLAGS) -olists_tests $(LISTS_OBJS) $(LFLAGS)

json_parser_tests: $(JSON_OBJS)
	$(CC) $(CFLAGS) -ojson_parser_tests $(JSON_OBJS) $(LFLAGS)

-include $(DEPS)

../objs/%.o: phony
	$(MAKE) -C ../objs $*.o


clean: phony
	-rm json_parser_tests lists_tests

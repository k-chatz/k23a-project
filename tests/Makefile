CC	 = gcc
CFLAGS	 = -g -Wall -I../include/
LFLAGS	 = 

CSRC := $(wildcard *.c)
DEPS := $(patsubst %.c, ../deps/%.d, $(CSRC))

SPEC_TO_SPECS_OBJS = $(patsubst %.o, ../objs/%.o,             \
					spec_to_specs_tests.o \
					spec_to_specs.o	      \
					lists.o		      \
					hash.o)		      




all: spec_to_specs_tests

spec_to_specs_tests: $(SPEC_TO_SPECS_OBJS)
	$(CC) $(CFLAGS) -ospec_to_specs_tests $(SPEC_TO_SPECS_OBJS) $(LFLAGS)

-include $(DEPS)

../objs/%.o: ../src/%.c
	$(MAKE) -C ../objs $*.o

.PHONY: clean all
clean:
	rm spec_to_specs_tests

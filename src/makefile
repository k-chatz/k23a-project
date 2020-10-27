OBJS	= main.o lists.o
SOURCE	= main.c lists.c
HEADER	= lists.h
OUT	= project
CC	 = gcc
CFLAGS	 = -g -Wall -I../include/
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

tests: tests/spec_to_specs_tests
	tests/spec_to_specs_tests

tests/spec_to_specs_tests: objs/spec_to_specs_tests.o objs/spec_to_specs.o objs/lists.o objs/hash.o
	$(MAKE) -C objs spec_to_specs_tests
	mv objs/spec_to_specs_tests tests/

main.o: main.c
lists.o: lists.c

objs/%.o: src/%.c src/makefile
	$(MAKE) -C src $*.o
	mv src/$*.o $@

src/makefile: makefile
	cp makefile src/makefile

objs/makefile: makefile
	cp makefile objs/makefile


clean:
	rm -f *.o $(OUT) *_tests

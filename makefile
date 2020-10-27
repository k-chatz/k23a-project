OBJS	= main.o lists.o
SOURCE	= main.c lists.c
HEADER	= lists.h
OUT	= project
CC	 = gcc
CFLAGS	 = -g -Wall -I../include/
LFLAGS	 = 

.PHONY: tests all clean githooks

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)


tests:
	$(MAKE) -C tests
	for test in tests/*; do [ -x $$test ] && ./$$test; done

tests/spec_to_specs_tests: objs/spec_to_specs_tests.o objs/spec_to_specs.o objs/lists.o objs/hash.o
	$(MAKE) -C objs spec_to_specs_tests
	mv objs/spec_to_specs_tests tests/

githooks:
	git config --local core.hooksPath ".githooks/"

clean:
	rm -rf deps
	$(MAKE) -C objs clean
	$(MAKE) -C tests clean

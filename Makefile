OBJS	= main.o lists.o spec_to_specs.o spec_ids.o hash.o json_parser.o
SOURCE	= main.c lists.c spec_to_specs.c spec_ids.c	hash.c json_parser.c
HEADER	= lists.h spec_to_specs.h spec_ids.h hash.h json_parser.h
OUT		= project
CC		= gcc
CFLAGS	= -g -O3 -Wall

.PHONY: tests all clean githooks docs phony

all: tests $(foreach d, $(OBJS), objs/$d)
	$(MAKE) -C objs $(OBJS)
	$(CC) $(foreach d, $(OBJS), objs/$d) -o $(OUT)

tests:
	$(MAKE) -C tests-bin
	for test in tests-bin/*; do if [ -x $$test ]; then ./$$test || exit 1; fi done

objs/%.o: phony
	$(MAKE) -C objs $*.o

githooks:
	git config --local core.hooksPath ".githooks/"

docs:
	doxygen Doxyfile

clean:
	-rm -rf deps $(OUT)
	$(MAKE) -C objs clean
	$(MAKE) -C tests-bin clean
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

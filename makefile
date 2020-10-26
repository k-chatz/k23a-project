OBJS	= main.o lists.o
SOURCE	= main.c lists.c
HEADER	= lists.h
OUT	= project
CC	 = gcc
CFLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

tests: spec_to_specs_tests
	./spec_to_specs_tests

spec_to_specs_tests: spec_to_specs_tests.o spec_to_specs.o lists.o hash.o spec_to_specs.h

main.o: main.c
lists.o: lists.c lists.h

clean:
	rm -f *.o $(OUT) *_tests

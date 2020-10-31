OBJS	= main.o lists.o
SOURCE	= main.c lists.c
HEADER	= lists.h
OUT	= project
CC	 = gcc
CFLAGS	 = -g -Wall -I../
LFLAGS	 = 

.PHONY: tests all clean githooks

all: tests
	$(MAKE) -C objs $(OBJS)
	$(CC) $(foreach d, $(OBJS), objs/$d) -o $(OUT) $(LFLAGS)

tests:
	$(MAKE) -C tests-bin
	for test in tests-bin/*; do [ -x $$test ] && ./$$test; done

githooks:
	git config --local core.hooksPath ".githooks/"

clean:
	-rm -rf deps $(OUT)
	$(MAKE) -C objs clean
	$(MAKE) -C tests-bin clean
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

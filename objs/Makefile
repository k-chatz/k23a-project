CC	 = gcc
CFLAGS	 = -g -Wall -I../
LFLAGS	 = 

CSRC := $(wildcard ../src/*.c)
DEPS := $(patsubst %.c, ../deps/%.d, $(CSRC))


-include $(DEPS)

%_tests.o: ../tests/%_tests.c
	$(MAKE) -C ../tests/ ../objs/$@


%.o: ../src/%.c
	$(MAKE) -C ../src/ ../objs/$@

.PHONY: clean
clean:
	rm -f *.o

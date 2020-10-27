CC	 = gcc
CFLAGS	 = -g -Wall -I../include/
LFLAGS	 = 

CSRC := $(wildcard ../src/*.c)
DEPS := $(patsubst %.c, ../deps/%.d, $(CSRC))


-include $(DEPS)

%.o: ../src/%.c
	$(MAKE) -C ../src/ ../objs/$@

.PHONY: clean
clean:
	rm *.o

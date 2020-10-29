CC	 = gcc
CFLAGS	 = -g -Wall -I../
LFLAGS	 = 

CSRC := $(wildcard *.c)
DEPS := $(patsubst %.c, ../deps/%.d, $(CSRC))



../deps/%_tests.d: ../tests/%_tests.c
	mkdir -p ../deps
	cpp $(CFLAGS) $< -MM -MT../objs/$*.o >$@

../deps/%.d: ../src/%.c
	mkdir -p ../deps
	cpp $(CFLAGS) $< -MM -MT../objs/$*.o >$@

-include $(DEPS)

../objs/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *~

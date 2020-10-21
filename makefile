OBJS	= main.o list.o
SOURCE	= main.c list.c
HEADER	= list.h
OUT	= project
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c 

list.o: list.c
	$(CC) $(FLAGS) list.c 


clean:
	rm -f $(OBJS) $(OUT)

CC=gcc
CFLAGS=-Wall
OBJS=packet.o main.o
OUTPUT=analyzer
LFLAGS=-lncurses -lpanel

all: analyzer

analyzer: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(OUTPUT)

clean:
	rm *~ -f
	rm $(OBJS) -f
	rm $(OUTPUT) -f


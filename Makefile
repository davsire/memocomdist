TARGET = memcomdist
SRC = main.c
CC = gcc

all: $(SRC)
	$(CC) $(SRC) -o $(TARGET)

clean:
	rm $(TARGET)

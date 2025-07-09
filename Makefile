TARGET = memocomdist
SRC = src/main.c
CC = gcc

all: $(SRC)
	$(CC) $(SRC) -o $(TARGET)

clean:
	rm $(TARGET)

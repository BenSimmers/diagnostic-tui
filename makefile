CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LIBS = -lncurses
TARGET = main.out
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

to-asm:
	$(CC) $(CFLAGS) -S $(SRC)

run-asm: $(TARGET)
	./$(TARGET)
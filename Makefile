CC = gcc
CFLAGS = $(shell pkg-config --cflags cairo) -Wno-implicit-function-declaration
LDFLAGS = $(shell pkg-config --libs cairo) -lX11 -lXext -lm

TARGET = clock
SRC = clock.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

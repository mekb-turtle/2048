CC=cc
CFLAGS=-Wall -O2
LFLAGS=-lm -s

OBJS=2048.o

TARGET=2048

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -fv -- $(OBJS) $(TARGET)

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -pthread
LDFLAGS := -pthread

TARGET  := proxy
SRCS    := proxy.c picohttpparser.c
OBJS    := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

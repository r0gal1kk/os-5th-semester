TARGETS = cancel_a cancel_b cancel_c cancel_d
SRCS    = $(wildcard src/*.c)

CC      = gcc
RM      = rm -f
CFLAGS  = -g -Wall -Wextra -Werror -fsanitize=address -fsanitize=leak
LIBS    = -lpthread
INCLUDE_DIR = "."
CFLAGS += -D_GNU_SOURCE

all: $(TARGETS)

$(TARGETS): %: src/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< $(LIBS) -o $@

clean:
	$(RM) $(TARGETS) *.o

re: clean all

.PHONY: all clean re
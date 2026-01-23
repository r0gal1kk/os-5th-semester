TARGETS = joinable_a joinable_b joinable_c joinable_d joinable_e joinable_f
SRCS = $(wildcard src/*.c)

CC = gcc
RM = rm -f
CFLAGS = -g -Wall -Wextra -Werror -fsanitize=address -fsanitize=leak
LIBS = -lpthread
INCLUDE_DIR = "."

all: $(TARGETS)

$(TARGETS): %: src/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< $(LIBS) -o $@

clean:
	$(RM) $(TARGETS) *.o

re: clean all

.PHONY: all clean re

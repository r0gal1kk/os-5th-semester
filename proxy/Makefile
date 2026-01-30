CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -pthread -I.
LDFLAGS := -pthread

TARGET  := proxy-http
BUILDDIR := build

SRCS    := proxy.c picohttpparser.c
OBJS    := $(SRCS:%.c=$(BUILDDIR)/%.o)

.PHONY: all clean distclean run

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJS) | $(BUILDDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/$(TARGET)

distclean: clean
	rm -rf $(BUILDDIR)

run: all
	./$(BUILDDIR)/$(TARGET)

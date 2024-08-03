# Detect the operating system
UNAME_S := $(shell uname -s)

# Default target
all: timer

# macOS specific settings
ifeq ($(UNAME_S),Darwin)
    CC = clang
    CFLAGS = -framework Cocoa
    SOURCE = timer.m
# Linux (Wayland) specific settings
else
    CC = gcc
    CFLAGS = -lX11 `pkg-config --cflags freetype2` -lXft -lXrender -lm
    SOURCE = timer.c
endif

# Compile the timer
timer: $(SOURCE)
	$(CC) $< $(CFLAGS) -o $@

# Clean up
clean:
	rm -f timer

.PHONY: all clean

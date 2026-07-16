# cf-ddns — DDNS client for Cloudflare in C
# Usage:
#   make          build ./cf-ddns
#   make run      build and run
#   make clean    remove the executable

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2 -Ivendor/cjson
CFLAGS  += $(shell pkg-config --cflags libcurl)
LDLIBS  := $(shell pkg-config --libs libcurl)

TARGET  := cf-ddns

# Sources: every .c in src/ (auto-discovered) + third-party library
SRCS    := $(wildcard src/*.c) vendor/cjson/cJSON.c

.PHONY: all run clean

all: $(TARGET)

# Compile all sources at once, straight into the executable (no intermediate .o/.d)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

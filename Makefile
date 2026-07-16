# cf-ddns — cliente DDNS para Cloudflare em C
# Uso:
#   make          compila para build/cf-ddns
#   make run      compila e executa
#   make clean    remove os artefatos de build

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2 -Ivendor/cjson
CFLAGS  += $(shell pkg-config --cflags libcurl)
LDLIBS  := $(shell pkg-config --libs libcurl)

BUILD   := build
TARGET  := $(BUILD)/cf-ddns

# Fontes: teu código + biblioteca de terceiros (vendored)
SRCS    := ddns-updater.c vendor/cjson/cJSON.c
OBJS    := $(SRCS:%.c=$(BUILD)/%.o)
DEPS    := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

# Compila cada .c para build/, recriando a subpasta e gerando .d (dependências de headers)
$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD)

-include $(DEPS)

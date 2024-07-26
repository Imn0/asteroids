# CC = clang
CC = gcc
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Wpedantic 
CFLAGS += -g 
# CFLAGS = -ggdb3 
# CFLAGS += -Wno-pointer-arith -Wno-unused-parameter -Wno-int-conversion
# CFLAGS +=  -fbracket-depth=1024
CFLAGS += -Isrc/headers
CFLAGS += -Isrc/common
LDFLAGS = -lm -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
OBJDIR = obj
OBJ  = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
BIN = bin
TARGET = $(BIN)/asteroids

.PHONY: all clean run

all: $(TARGET)

run: $(TARGET)
	cd $(BIN) && ./asteroids

$(TARGET): $(OBJ) | $(BIN)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJDIR):
	mkdir -p $@
	@find src -type d | sed 's/src/obj/' | xargs mkdir -p

$(BIN):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
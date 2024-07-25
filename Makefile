# CC = clang
CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS = -g 
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

.PHONY: all clean

all: dirs lnk

dirs:
	mkdir -p $(OBJDIR) $(BIN)
	@find src -type d | sed 's/src/obj/' | xargs mkdir -p

run: all
	cd $(BIN) && ./asteroids

lnk: $(OBJ)
	$(CC) -o $(BIN)/asteroids $^ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(OBJDIR)
	rm -f $(BIN)/asteroids
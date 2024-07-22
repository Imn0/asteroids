CC = gcc
CFLAGS = -std=c17 -O2 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS = -g 
# CFLAGS = -ggdb3 
CFLAGS += -Wno-pointer-arith -Wno-unused-parameter -Wno-int-conversion
# CFLAGS +=  -fbracket-depth=1024
LDFLAGS = -lm -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
OBJDIR = obj
OBJ  = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
BIN = bin

.PHONY: all clean

all: dirs lnk

dirs:
	mkdir -p $(OBJDIR) $(BIN)

run: all
	cd $(BIN) && ./3d-demo

lnk: $(OBJ)
	$(CC) -o $(BIN)/3d-demo $^ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(OBJDIR)
	rm $(BIN)/3d-demo

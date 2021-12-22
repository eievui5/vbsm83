CC := clang++
BIN := bin/dcc-backend
OBJS := $(patsubst src/%.cpp, obj/%.o, $(shell find src/ -name '*.cpp'))

DEBUGFLAGS := -O0 -g
CFLAGS := -Isrc/include -std=c++17 -Wall -Wimplicit-fallthrough -include "defines.hpp"
CFLAGS += $(DEBUGFLAGS)

all: $(BIN)

clean:
	rm -rf bin/
	rm -rf obj/

rebuild:
	$(MAKE) clean
	$(MAKE) all

test: all
	@echo "	=== BEGINNING TEST! ==="
	./$(BIN) -v -o bin/output.asm -i examples/adder.dcc

memcheck: all
	valgrind --leak-check=full ./$(BIN) -v -o bin/output.asm -i examples/adder.dcc

# Compile each source file.
obj/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link the output binary.
$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

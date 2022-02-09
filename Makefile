CC := gcc
BIN := bin/dcc-backend
OBJS := $(patsubst src/%.c, obj/%.o, $(shell find src/ -name '*.c'))

CFLAGS := -Isrc/include -Isrc/ -std=c17 -Wall -Wimplicit-fallthrough -Wno-unused-result -Wno-parentheses -MD
RELEASEFLAGS := -Os -s -flto
DEBUGFLAGS := -Og -g
TESTFLAGS := --ir - --input examples/adder.dcc

CFLAGS += $(DEBUGFLAGS)

all: $(BIN)

clean:
	rm -rf bin/ obj/

rebuild: clean all

test: all
	./$(BIN) $(TESTFLAGS)

memcheck: all
	valgrind --leak-check=full ./$(BIN) $(TESTFLAGS)

# Compile each source file.
obj/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link the output binary.
$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

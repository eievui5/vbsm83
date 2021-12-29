CC := gcc
BIN := bin/dcc-backend
OBJS := $(patsubst src/%.c, obj/%.o, $(shell find src/ -name '*.c'))

CFLAGS := -Isrc/include -std=c17 -Wall -Wimplicit-fallthrough -Wno-unused-result -MD -O0 -g
TESTFLAGS := -o - -i examples/adder.dcc

all:
	$(MAKE) $(BIN)

clean:
	rm -rf bin/ obj/

rebuild:
	$(MAKE) clean
	$(MAKE) all

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

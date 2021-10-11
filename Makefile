BIN := bin/dcc-backend
OBJS := $(patsubst src/%.c, obj/%.o, $(shell find src/ -name '*.c'))

CFLAGS := -Isrc/include

all: $(BIN)

clean:
	rm -rf bin/
	rm -rf obj/

rebuild:
	$(MAKE) clean
	$(MAKE) all

test: all
	@echo " === BEGINNING TEST! ==="
	./$(BIN) -o bin/output.asm -i examples/adder.dcc --info

# Compile each .c file.
obj/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $^

# Link the output binary.
$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^
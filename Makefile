BIN := bin/dcc-backend
OBJS := $(patsubst src/%.cpp, obj/%.o, $(shell find src/ -name '*.cpp'))

CFLAGS := -Isrc/include -std=c++17

all: $(BIN)

clean:
	rm -rf bin/
	rm -rf obj/

rebuild:
	$(MAKE) clean
	$(MAKE) all

test: all
	@echo " === BEGINNING TEST! ==="
	./$(BIN) -v -o bin/output.asm -i examples/adder.dcc

# Compile each .c file.
obj/%.o: src/%.cpp
	@mkdir -p $(@D)
	g++ $(CFLAGS) -c -o $@ $<

# Link the output binary.
$(BIN): $(OBJS)
	@mkdir -p $(@D)
	g++ $(CFLAGS) -o $@ $^
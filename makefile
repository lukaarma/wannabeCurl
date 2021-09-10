CC = gcc -Wall -pedantic -std=gnu99
HEADERS := $(wildcard ./src/*.h)
OBJECTS := $(patsubst ./src/%.c, ./obj/%.o, $(wildcard ./src/*.c))

.PHONY: clean debug

# $^ replaced by all prerequisites, $@ replaced by target
wannabeCurl: $(OBJECTS)
	$(CC) $^ -o $@ -lssl -lcrypto

# $< replaced by the first prerequisite, used since we are just compiling
obj/%.o: src/%.c $(HEADERS)
	$(CC) -c $< -o $@

# -g adds debug info to the executable, -O0 helps Valgrind
debug: $(OBJECTS)
	$(CC) $^ -o wannabeCurl -lssl -lcrypto -g -O0

clean:
	rm -f $(OBJECTS) ./wannabeCurl ./out/*

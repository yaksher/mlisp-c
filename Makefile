CFLAGS = -Wall -Wextra -fsanitize=address,undefined -g -Iinclude
LIBS = $(shell ls src | grep -E '.*\.c' | sed 's/\.c//g')
OBJS = $(addprefix out/,$(LIBS:=.o))

run: bin/main
	bin/main $(ARGS)

out/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

bin/%: out/%.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
bin/%: %.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin/*.dSYM
	rm -f out/*.o bin/*

.PHONY: make run clean

all: clean test

clean:
	rm -f test-throw

test:
	$(CC) test.c -std=c99 -o test-throw
	./test-throw

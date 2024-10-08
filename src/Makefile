CC = gcc
CFLAGS = -std=c11
SF = -fsanitize=leak,address

all: primaUDPServer

primaUDPServer: main.c
	$(CC) $(CFLAGS) prima_server.c main.c  -o prima

test: prima
	$(CC) $(CFLAGS) -o prima_test prima_server.c ./test/test.c -lcheck -lsubunit -lm
	./prima_test
clean:
	rm -rf prima prima_test
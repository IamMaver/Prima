CC = gcc
CFLAGS = -std=c11
SF = -fsanitize=leak,address

all: primaUdpServer

primaUdpServer: main.c prima_server.c prima_tlv.c
	$(CC) $(CFLAGS) main.c prima_tlv.c prima_functions.c prima_server.c -o prima

test: prima
	$(CC) $(CFLAGS) -o prima_test prima_tlv.c prima_functions.c prima_server.c ./test/test.c -lcheck -lsubunit -lm
	./prima_test
clean:
	rm -rf prima prima_test
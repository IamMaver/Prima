#ifndef PRIMA
#define PRIMA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#define PORT 31337
#define BUFFER_SIZE 65535
#define TIMEOUT -1
#define HEADER_T 4
#define HEADER_L 4
#define HEADER_TL 8

typedef struct
{
    uint32_t tag;
    char *nameCommand;
    int (*isValid)(uint32_t length, const uint8_t *value);
    void (*processCommand)(uint32_t length, const uint8_t *value);
} Command;

void ServerUdp(uint16_t port);
int setSocketNonblocking(int sockfd);
int isValidPowerControl(uint32_t length, const uint8_t *value);
void processPowerControl(uint32_t length, const uint8_t *value);
int isValidNetworkLabel(uint32_t length, const uint8_t *value);
void processNetworkLabel(uint32_t length, const uint8_t *value);
ssize_t getCommandIndex(uint32_t tag, Command commands[], size_t commandsAmount);
#endif
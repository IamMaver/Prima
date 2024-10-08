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

#define PORT 31337 // We are Elite))
#define BUFFER_SIZE 1024
#define TIMEOUT 5000

typedef struct
{
    uint32_t tag;
    char *nameCommand;
    int (*isValid)(uint32_t length, const uint8_t *value);
    void (*processCommand)(uint32_t length, const uint8_t *value, char *nameCommand);
} Command;

int isValid(uint32_t length, const uint8_t *value);
void processCommand(uint32_t length, const uint8_t *value, char *nameCommand);
void Server_UDP(uint16_t port);
int set_socket_nonblocking(int sockfd);
int isValidPowerControl(uint32_t length, const uint8_t *value);
void processPowerControl(uint32_t length, const uint8_t *value, char *nameCommand);
int isValidNetworkLabel(uint32_t length, const uint8_t *value);
void processNetworkLabel(uint32_t length, const uint8_t *value, char *nameCommand);
ssize_t getCommandIndex(uint32_t tag, Command commands[], size_t commandsAmount);
#endif
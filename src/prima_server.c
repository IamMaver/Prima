#include "prima.h"

int set_socket_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        return -1;
    }
    return 0;
}

//Для добавления команд нужно подготовить для них 2 функции - валидатор и исполнитель, а также добавить их в массив, не забыв увеличить переменную с размером массива.

int isValidPowerControl(uint32_t length, const uint8_t *value)
{
    if (length != 1 || (value[0] != 0 && value[0] != 1))
    {
        return 0;
    }
    return 1;
}

void processPowerControl(uint32_t length, const uint8_t *value, char *nameCommand)
{
    if (value[0] == 0)
    {
        fprintf(stderr, "%s: Power ON\n", nameCommand);
    }
    else
    {
        fprintf(stderr, "%s: Power OFF\n", nameCommand);
    }
}

int isValidNetworkLabel(uint32_t length, const uint8_t *value)
{
    if (length == 0 || length > 255)
    {
        return 0;
    }
    return 1;
}

void processNetworkLabel(uint32_t length, const uint8_t *value, char *nameCommand)
{
    // немного кода для борьбы со злыми ненультерминированными строками
    char networkLabel[256];
    memcpy(networkLabel, value, length);
    networkLabel[255] = 0;
    fprintf(stderr, "%s: Network label = %.*s\n", networkLabel, length, value);
}

ssize_t getCommandIndex(uint32_t tag, Command commands[], size_t commandsAmount)
{
    ssize_t result = -1;
    for (size_t i = 0; i < commandsAmount; i++)
    {
        if (commands[i].tag == tag)
        {
            result = i;
            break;
        }
    }
    return result;
}

void Server_UDP(uint16_t port)
{

    Command commands[] = {
        {0x0, "Power Control", isValidPowerControl, processPowerControl},
        {0x1, "Network Label", isValidNetworkLabel, processNetworkLabel},
    };
    size_t commandsAmount = 2; // Ох уж мне эти Сишные массивы((

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    uint8_t buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        close(sockfd);
        exit(-1);
    }

    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 1, TIMEOUT);
        if (ret < 0)
        {
            perror("poll error");
            break;
        }
        else if (ret == 0)
        {
            // Таймаут
            // fprintf(stderr, "Timeout occurred, no data received.\n");
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            len = sizeof(cliaddr);
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    continue;
                }
                else
                {
                    perror("recvfrom error");
                    break;
                }
            }

            int processed_commands = 0;
            int offset = 0;

            while (offset + 8 <= n)
            {

                uint32_t tag, length;
                memcpy(&tag, buffer + offset, 4);
                memcpy(&length, buffer + offset + 4, 4);

                tag = ntohl(tag);
                length = ntohl(length);

                offset += 8;

                if (offset + length > n)
                {
                    fprintf(stderr, "Invalid length for tag 0x%08X\n", tag);
                    break;
                }

                ssize_t index = getCommandIndex(tag, commands, commandsAmount);
                if (index<0||!commands[index].isValid(length, buffer + offset))
                {
                    fprintf(stderr,"ERROR!\n");
                    break;
                }
                commands[index].processCommand(length, buffer + offset, commands[tag].nameCommand);
                offset += length;
                processed_commands++;
            }

            if (processed_commands > 0 && offset == n)
            {
                sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&cliaddr, len);
            }
        }
    }
    close(sockfd);
}
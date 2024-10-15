#include "prima.h"

int setSocketNonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        fprintf(stderr, "fcntl F_GETFL\n");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        fprintf(stderr, "fcntl F_SETFL O_NONBLOCK\n");
        return -1;
    }
    return 0;
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

void ServerUdp(uint16_t port)
{

    Command commands[] = {
        {0x0, "Power Control", isValidPowerControl, processPowerControl},
        {0x1, "Network Label", isValidNetworkLabel, processNetworkLabel},
    };
    size_t commandsAmount = sizeof(commands) / sizeof(commands[0]);

    int sockfd;

    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "socket creation failed\n");
        exit(-1);
    }

    if (setSocketNonblocking(sockfd) < 0)
    {
        fprintf(stderr, "failed to set non-blocking mode on socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr = {0}, cliaddr = {0};

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "bind failed");
        exit(-1);
    }

    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    uint8_t buffer[BUFFER_SIZE];
    int noError = 1;
    while (1)
    {
        int ret = poll(fds, 1, TIMEOUT);
        if (ret < 0)
        {
            fprintf(stderr, "poll error\n");
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            len = sizeof(cliaddr);
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
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
            u_int64_t offset = 0;
            noError = 1;
            while (offset + HEADER_TL <= n)
            {

                uint32_t tag, length;
                memcpy(&tag, buffer + offset, HEADER_T);
                memcpy(&length, buffer + offset + HEADER_T, HEADER_L);

                tag = ntohl(tag);
                length = ntohl(length);

                offset += HEADER_TL;

                if (offset + length > n)
                {
                    fprintf(stderr, "Invalid length for tag 0x%08X\n", tag);
                    break;
                }

                ssize_t index = getCommandIndex(tag, commands, commandsAmount);
                if (index < 0 || !commands[index].isValid(length, buffer + offset))
                {
                    fprintf(stderr, "ERROR!\n");
                    noError = 0;
                    continue;
                }
                commands[index].processCommand(length, buffer + offset);
                offset += length;
                processed_commands++;
            }

            if (processed_commands > 0 && noError)
            {
                fds[0].events = POLLOUT;

                int ret = poll(fds, 1, TIMEOUT);
                if (ret > 0 && (fds[0].revents & POLLOUT))
                {
                    ssize_t sent_bytes = sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&cliaddr, len);
                    if (sent_bytes < 0)
                    {
                        fprintf(stderr, "sendto failed\n");
                    }
                }
                else if (ret < 0)
                {
                    fprintf(stderr, "poll error during send\n");
                }
                else if (ret == 0)
                {
                    fprintf(stderr, "timeout waiting for socket to become writable\n");
                }

                fds[0].events = POLLIN;
            }
        }
    }
    close(sockfd);
}
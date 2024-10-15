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

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setSocketNonblocking(sockfd) < 0) {
        fprintf(stderr, "failed to set non-blocking mode on socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr = {0}, cliaddr = {0};

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    uint8_t buffer[BUFFER_SIZE];
    socklen_t len = sizeof(cliaddr);

    while (1) {
        int ret = poll(fds, 1, TIMEOUT);
        if (ret < 0) {
            perror("poll error");
            break;
        } else if (ret == 0) {
            continue;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                    continue;
                } else {
                    perror("recvfrom error");
                    break;
                }
            }

            u_int64_t offset = 0;
            int noError = 1;

            while (offset + HEADER_TL <= n) {
                TLVCommand tlvCmd;
                if (decodeTLV(buffer + offset, n - offset, &tlvCmd) < 0) {
                    fprintf(stderr, "TLV decoding error\n");
                    noError = 0;
                    break;
                }

                ssize_t index = getCommandIndex(tlvCmd.tag, commands, commandsAmount);
                if (index < 0 || !commands[index].isValid(tlvCmd.length, tlvCmd.value)) {
                    fprintf(stderr, "Invalid command\n");
                    noError = 0;
                } else {
                    commands[index].processCommand(tlvCmd.length, tlvCmd.value);
                }

                offset += HEADER_TL + tlvCmd.length;
                freeTLV(&tlvCmd);
            }

            if (noError) {
                fds[0].events = POLLOUT;
                ret = poll(fds, 1, TIMEOUT);
                if (ret > 0 && (fds[0].revents & POLLOUT)) {
                    sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&cliaddr, len);
                }
                fds[0].events = POLLIN;
            }
        }
    }
    close(sockfd);
}
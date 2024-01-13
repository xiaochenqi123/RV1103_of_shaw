/*
UDP loopback test on rv1103
DATE :12/1/2024
Auther : SHAW

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "udp_client.h"

#define HOST "192.168.1.100"  // 主机IP地址
#define PORT 5000
#define BUFFER_SIZE (4 * 1024) // 数据缓存区4K

int main(void)
{
    int sockfd, ret;
    struct sockaddr_in client;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    printf("UDP CLIENT TEST\r\n");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    printf("UDP server started on port %d...\n", PORT);

    while (1)
    {
        socklen_t addr_len = sizeof(struct sockaddr_in);

        int len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &addr_len);

        if (len > 0)
        {
            while(1)
            {
            sendto(sockfd, buffer, len, 0, (struct sockaddr *)&client, addr_len);
            }
        }
    }

    close(sockfd);
    return 0;
}

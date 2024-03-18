#include "loopback.h"
#include "adc.h"
#include "udpsend.h"
#include "main.h"
#include "sd.h"


#define PORT2          5000
#define BUF_SIZE       100

struct client_addr;
extern uint8_t WORKMODE;
extern int sockfd2;
extern struct sockaddr_in loopback;
char recv_buffer[128];

extern socklen_t client_addr_size;
void initsockfd2(void)
{
    sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd2 < 0)
    {
        perror("socket");
        exit(1);
    }
    memset(&loopback, 0 ,sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = htonl(INADDR_ANY);
    loopback.sin_port = htons(PORT2);
    if (bind(sockfd2, (struct sockaddr *)&loopback, sizeof(loopback)) < 0)
    {
        perror("bind");
        exit(1);
    }
    printf("UDP Echo Server started on port %d\n", PORT2);
}

void switch_work_mode(void) {
    struct sockaddr_in client_addr;
    while (1) { 

        client_addr_size = sizeof(client_addr);
        ssize_t recv_len = recvfrom(sockfd2, recv_buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_size);
        if (recv_len == -1) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }
        if (strcmp(inet_ntoa(client_addr.sin_addr), "192.168.1.100") == 0) {
        // 发送回复到上位机
        if (sendto(sockfd2, recv_buffer, recv_len, 0, (struct sockaddr *)&client_addr, client_addr_size) == -1) {
                perror("sendto failed");
                exit(EXIT_FAILURE);
            }

    }
    }
}

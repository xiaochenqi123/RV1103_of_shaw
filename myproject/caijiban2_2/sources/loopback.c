#include "loopback.h"
#include "adc.h"
#include "udpsend.h"
#include "main.h"
#include "sd.h"


#define PORT2          6000


extern uint8_t WORKMODE;
extern int sockfd2;
extern struct sockaddr_in loopback;
char recv_buffer[128];
void initsockfd2(void)
{
    if ((sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
 

    memset(&loopback , 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    
    loopback.sin_addr.s_addr = inet_addr(HOST);
    loopback.sin_port = htons(PORT2);

    if (bind(sockfd2, (struct sockaddr *)&loopback, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    printf("UDP server started on port %d...\n", PORT2);
}

void switch_work_mode(void) {
    while (1) {
        socklen_t addr_len = sizeof(struct sockaddr_in);
        int len = recvfrom(sockfd2, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&loopback, &addr_len);
        if (len > 0) {
            recv_buffer[len] = '\0';
            if (strcmp(recv_buffer, "UDPMODE") == 0) {
                WORKMODE = UDPMODE;
                printf("Switched to UDPMODE\n");
            } else if (strcmp(recv_buffer, "SDMODE") == 0) {
                WORKMODE = SDMODE;
                printf("Switched to SDMODE\n");
            } else {
                printf("Invalid mode\n");
            }
        }
    }
}

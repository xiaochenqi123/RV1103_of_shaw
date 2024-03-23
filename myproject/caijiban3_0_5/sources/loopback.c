#include "loopback.h"
#include "adc.h"
#include "udpsend.h"
#include "main.h"
#include "sd.h"


#define PORT2          5000
#define BUF_SIZE       100

struct client_addr;
extern uint8_t WORKMODE;
extern struct pollfd fds[1];
extern uint16_t flag ;
extern int sockfd2;
extern struct sockaddr_in loopback;
char recv_buffer[128];
extern pthread_mutex_t flag_mutex;
extern socklen_t client_addr_size;
extern sem_t udp_clode;                //用于关闭UDP发送线程的信号量
extern sem_t sd_close;                 // 用于关闭SD卡线程的信号量
extern sem_t adc_close;                // 用于关闭ADC采集线程的信号量
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


    while (1) {
        int ret = poll(fds, 1, -1);  // 永久等待直到有事件发生
        if (ret == -1) {
            perror("poll");
            // 处理错误
        } else if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                ssize_t recv_len = recv(sockfd2, recv_buffer, BUF_SIZE, 0);
                if (recv_len > 0) {
                    printf("change mode\n");

                    recv_buffer[recv_len] = '\0';
                    if (strcmp(recv_buffer, "UDPMODE") == 0) {
                        pthread_mutex_lock(&flag_mutex);
                        flag = 0;
                        pthread_mutex_unlock(&flag_mutex);
                        WORKMODE = UDPMODE;
                        printf("Switched to UDPMODE\n");
                    } else if (strcmp(recv_buffer, "SDMODE") == 0) {
                        pthread_mutex_lock(&flag_mutex);
                        flag = 0;
                        pthread_mutex_unlock(&flag_mutex);
                        WORKMODE = SDMODE;
                        printf("Switched to SDMODE\n");
                    } else if (strcmp(recv_buffer, "CLOSE") == 0) {
                        pthread_mutex_lock(&flag_mutex);
                        flag = 0;
                        pthread_mutex_unlock(&flag_mutex);
                        printf("CLOSE FILE\n");
                        fclose(sd_file);
                        printf("Switched to SDMODE\n");
                    } else if (strcmp(recv_buffer, "NONE") == 0) {
                        printf("Switch to none mode\n");
                        sem_post(&adc_close);
                        sem_post(&sd_close);
                        sem_post(&udp_clode);
                        pthread_mutex_lock(&flag_mutex);
                        flag = 0;
                        pthread_mutex_unlock(&flag_mutex);
                        WORKMODE = NONE;
                        printf("Switched to NONE\n");
                    } else {
                        printf("Invalid mode\n");
                    }
                }
            }
        }
    }
}

/**
 * caijiban code
 * DATE 2024/3/6
 * AUTHOR shaw
 * VERSION 1.0
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
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define HOST "192.168.1.100"
#define PORT 5000
#define BUFFER_SIZE 1400
#define TIMER_INTERVAL_US 20
#define UDPMODE 0
#define SDMODE  1

int sockfd;
uint16_t flag = 0;
uint16_t flag_2 = 0;
struct sockaddr_in client;
uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区
uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
uint8_t WORKMODE = UDPMODE;
uint8_t timer_flag = 0;

FILE *scale_file, *in0_raw_file, *sd_file;

pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;

void read_ADC_values() {
    fseek(scale_file, 0, SEEK_SET);
    fseek(in0_raw_file, 0, SEEK_SET);

    if (scale_file && in0_raw_file)
    {
        pthread_mutex_lock(&flag_mutex);
        flag += 2;
        
        pthread_mutex_unlock(&flag_mutex);

        fgets(buffer1, BUFFER_SIZE, scale_file);
        float scale = strtof(buffer1, NULL);

        fgets(buffer1, BUFFER_SIZE, in0_raw_file);
        uint16_t in0_raw_value = atoi(buffer1);

        // 获取高八位和低八位
        uint8_t high_byte = (in0_raw_value >> 8) & 0xFF;
        uint8_t low_byte = in0_raw_value & 0xFF;

        if (flag_2 % 2 == 0) {
            buffer1[flag - 2] = high_byte;
            buffer1[flag - 1] = low_byte;
        } else {
            buffer2[flag - 2] = high_byte;
            buffer2[flag - 1] = low_byte;
        }
    }
}

void timer_handler(int signum, siginfo_t *info, void *context)
{
    timer_flag = 1;
}

void *timer_thread(void *arg)
{
    struct sigaction sa;
    struct sigevent sev;
    timer_t timerid;

    // 设置定时器信号处理函数
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler; 
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // 创建定时器
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
    {
        perror("timer_create");
        exit(1);
    }

    // 设置定时器的间隔时间
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = TIMER_INTERVAL_US * 1000;  // 将微秒转换为纳秒
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = TIMER_INTERVAL_US * 1000;
    if (timer_settime(timerid, 0, &its, NULL) == -1)
    {
        perror("timer_settime");
        exit(1);
    }

    // 等待定时器线程退出
    pthread_exit(NULL);
}

// 辅助函数：切换工作模式
void switch_work_mode(const char *mode_str) {
    if (strcmp(mode_str, "UDPMODE") == 0) {
        WORKMODE = UDPMODE;
        printf("Switched to UDPMODE\n");
    } else if (strcmp(mode_str, "SDMODE") == 0) {
        WORKMODE = SDMODE;
        printf("Switched to SDMODE\n");
    } else {
        printf("Invalid mode\n");
    }
}

int main(void)
{
    scale_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage_scale", "r");
    in0_raw_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");

    if (scale_file == NULL || in0_raw_file == NULL)
    {
        perror("Error opening ADC files");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(HOST);
    client.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("connect");
        exit(1);
    }

    printf("UDP server started on port %d...\n", PORT);

    // 创建定时器线程
    pthread_t tid;
    if (pthread_create(&tid, NULL, timer_thread, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }

    while (1)
    {
        // 接收来自上位机的数据包
        char recv_buffer[128];
        ssize_t bytes_received = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
        if (bytes_received > 0) {
            printf("check point 1");
            recv_buffer[bytes_received] = '\0';
            switch_work_mode(recv_buffer);
        }

        // 如果 timer_flag 被置位，执行 ADC 数据采集
        if (timer_flag == 1)
        {
            read_ADC_values();
            flag_2++;
            timer_flag = 0;
        }

        pthread_mutex_lock(&flag_mutex);
        if (flag == BUFFER_SIZE) {
            if (WORKMODE == UDPMODE) {
                if (flag_2 % 2 == 0) {
                    ssize_t bytes_sent = send(sockfd, buffer1, BUFFER_SIZE, 0);
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                } else {
                    ssize_t bytes_sent = send(sockfd, buffer2, BUFFER_SIZE, 0);
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                }
            } else if (WORKMODE == SDMODE) {
                FILE *sd_file;
                sd_file = fopen("/mnt/sdcard/test.txt", "a");
                if (sd_file == NULL)
                {
                    perror("Error opening SD card file");
                    exit(1);
                }
                if (flag_2 % 2 == 0) {
                    fwrite(buffer1, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                } else {
                    fwrite(buffer2, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                }
                fclose(sd_file);
            }
            flag = 0;
            pthread_mutex_unlock(&flag_mutex);
        } else {
            pthread_mutex_unlock(&flag_mutex);
            usleep(1000); // sleep for 1ms to prevent busy waiting
        }
    }

    close(sockfd);
    fclose(scale_file);
    fclose(in0_raw_file);

    return 0;
}

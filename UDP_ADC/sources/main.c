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



#define HOST "192.168.1.100"
#define PORT 5000
#define BUFFER_SIZE (4 * 1024)
#define TIMER_INTERVAL_US 20

int sockfd;
uint16_t flag = 0;
struct sockaddr_in client;
uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区
uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
uint8_t *active_buffer = buffer1;  // 指向当前活跃的缓冲区
FILE *scale_file, *in0_raw_file;

void timer_handler(int signum, siginfo_t *info, void *context)
{
    fseek(scale_file, 0, SEEK_SET);
    fseek(in0_raw_file, 0, SEEK_SET);

    if (scale_file && in0_raw_file)
    {
        flag += 2;

        fgets(active_buffer, BUFFER_SIZE, scale_file);
        float scale = strtof(active_buffer, NULL);

        fgets(active_buffer, BUFFER_SIZE, in0_raw_file);
        uint16_t in0_raw_value = atoi(active_buffer);

        // 获取高八位和低八位
        uint8_t high_byte = (in0_raw_value >> 8) & 0xFF;
        uint8_t low_byte = in0_raw_value & 0xFF;

        // 存储到当前活跃的缓冲区中
        active_buffer[flag - 2] = high_byte;
        active_buffer[flag - 1] = low_byte;
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

    struct sigaction sa;
    struct sigevent sev;
    timer_t timerid;

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler; 
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
    {
        perror("timer_create");
        exit(1);
    }

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

    while (1)
    {
        if (flag >= 1400)
        {
            ssize_t bytes_sent = send(sockfd, active_buffer, flag, 0);
            if (bytes_sent == -1)
            {
                perror("send");
                exit(1);
            }
            flag = 0;

            // 切换活跃的缓冲区
            active_buffer = (active_buffer == buffer1) ? buffer2 : buffer1;
        }
    }

    close(sockfd);
    fclose(scale_file);
    fclose(in0_raw_file);

    return 0;
}
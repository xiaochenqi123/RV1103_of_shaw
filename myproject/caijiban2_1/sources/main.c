/**
 * caijiban code
 * DATE 2024/3/6
 * AUTHOR shaw
 * VERSION 2.0
*/
#include "main.h"
#include "sd.h"
#include "loopback.h"
#include "adc.h"
#include "udpsend.h"
#include "sd.h"
#include <sys/time.h>


#define PORT 5000
#define TIMER_INTERVAL_US 100
#define UDPMODE 0
#define SDMODE  1
#define NONE    2



extern pthread_t LoopbackID;  // udp回环线程
extern pthread_t ADCID;       // adc采集线程
extern pthread_t UDPSendID;   // UDP发送线程
extern pthread_t SDSaveID;    // SD卡存储线程


int sockfd;
int sockfd2;               //创建两个Socket用于回环
uint16_t flag = 0;
uint16_t flag_2 = 0;
struct sockaddr_in client;
struct sockaddr_in loopback;
uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区
uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
uint8_t WORKMODE = UDPMODE;
uint8_t timer_flag = 0;

extern char recv_buffer[128];     //回环的缓冲区

FILE *scale_file, *in0_raw_file, *sd_file;

pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;



void timeout_info(int signum, siginfo_t *info, void *context)
{
    timer_flag = 1;
    //flag=flag+2;
    //printf("flag0=%d\r\n",flag);
}

//指定定时操作内容
void init_sigaction(void)
{
    struct sigaction tact;
    tact.sa_handler = timeout_info;
    tact.sa_flags = 0;
    sigemptyset(&tact.sa_mask);
    sigaction(SIGALRM, &tact, NULL);
}

//设定定时时间
void init_timer(void)
{
    // 设置定时器的间隔时间
     struct itimerval it_val;
    it_val.it_value.tv_sec = 0;
    it_val.it_value.tv_usec = TIMER_INTERVAL_US;
    it_val.it_interval = it_val.it_value;
    setitimer(ITIMER_REAL, &it_val, NULL);

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


    /*创建回环所需的socket*/
    //initsockfd2();

    // //初始化回环线程
    // pthread_create(&LoopbackID, NULL, switch_work_mode, NULL);
    // printf("Loopback thread created\r\n");

    //初始化ADC线程
    pthread_create(&ADCID, NULL, read_ADC_values, NULL);
    printf("ADC thread created\r\n");

    //初始化UDP发送线程
    pthread_create(&UDPSendID, NULL, udpsend, NULL);
    printf("UDPSend thread created\r\n");

    // //初始化SD卡存储线程
    // pthread_create(&SDSaveID, NULL, sdsave, NULL);
    // printf("SDSave thread created\r\n");
    init_sigaction();
    init_timer();
    printf("init_sigaction\r\n");

    while (1)
    {
        sleep(1);
    }

  //  close(sockfd);
    fclose(scale_file);
    fclose(in0_raw_file);

    printf("program return\r\n");
    return 0;
}

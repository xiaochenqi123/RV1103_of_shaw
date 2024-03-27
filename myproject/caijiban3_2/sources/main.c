/**
 * caijiban code
 * DATE 2024/3/26
 * AUTHOR shaw
 * VERSION 3.2
*/
#include "main.h"
#include "sd.h"
#include "loopback.h"
#include "adc.h"
#include "udpsend.h"
#include "sd.h"
#include <sys/time.h>



#define PORT 5000
#define TIMER_INTERVAL_US 20

#define SDMODE  1


uint8_t *buffer1;
uint8_t *buffer2;
extern pthread_t LoopbackID;  // udp回环线程
extern pthread_t ADCID;       // adc采集线程
extern pthread_t UDPSendID;   // UDP发送线程
extern pthread_t SDSaveID;    // SD卡存储线程


int sockfd;                //创建Socket用于udp发送数据
int sockfd2;               //创建Socket用于回环
uint16_t flag = 0;
uint16_t flag_2 = 0;
uint16_t buffer_raw;
struct sockaddr_in client;
struct sockaddr_in loopback;
//用于poll函数
struct pollfd fds[1];

socklen_t client_addr_size;
//uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区

//uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
uint8_t WORKMODE = NONE;
uint8_t timer_flag = 0;

extern char recv_buffer[128];     //回环的缓冲区
sem_t semaphore;
sem_t adc_sd;
sem_t semaphore2;
sem_t udp_clode;                //用于关闭UDP发送线程的信号量
sem_t sd_close;                 // 用于关闭SD卡线程的信号量
sem_t adc_close;                // 用于关闭ADC采集线程的信号量

FILE *scale_file, *in0_raw_file, *sd_file;
FILE *sd_file;

pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;



void timeout_info(int signum, siginfo_t *info, void *context)
{
     sem_post(&semaphore2);
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
    buffer1 = (uint8_t*)malloc(BUFFER_SIZE * sizeof(uint8_t));
    buffer2 = (uint8_t*)malloc(BUFFER_SIZE * sizeof(uint8_t));
    scale_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage_scale", "r");
    in0_raw_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
    
    sd_file = fopen("/mnt/sdcard/data.txt", "w");
    
    //用于SD卡存储线程与adc线程的同步
    sem_init(&adc_sd,0, 0);
    //用于adc线程与UDP发送线程的同步
    sem_init(&semaphore, 0, 0);

    //用于定时器线程与主线程的同步
    sem_init(&semaphore2, 0, 0);

    sem_init(&udp_clode, 0, 0);
    sem_init(&sd_close, 0, 0);
    sem_init(&adc_close, 0, 0);
    
    if (sd_file == NULL)
    {
        perror("Error opening SD card file");
        exit(1);
    }else{
        printf("file open succeed\n");
    }
        fprintf(sd_file, "这是一个测试文件在SD卡中创建的内容。\n");

    // 关闭文件
    //fclose(sd_file);

    printf("文件已成功创建在SD卡中。\n");

    fds[0].fd = sockfd2;
    fds[0].events = POLLIN;


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
    initsockfd2();

    //初始化回环线程
    pthread_create(&LoopbackID, NULL, switch_work_mode, NULL);
    printf("Loopback thread created\r\n");

    //初始化ADC线程
    pthread_create(&ADCID, NULL, read_ADC_values, NULL);
    printf("ADC thread created\r\n");



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

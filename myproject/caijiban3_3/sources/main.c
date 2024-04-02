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
#include <semaphore.h>
#include <poll.h>

#define PORT 5000
#define PORT2 5000
#define BUFFER_SIZE 1400
#define TIMER_INTERVAL_US 20
#define HOST "192.168.1.100"
#define BUF_SIZE       100
#define UDPMODE 0
#define SDMODE 1
#define NONE 2

char recv_buffer[128];

int sockfd;            //创建Socket用于udp发送数据
int sockfd2;           //创建Socket用于回环
int WORKMODE;

pthread_t ADCID;       // adc采集线程
pthread_t UDPSendID; 
pthread_t SDSaveID;
pthread_t LoopbackID;

FILE *scale_file;
FILE *in0_raw_file;
FILE *sd_file;


sem_t semaphore;
sem_t semaphore2;

uint8_t *buffer1;
uint8_t *buffer2;
uint8_t high_byte;
uint8_t low_byte;

uint16_t highbyte1=0;
uint16_t lowbyte1=0;
uint16_t flag=0;
uint16_t flag_2=0;
uint16_t buffer_raw;

pthread_mutex_t flag_mutex;
pthread_mutex_t high_byte_mutex;
pthread_mutex_t low_byte_mutex;
pthread_t LoopbackID;  // udp回环线程

struct sockaddr_in client;
struct sockaddr_in loopback;

//初始化回环用到的套接字
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

         ssize_t recv_len = recv(sockfd2, recv_buffer, BUF_SIZE, 0);

         if (recv_len > 0) {
            pthread_mutex_lock(&flag_mutex);
            printf("change mode\r\n");

             recv_buffer[recv_len] = '\0';
            if (strcmp(recv_buffer, "UDPMODE") == 0) {
                 pthread_mutex_lock(&flag_mutex);
                 flag = 0;
                 pthread_mutex_unlock(&flag_mutex);
                WORKMODE = UDPMODE;
                //初始化UDP发送线程
                //pthread_create(&UDPSendID, NULL, udpsend, NULL);
                printf("UDPSend thread created\r\n");

                printf("Switched to UDPMODE\n");
            } else if (strcmp(recv_buffer, "SDMODE") == 0) {
                 
                 pthread_mutex_lock(&flag_mutex);
                 flag = 0;
                 pthread_mutex_unlock(&flag_mutex);
                WORKMODE = SDMODE;
                //初始化SD卡存储线程
               // pthread_create(&SDSaveID, NULL, sdsave, NULL);
                printf("SDSave thread created\r\n");
                printf("Switched to SDMODE\n");
            } else if (strcmp(recv_buffer,"CLOSE")==0){
                 pthread_mutex_lock(&flag_mutex);
                 flag = 0;
                 pthread_mutex_unlock(&flag_mutex);
                 WORKMODE = NONE;
                printf("CLOSE FILE\n");
                 // fclose(sd_file);
                  printf("CLOSE FILE\n");
             }else if (strcmp(recv_buffer,"NONE")==0){
                printf("swtich to none mode\n");
               // sem_post(&adc_close);
                //sem_post(&sd_close);
                //sem_post(&udp_clode);
                 pthread_mutex_lock(&flag_mutex);
                 flag = 0;
                 pthread_mutex_unlock(&flag_mutex);
                  WORKMODE = NONE;
             }else {
                 printf("Invalid mode\n");
             }
            pthread_mutex_unlock(&flag_mutex);
        }
        else{
            break;
        }
     }
}

void udpsend(void)
{
    while(1)
    {
        sem_wait(&semaphore);
        if(flag_2%2==0)
        {
            ssize_t bytes_sent = send(sockfd, buffer1, BUFFER_SIZE, 0);                    
            if (bytes_sent == -1)
            {
                perror("send");
                exit(1);
            }
        }else{
            ssize_t bytes_sent = send(sockfd, buffer2, BUFFER_SIZE, 0);                    
            if (bytes_sent == -1)
            {
                perror("send");
                exit(1);
            }
        }
    }

}


void sdsave(void)
{
   
  if(WORKMODE == SDMODE){
    while(1){
               sem_wait(&semaphore);
            // pthread_mutex_lock(&flag_mutex);
            // flag = 0;
            // pthread_mutex_unlock(&flag_mutex);
                if (flag_2 % 2 == 0) {
                size_t bytes_written=fwrite(buffer1, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                if (ferror(sd_file)) {
                // 写入发生错误
                perror("Error writing to file");
                } 
                
                pthread_mutex_lock(&flag_mutex);
                flag = 0;
                pthread_mutex_unlock(&flag_mutex);
                
                flag_2++;
               
            } else {
                size_t bytes_written=fwrite(buffer2, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                pthread_mutex_lock(&flag_mutex);
                flag = 0;
                pthread_mutex_unlock(&flag_mutex);
                flag_2--;
             
            }
                if (WORKMODE != SDMODE) {
                break; // Exit the loop if WORKMODE is no longer SDMODE
            }
            }
            // if(sem_wait(&sd_close)==0)
            // {
            //     pthread_exit(NULL); // 以默认退出状态退出线程
            // }
        
        }
    
    }

void read_ADC_values()
{   
    fseek(scale_file, 0, SEEK_SET);
    fseek(in0_raw_file, 0, SEEK_SET);
    while(1){
        sem_wait(&semaphore2);
        flag+=2;
        highbyte1++;
        lowbyte1++;
        if (scale_file && in0_raw_file)
        {
            fscanf(in0_raw_file, "%hu", &buffer_raw);

            //获取高八位和低八位
            pthread_mutex_lock(&high_byte_mutex);
            high_byte = (buffer_raw >> 8) & 0xFF;
            pthread_mutex_unlock(&high_byte_mutex);
            pthread_mutex_lock(&low_byte_mutex);
            low_byte = buffer_raw & 0xFF;
            pthread_mutex_unlock(&low_byte_mutex);
            
            if(flag_2%2==0)
            {
                pthread_mutex_lock(&flag_mutex);
                buffer1[flag-2]=high_byte;
                buffer1[flag-1]=low_byte;
                pthread_mutex_unlock(&flag_mutex);
            }else{
                pthread_mutex_lock(&flag_mutex);
                buffer2[flag-2]=high_byte;
                buffer2[flag-1]=low_byte;    
                pthread_mutex_unlock(&flag_mutex);   
            }
        }
        if(flag==BUFFER_SIZE)
        {
            pthread_mutex_lock(&flag_mutex);
            flag=0;
            pthread_mutex_unlock(&flag_mutex);
            sem_post(&semaphore);
            highbyte1=0;
            lowbyte1=0;
            flag_2++;
        }
        if(flag>BUFFER_SIZE)
        {
           printf("out");
        }
    }
}

void timer_handler(int sig, siginfo_t *si, void *uc)
{
    //send(sockfd, buffer1, BUFFER_SIZE, 0);
    sem_post(&semaphore2);
}

int main(void){
    
    buffer1 = (uint8_t*)malloc(BUFFER_SIZE * sizeof(uint8_t));
    buffer2 = (uint8_t*)malloc(BUFFER_SIZE * sizeof(uint8_t));  

    scale_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage_scale", "r");
    in0_raw_file = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
    sd_file = fopen("/mnt/sdcard/data.txt", "w");
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
    
    sem_init(&semaphore, 0, 0);
    sem_init(&semaphore2, 0, 0);
    
    struct sigevent sev;
    struct itimerspec its;
    timer_t timerid;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    signal(SIGRTMIN, timer_handler);

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = TIMER_INTERVAL_US * 1000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = TIMER_INTERVAL_US * 1000;

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

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
    {
        perror("timer_create");
        exit(1);
    }

    if (timer_settime(timerid, 0, &its, NULL) == -1)
    {
        perror("timer_settime");
        exit(1);
    }

    pthread_create(&ADCID, NULL, read_ADC_values, NULL);
    pthread_create(&UDPSendID, NULL, udpsend, NULL);
    pthread_create(&SDSaveID, NULL, sdsave, NULL);
    pthread_create(&LoopbackID, NULL, switch_work_mode, NULL);

    
    printf("ADC thread created\r\n");
    while(1)
    {
        sleep(1);
    }
}
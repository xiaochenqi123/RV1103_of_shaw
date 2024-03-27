// 接收来自上位机的数据包
#include "udpsend.h"
#include "adc.h"
#include "sd.h"
#include "main.h"
    extern int sockfd;
    extern uint16_t flag;
    extern uint8_t WORKMODE;
    extern uint8_t timer_flag;
    extern uint16_t flag_2;
    extern sem_t semaphore;
    extern sem_t adc_sd;
    extern pthread_mutex_t flag_mutex;

    extern uint8_t buffer1[BUFFER_SIZE];
    extern uint8_t buffer2[BUFFER_SIZE];
    extern sem_t udp_clode;                //用于关闭UDP发送线程的信号量
void udpsend(void)
{
    if(WORKMODE == UDPMODE)
    {
        while(1){
            if (sem_wait(&semaphore)==0) {
        
            pthread_mutex_lock(&flag_mutex);
            flag = 0;
            pthread_mutex_unlock(&flag_mutex);
                if (flag_2 % 2 == 0) {
                    ssize_t bytes_sent = send(sockfd, buffer1, BUFFER_SIZE, 0);
                    pthread_mutex_lock(&flag_mutex);
                    flag = 0;
                    pthread_mutex_unlock(&flag_mutex);
                    //printf("send buffer_1\r\n");
                    flag_2++;
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                    
                } else {
                    ssize_t bytes_sent = send(sockfd, buffer2, BUFFER_SIZE, 0);
                    pthread_mutex_lock(&flag_mutex);
                    flag = 0;
                    pthread_mutex_unlock(&flag_mutex);
                    //printf("send buffer_2\r\n");
                    flag_2--;
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                    
                }
                if (WORKMODE != UDPMODE) {

                break; // Exit the loop if WORKMODE is no longer SDMODE
            }
            }
            // if(sem_wait(&udp_clode)==0)
            // {
            //     pthread_exit(NULL); // 以默认退出状态退出线程
            // }
        }
           // usleep(1000); // sleep for 1ms to prevent busy waiting
        }else{
            pthread_exit(NULL);
        }

}
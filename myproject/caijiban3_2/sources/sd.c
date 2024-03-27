#include "sd.h"
#include "udpsend.h"
#include "adc.h"
#include "main.h"

extern uint8_t WORKMODE;
extern uint16_t flag_2;
extern uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区
extern uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
extern pthread_mutex_t flag_mutex;
extern uint16_t flag;
extern sem_t semaphore;
extern sem_t adc_sd;
extern FILE *sd_file;
extern sem_t sd_close;                 // 用于关闭SD卡线程的信号量
void sdsave(void)
{
   
  if(WORKMODE == SDMODE){
    while(1){
            if (sem_wait(&semaphore)==0) {
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
    
    }else{
        pthread_exit(NULL);
    }

}
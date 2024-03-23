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
extern FILE *sd_file;
extern sem_t sd_close;                 // 用于关闭SD卡线程的信号量
void sdsave(void)
{
   
  if(WORKMODE == SDMODE){
    while(1){
        
       
            if (sem_wait(&semaphore)==0) {
            
            pthread_mutex_lock(&flag_mutex);
            flag = 0;
            pthread_mutex_unlock(&flag_mutex);
               
                if (flag_2 % 2 == 0) {
                fwrite(buffer1, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                
                pthread_mutex_lock(&flag_mutex);
                flag = 0;
                pthread_mutex_unlock(&flag_mutex);
                
                flag_2++;
                printf("buffer1 write success\r\n");
            } else {
                fwrite(buffer2, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                pthread_mutex_lock(&flag_mutex);
                flag = 0;
                pthread_mutex_unlock(&flag_mutex);
                flag_2--;
                printf("buffer2 write success\r\n");
            }
               
            }
            // if(sem_wait(&sd_close)==0)
            // {
            //     pthread_exit(NULL); // 以默认退出状态退出线程
            // }
        
        }
    
    }

}
#include "adc.h"
#include "udpsend.h"
#include "main.h"

extern uint16_t flag;
extern uint16_t flag_2;
extern uint8_t timer_flag;
extern buffer1[BUFFER_SIZE];
extern buffer2[BUFFER_SIZE];
extern pthread_mutex_t flag_mutex;
extern sem_t semaphore;
extern sem_t semaphore2;
extern uint16_t buffer_raw;

extern sem_t adc_close;                // 用于关闭ADC采集线程的信号量
void read_ADC_values() {
            
            fseek(scale_file, 0, SEEK_SET);
            fseek(in0_raw_file, 0, SEEK_SET);
        while(1){
        if(sem_wait(&semaphore2)==0)
        {
           

             if (scale_file && in0_raw_file)
            {
                pthread_mutex_lock(&flag_mutex);
                flag += 2;
                
                 pthread_mutex_unlock(&flag_mutex);
               
                // fgets(buffer1, BUFFER_SIZE, scale_file);
                // float scale = strtof(buffer1, NULL);

                fscanf(in0_raw_file, "%hu", &buffer_raw);

                // //获取高八位和低八位
                uint8_t high_byte = (buffer_raw >> 8) & 0xFF;
                uint8_t low_byte = buffer_raw & 0xFF;

                if (flag_2 % 2 == 0) {
                    buffer1[flag - 2] = high_byte;
                    buffer1[flag - 1] = low_byte;
                } else {
                    buffer2[flag - 2] = high_byte;
                    buffer2[flag - 1] = low_byte;
                    }
             
                if(flag==BUFFER_SIZE)
                {
                    sem_post(&semaphore);
                }
                if(flag>1400)
                {
                     pthread_mutex_lock(&flag_mutex);
                flag = 0;
                
                 pthread_mutex_unlock(&flag_mutex);
                }
                if(WORKMODE == NONE)
                {
                    pthread_mutex_lock(&flag_mutex);
                    flag = 0;
                
                  pthread_mutex_unlock(&flag_mutex);
                }

                    
            }
            // if(sem_wait(&adc_close)==0)
            // {
            //     pthread_exit(NULL); // 以默认退出状态退出线程
            // }
        }
  
    }

    
}
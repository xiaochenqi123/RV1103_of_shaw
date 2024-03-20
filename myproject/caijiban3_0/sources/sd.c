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
void sdsave(void)
{
   
  if(WORKMODE == SDMODE){
    while(1){
        
       
            if (sem_wait(&semaphore)==0) {

            if (flag_2 % 2 == 0) {
                fwrite(buffer1, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                flag_2++;
                printf("buffer1 write success\r\n");
            } else {
                fwrite(buffer2, sizeof(uint8_t), BUFFER_SIZE, sd_file);
                flag_2--;
                printf("buffer2 write success\r\n");
            }
               
            }
        
        }
    
    }

}
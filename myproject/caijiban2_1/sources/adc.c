#include "adc.h"
#include "udpsend.h"

extern uint16_t flag;
extern uint16_t flag_2;
extern uint8_t timer_flag;
extern buffer1[BUFFER_SIZE];
extern buffer2[BUFFER_SIZE];
extern pthread_mutex_t flag_mutex;

void read_ADC_values() {
 while(1){
        if(timer_flag==1)
        {
            timer_flag=0;
            fseek(scale_file, 0, SEEK_SET);
            fseek(in0_raw_file, 0, SEEK_SET);

            if (scale_file && in0_raw_file)
            {
                pthread_mutex_lock(&flag_mutex);
                flag += 2;
              //  printf("flag=%d\r\n",flag);
                pthread_mutex_unlock(&flag_mutex);

                if(flag<=BUFFER_SIZE) {
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
                else{
                    pthread_mutex_lock(&flag_mutex);
                    flag = 0;
                    
                    pthread_mutex_unlock(&flag_mutex);
                }
            }
        }
  
    }

    
}

    


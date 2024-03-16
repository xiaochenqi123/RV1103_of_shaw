// #include "sd.h"
// #include "udpsend.h"
// #include "adc.h"


// extern uint8_t WORKMODE;
// extern uint16_t flag_2;
// extern uint8_t buffer1[BUFFER_SIZE];  // 第一个缓冲区
// extern uint8_t buffer2[BUFFER_SIZE];  // 第二个缓冲区
// extern pthread_mutex_t flag_mutex;
// extern uint16_t flag;

// void sdsave(void)
// {
//     pthread_mutex_lock(&flag_mutex);
//     FILE *sd_file;
//     sd_file = fopen("/mnt/sdcard/test.txt", "a");
//     if (sd_file == NULL)
//     {
//         perror("Error opening SD card file");
//         exit(1);
//     }
//     while(1){
        
//         if(flag==BUFFER_SIZE){
//             if (WORKMODE == SDMODE) {

//             if (flag_2 % 2 == 0) {
//                 fwrite(buffer1, sizeof(uint8_t), BUFFER_SIZE, sd_file);
//                 printf("write success\r\n");
//             } else {
//                 fwrite(buffer2, sizeof(uint8_t), BUFFER_SIZE, sd_file);
//             }
//                 fclose(sd_file);
//             }
//             flag = 0;
//             pthread_mutex_unlock(&flag_mutex);
//         }
//         pthread_mutex_unlock(&flag_mutex);
//     }

// }
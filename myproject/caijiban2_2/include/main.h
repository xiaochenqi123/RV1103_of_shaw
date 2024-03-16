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


pthread_t LoopbackID;  // udp回环线程
pthread_t TimerID;     // 定时器线程
pthread_t UDPSendID;   // UDP发送线程
pthread_t SDSaveID;    // SD卡存储线程
pthread_t ADCID;       // ADC采集线程


#define HOST "192.168.1.100"

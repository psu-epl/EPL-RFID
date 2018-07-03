// Functions, prototypes, defines, includes, etc.
#include <stdio.h>
#include <pigpio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int init();
void cleanup();
int read_station_info(char* station_id);
void* read_125(void* cardNo);


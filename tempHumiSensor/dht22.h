#ifndef DHT22_H
#define DHT22_H

#include <stdio.h>
#include <stdlib.h>

#include "../wiringPiService/wiringPiController.h"
#include "../globalConfig.h"

typedef struct {
    float humidity;
    float temperature;
    char state;
} Result;

char setupDHT22(unsigned char pinValue);
char handShake();
char read_dht_data(float *humidity, float *temperature);
Result getTempHumiData();
char initializeValidationGuard();

#endif 

#ifndef WIRINGPI_CONTROLLER_H
#define WIRINGPI_CONTROLLER_H

#include <stdio.h>
#ifdef WIRING_PI
#include <wiringPi.h>
#define LOW_VAL     LOW
#define HIGH_VAL    HIGH
#define OUTPUT_VAL  OUTPUT
#define	INPUT_VAL   INPUT
#else
#define LOW_VAL     0
#define HIGH_VAL    1
#define	OUTPUT_VAL  1
#define	INPUT_VAL   1
#endif

#ifdef WIRING_PI_I2C
#include <wiringPiI2C.h>
#endif

#include "../globalConfig.h"



typedef struct {
    int fileDescriptior;
    char isSuccess;
} I2CSetupResult;

char setupGPIO();
I2CSetupResult setupI2C(unsigned short address);
void writeI2CReg(short fd, char reg, char value);

#endif

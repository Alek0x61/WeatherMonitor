#include "wiringPiController.h"

static char gpioSetupDone = 0;
static char i2cSetupDone = 0;

static I2CSetupResult result;

char setupGPIO() {
    if (gpioSetupDone) {
        printf("GPIO is already set up.\n");
        return SUCCESS;
    }

    if (wiringPiSetupGpio() == -1) {
        printf("Failed to set up GPIO!\n");
        return ERROR;
    }

    gpioSetupDone = 1;
    printf("GPIO setup complete.\n");
    return SUCCESS;
}

I2CSetupResult setupI2C(unsigned short address) {
    if (i2cSetupDone) {
        printf("I2C is already set up.\n");
        return result;
    }
    
    short fd = wiringPiI2CSetup(address);
    if (fd == -1) {
        printf("Failed to set up I2C!\n");
        result.isSuccess = ERROR;
        return result;
    }

    i2cSetupDone = 1;

    result.isSuccess = SUCCESS;
    result.fileDescriptior = fd;
    printf("I2C setup complete.\n");
    return result;
}

void writeI2CReg(short fd, char reg, char value) {
    wiringPiI2CWriteReg8(fd, reg, value);
}

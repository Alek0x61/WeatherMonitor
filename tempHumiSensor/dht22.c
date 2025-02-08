//https://www.waveshare.com/wiki/DHT22_Temperature-Humidity_Sensor

#include "dht22.h"

#define TIMEOUT 80

unsigned char pin;
float temperatureGuardData;

char setupDHT22(unsigned char pinValue) {
    if (setupGPIO() == ERROR) {
        printf("WiringPi setup failed!\n");
        return ERROR;
    }

    if (pinValue < 1) {
        printf("SDA Pin is not set");
        return ERROR;
    }

    pin = pinValue;
    return SUCCESS;
}

char handShake() {
    pinMode(pin, OUTPUT_VAL); // Send the OUTPUT_VAL signal to the sensor to indicate the Raspberry Pi's readiness to send data
    digitalWrite(pin, LOW_VAL); // Send a LOW_VAL signal for 18 milliseconds to the sensor to indicate the Raspberry Pi's readiness to read the data
    delay(18);

    pinMode(pin, INPUT_VAL);

    unsigned long start_time = micros();

    // Wait for the sensor to pull the line LOW_VAL
    while (digitalRead(pin) == HIGH_VAL) {
        if (micros() - start_time > TIMEOUT) {
            printf("Timeout waiting for LOW_VAL signal\n");
            return ERROR;
        }
    }

    // Wait for exactly 80µs while the line is LOW_VAL
    start_time = micros();
    while (micros() - start_time < TIMEOUT) {
        if (digitalRead(pin) == HIGH_VAL) {
            break;
        }
    }

    // Wait for the sensor to pull the line HIGH_VAL
    start_time = micros();
    while (digitalRead(pin) == LOW_VAL) {
        if (micros() - start_time > TIMEOUT) {
            printf("Timeout waiting for HIGH_VAL signal\n");
            return ERROR;
        }
    }

    // Wait for exactly 80µs while the line is HIGH_VAL
    start_time = micros();
    while (micros() - start_time < TIMEOUT) {
        if (digitalRead(pin) == LOW_VAL) {
            break;
        }
    }

    // Wait for the sensor to pull the line LOW_VAL again
    start_time = micros();
    while (digitalRead(pin) == HIGH_VAL) {
        if (micros() - start_time > TIMEOUT) {
            printf("Timeout waiting for LOW_VAL signal\n");
            return ERROR; 
        }
    }

    // Wait for exactly 80µs while the line is LOW_VAL
    start_time = micros();
    while (micros() - start_time < TIMEOUT) {
        if (digitalRead(pin) == HIGH_VAL) {
            break; 
        }
    }

    return SUCCESS;
}

char read_dht_data(float *humidity, float *temperature) {
    unsigned char data[5] = {0, 0, 0, 0, 0};

    if (handShake() == ERROR)
        return ERROR;

    // The loop iterates over 'i', which represents the byte being read. The DHT22 sensor returns 5 bytes (40 bits), so the loop runs 5 times.

    // Another loop iterates over 'j', which represents the bit within each byte. 
    // If the time that the signal stays HIGH_VAL is longer than 40µs, the current bit is set to 1.
    // If the signal stays HIGH_VAL for a shorter time, the bit remains 0.

    // Example: If j = 7, the byte in data[i] would look like this: 10000000. 
    // Then, if j = 2, the byte would look like this: 10000100.
    for (int i = 0; i < 5; i++) {
        for (int j = 7; j >= 0; j--) {
            unsigned long HIGH_VAL_duration = 0; // Variable to store how long the pin stays HIGH_VAL
            unsigned long start_time = 0;    // Start time for HIGH_VAL duration

            while (digitalRead(pin) == LOW_VAL) {}

            start_time = micros();

            while (digitalRead(pin) == HIGH_VAL) {
                
                HIGH_VAL_duration = micros() - start_time;  // Calculate the duration the pin has been HIGH_VAL
                if (HIGH_VAL_duration > 200)
                    return ERROR;
            }

            if (HIGH_VAL_duration > 40) { // If the HIGH_VAL pulse duration is more than 40µs, it's a '1'
                data[i] |= (1 << j);
            }
        }
    }

    *humidity = (data[0] << 8) | data[1]; // Humidity is in the first 2 bytes
    *temperature = (data[2] << 8) | data[3]; // Temperature is in the next 2 bytes

    if (data[2] == 0x80) {
        *temperature *= -1;
    }

    // Checksum (last byte)
    int checksum = data[4];
    int calculated_checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != calculated_checksum) {
        return ERROR;
    }

    return SUCCESS;
}

Result getTempHumiData() {
    float humidity = 0, temperature = 0;
    Result result;
    result.state = SUCCESS;

    if (read_dht_data(&humidity, &temperature) == 0) {
        result.temperature = temperature / 10;
        result.humidity = humidity / 10;

        if (result.temperature > temperatureGuardData + 5 || result.temperature < temperatureGuardData - 5) {
            result.state = ERROR;
        }
        else {
            temperatureGuardData = result.temperature;
        }
    }
    else {
        result.state = ERROR;
    }
    
    return result;
}

char initializeValidationGuard() {
    float humidity = 0, temperature = 0;

    int tempData[10];

    int tempDataIndex = 0;
    for (int i = 0; i < 10; i++) {
       if (read_dht_data(&humidity, &temperature) == 0) {
            tempData[tempDataIndex] = temperature / 10;
            tempDataIndex++;
        }
        delay(500);
    }    

    int maxCount = 0, maxNum = tempData[0];

    for (int i = 0; i < 10; i++) {
        int count = 0;
        for (int j = 0; j < 10; j++) {
            if (tempData[j] == tempData[i]) {
                count++;
            }
        }
        if (count > maxCount) {
            maxCount = count;
            maxNum = tempData[i];
        }
    }
    temperatureGuardData = maxNum;
    
    return SUCCESS;
}
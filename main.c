#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#include "http.c"
#include "tempHumiSensor/dht22.h"
#include "display/SSD1306.h"
#include "wiringPiService/wiringPiController.h"
#include "repository/weatherData.h"

char temperature[20];
char humidity[20];

char* queryAllData(QueryParam* queryParam) {
    WeatherViewResult WeatherViewResult = queryAllWeatherData(queryParam);
    WeatherView* weatherView = WeatherViewResult.weatherView;

    if (WeatherViewResult.rowCount == 0) {
        if (weatherView) {
            free(weatherView);
        }
        return NULL;
    }

    size_t bufferSize = WeatherViewResult.rowCount * 85;
    char* buffer = malloc(bufferSize);
    if (!buffer) {
        perror("Failed to allocate memory for queryAllData buffer");
        free(weatherView);
        return NULL;
    }
    int offset = 0;
    buffer[offset++] = '[';

    for (size_t i = 0; i < WeatherViewResult.rowCount; i++)
    {
        int requiredSizeEstimate = snprintf(
            NULL, 
            0, 
            "%s{ \"timestamp\": \"%s\", \"temperature\": \"%s\", \"humidity\": \"%s\" }", 
            (i == 0) ? "" : ",", weatherView[i].timestamp, weatherView[i].temperature, weatherView[i].humidity
        );

        if (offset + requiredSizeEstimate >= bufferSize) {
            bufferSize = offset + requiredSizeEstimate;
            char* temp = realloc(buffer, bufferSize);
            if (!temp) {
                free(weatherView);
                free(buffer);
                return NULL; 
            }
            buffer = temp;
        }

        int written = snprintf(
            buffer + offset, bufferSize - offset,
            "%s{ \"timestamp\": \"%s\", \"temperature\": \"%s\", \"humidity\": \"%s\" }",
            (i == 0) ? "" : ",",
            weatherView[i].timestamp,
            weatherView[i].temperature,
            weatherView[i].humidity
        );
        offset += written;
    }
    buffer[offset++] = ']';
    buffer[offset] = '\0'; 
    free(weatherView);
    return buffer;
}

void* extractQueryParams(char* query, short length) {
    QueryParam* queryParam = malloc(sizeof(QueryParam));
    if (!queryParam) {
        perror("Failed to allocate memory for queryParam");
        return NULL;
    }

    unsigned int capacity;

    char* capacityStrKey = "/?capacity=";
    char* capacityStr = strstr(query, capacityStrKey);
    if (capacityStr) { 
        capacityStr += strlen(capacityStrKey);
        if (sscanf(capacityStr, "%d", &capacity) == 0) {
            capacity = 100;
        }
    }
    queryParam->capacity = capacity;
    return queryParam;
}

Endpoint* setEndpoints() {
    Endpoint *endpoints = allocEndpoints();
    if (!endpoints) 
        return NULL;
    
    strcpy(endpoints->route, "^/[\?]capacity=[0-9]+");
    endpoints->endpointFunc = (EndpointFunc)queryAllData;
    endpoints->extractQueryParams = (ExtractQueryParams)extractQueryParams;
    return endpoints;
}

void handle_sigint(int sig) {
    closeDatabaseConnection();
    closeHttpConnection();
    exit(0);
}

int setupDevices() {
    signed char pin = 27;

    if (setupDHT22(pin) == ERROR) {
        printf("Error during DHT22 setup\n");
        exit(1);
    }
    printf("Pin %d set up successfully\n", pin);

    if (setupSSD1306() == ERROR) {
        printf("Error during SSD1306 setup\n");
        exit(1);
    }
    printf("SSD1306 set up successfully\n");

    if (initializeValidationGuard() == ERROR) {
        printf("Error during validation guard setup\n");
        exit(1);
    }
    printf("Validated guard set up successfully\n");

    return SUCCESS;
}

void processData() {
    while(1) {
        Result result = getTempHumiData();
        if (result.state == SUCCESS) {
            insertWeatherData(result.temperature, result.humidity);

            sprintf(temperature, "temp = %.1f *C", result.temperature);
            sprintf(humidity, "humi = %.1f %%", result.humidity);

            displayOff();
            printOnCoordinates(temperature, 0, 10);
            printOnCoordinates(humidity, 0, 40);
            displayOn();
        }
        sleep(60); //60 seconds
    }
}

void* processDataThread(void* arg) {
    processData();
    return NULL;
}

int main() {
    signal(SIGINT, handle_sigint);
    if (initDatabaseConnection(DB_PATH) != SQLITE_OK) {
        return ERROR;
    }
    
    if(initDatabaseTable() == ERROR) {
        return ERROR;
    }

    if (setupDevices() == ERROR) {
        return ERROR;
    }
    printf("\nStarting data gathering\n");
    pthread_t dataThread;
    if (pthread_create(&dataThread, NULL, processDataThread, NULL) != 0) {
        fprintf(stderr, "Error creating data collection thread\n");
        return ERROR;
    }

    Endpoint *endpoints = setEndpoints();

    if (runServer(endpoints) == ERROR) {
        return ERROR;
    }

    pthread_join(dataThread, NULL);
    free(endpoints);
    return SUCCESS;
}
#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#define DB_PATH "/var/lib/weather.db"

#include <stdlib.h>
#include <string.h>

#include "connection.h"

typedef struct {
    char humidity[20];
    char temperature[20];
    char timestamp[32];
} WeatherView;

typedef struct {
    WeatherView *weatherView;
    unsigned int rowCount;
} WeatherViewResult;

typedef struct {
    WeatherView *data;
    unsigned int index;
    unsigned int capacity;
} WeatherViewContext;

typedef struct {
    unsigned int capacity;
} QueryParam;

int callback(void *NotUsed, int argc, char **argv, char **azColName);
int insertWeatherData(float temperature, float humidity);
WeatherViewResult queryAllWeatherData(QueryParam* queryParam);

#endif

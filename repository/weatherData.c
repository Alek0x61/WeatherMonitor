#include "weatherData.h"

unsigned int rowCount = 0;

int callback(void *context, int argc, char **argv, char **azColName) {
    rowCount++;
    WeatherViewContext *weatherCtx = (WeatherViewContext *)context;
    
    if (weatherCtx->index >= weatherCtx->capacity) {
        return 0;
    }

    WeatherView *entry = &weatherCtx->data[weatherCtx->index];

    strncpy(entry->temperature, argv[2] ? argv[2] : "0.0", sizeof(entry->temperature) - 1);
    strncpy(entry->humidity, argv[3] ? argv[3] : "0.0", sizeof(entry->humidity) - 1);
    strncpy(entry->timestamp, argv[1] ? argv[1] : "NULL", sizeof(entry->timestamp) - 1);

    entry->temperature[sizeof(entry->temperature) - 1] = '\0';
    entry->humidity[sizeof(entry->humidity) - 1] = '\0';
    entry->timestamp[sizeof(entry->timestamp) - 1] = '\0';

    weatherCtx->index++;
    return 0;
}


int insertWeatherData(float temperature, float humidity) {
    sqlite3 *db = getDatabaseConnection();
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO WeatherData (temperature, humidity) VALUES (%.2f, %.2f);", temperature, humidity);
    return executeQuery(db, sql, 0, 0);
}

WeatherViewResult queryAllWeatherData(QueryParam* queryParam) {
    WeatherViewResult result;
    rowCount = 0;

    unsigned int capacity = queryParam->capacity;
    free(queryParam);
    
    sqlite3 *db = getDatabaseConnection();

    WeatherView *weatherView = NULL;
    if (capacity > 0) {
        weatherView = malloc(capacity * sizeof(WeatherView));
        if (!weatherView) {
            perror("Failed to allocate memory for weatherView");
            result.weatherView = NULL;
            return result;
        }
    }

    WeatherViewContext context = { weatherView, 0, capacity };

    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, timestamp, temperature, humidity FROM WeatherData ORDER BY timestamp DESC LIMIT %d;", capacity);
    executeQuery(db, sql, callback, &context);

    result.rowCount = rowCount;
    result.weatherView = weatherView;

    return result;
}
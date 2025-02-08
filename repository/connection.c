#include "connection.h"

static sqlite3 *db = NULL;

char initDatabaseConnection(char* connectionString) {
    if (db != NULL) {
        return SUCCESS;
    }

    char rc = sqlite3_open(connectionString, &db);
    if (rc) {
        fprintf(stderr, "Can't open database at %s: %s\n", connectionString, sqlite3_errmsg(db));
        return ERROR;
    }
    return SUCCESS;
}

char initDatabaseTable() {
    if (db == NULL) {
        return ERROR;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS WeatherData ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "temperature REAL NOT NULL,"
                "humidity REAL NOT NULL"
                ");";

    char *errMsg = 0;
    int rcCreateTable = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rcCreateTable != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ERROR;
    }
    printf("Table created successfully\n");

    return SUCCESS;
}

sqlite3* getDatabaseConnection() {
    return db;
}

void closeDatabaseConnection() {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
        printf("Database connection closed\n");
    }
}

char executeQuery(sqlite3 *db, const char *sql, int (*callback)(void*, int, char**, char**), void *context) {
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, context, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ERROR;
    }
    return SUCCESS;
}
#ifndef CONNECTION_H
#define CONNECTION_H

#include <sqlite3.h>
#include <stdio.h>

#include "../globalConfig.h"

char initDatabaseConnection(char* connectionString);
char initDatabaseTable();
sqlite3* getDatabaseConnection();
void closeDatabaseConnection();
char executeQuery(sqlite3 *db, const char *sql, int (*callback)(void*, int, char**, char**), void *context);

#endif 

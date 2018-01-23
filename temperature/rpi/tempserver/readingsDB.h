#ifndef READINGSDB_H
#define READINGSDB_H

struct sqlite3;
struct sqlite3_stmt;

struct TempReading;

bool openDatabase(sqlite3 **db, const char *fullDbPath);
bool executeStatement(sqlite3 *db, const char *stmt);
void initializeDatabase(sqlite3 *db);
bool createTemperatureTable(sqlite3 *db);
sqlite3_stmt *prepareInsertTempStmt(sqlite3 *db);
bool insertTemperature(sqlite3 *db, sqlite3_stmt *stmt, const TempReading &tr);
void closeDatabase(sqlite3 *db);

#endif // READINGSDB_H











#include "readingsDB.h"
#include <sqlite3.h>
#include "common.h"
#include "tempreading.h"

bool openDatabase(sqlite3 **db, const char *fullDbPath) {
    bool anyError = false;
    int flags = 0;
    flags |= SQLITE_OPEN_READWRITE;
    flags |= SQLITE_OPEN_CREATE;
    flags |= SQLITE_OPEN_FULLMUTEX;
    
    const int result = sqlite3_open_v2(fullDbPath, db, flags, nullptr);
    if (result != SQLITE_OK) {
        log("Opening %s failed! Error: %s", fullDbPath, sqlite3_errstr(result));
        anyError = true;
        
        closeDatabase(*db);
    }

    return anyError;
}

bool executeStatement(sqlite3 *db, const char *stmt) {
    int rc = sqlite3_exec(db, stmt, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        log("Failed to execute (%s). Error: %s", stmt, sqlite3_errstr(rc));
    }
    
    bool success = rc == SQLITE_OK;
    return success;
}

void initializeDatabase(sqlite3 *db) {
    executeStatement(db, "PRAGMA auto_vacuum = 0;");
    executeStatement(db, "PRAGMA cache_size = -20000;");
    executeStatement(db, "PRAGMA case_sensitive_like = true;");
    executeStatement(db, "PRAGMA encoding = \"UTF-8\";");
    executeStatement(db, "PRAGMA journal_mode = WAL;");
    executeStatement(db, "PRAGMA locking_mode = NORMAL;");
    executeStatement(db, "PRAGMA synchronous = NORMAL;");
}

bool createTemperatureTable(sqlite3 *db) {
    const char *stmtText = "CREATE TABLE IF NOT EXISTS Temps (time INTEGER, sensorID INTEGER, temperature REAL);";
    bool success = executeStatement(db, stmtText);
    return success;
}

sqlite3_stmt *prepareInsertTempStmt(sqlite3 *db) {
    const char *stmtText = "INSERT INTO Temps (time, sensorID, temperature) VALUES (strftime('%s', 'now'), ?, ?)";
    sqlite3_stmt *insertStatement = 0;
    int rc = sqlite3_prepare_v2(db, stmtText, -1, &insertStatement, 0);
    if (rc != SQLITE_OK) {
        log("Failed to prepare statement (%s). Error: %s", stmtText, sqlite3_errstr(rc));
    }

    return insertStatement;
}

bool insertTemperature(sqlite3 *db, sqlite3_stmt *stmt, const TempReading &tr) {
    int rc = 0;
    bool success = false;

    do {
        rc = sqlite3_bind_int(stmt, 1, tr.m_SensorID);
        if (rc != SQLITE_OK) {
            log("Failed to bind sensorID");
            break;
        }

        rc = sqlite3_bind_double(stmt, 2, tr.m_Temperature);
        if (rc != SQLITE_OK) {
            log("Failed to bind temperature");
            break;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            log("Failed to INSERT values");
            break;
        }

        success = true;
    } while (false);

    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    return success;
}

void closeDatabase(sqlite3 *db) {
    const int closeResult = sqlite3_close(db);
    if (closeResult != SQLITE_OK) {
        log("Closing database failed! Error: %s", sqlite3_errstr(closeResult));
    }
}













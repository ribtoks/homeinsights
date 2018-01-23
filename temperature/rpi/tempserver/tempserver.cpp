#include "../../../vendors/rc-switch/RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdargs.h>
#include <sqlite3.h>

#define HEADER 0x2D
#define TEMP_MIN -30.0f
#define TEMP_STEP 0.0049438476f

typedef unsigned int uint;

RCSwitch tempSwitch;

int log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return printf(fmt, args);
}

void handleReading(unsigned int value) {
  bool success = false;
  float temperature = 0.0f;
  const uint originalValue = value;

  do {
    uint receivedChecksum = value & 0x3F;
    value >>= 6;
  
    uint tempCode = value & 0x3FFF;
    value >>= 14;

    uint id = value & 0x3F;
    value >>= 6;

    uint header = value & 0x3F;
  
    if (header != HEADER) {
      log("Protocol violation: header mismatch. Received: %u\n", header);
      break;
    }

    uint checksum = tempCode;
    checksum ^= id;
    checksum ^= header;
    checksum ^= (tempCode >> 8);
    checksum = checksum & 0x3F;

    if (checksum != receivedChecksum) {
      log("Protocol violation: checksum mismatch");
      break;
    } else {
      log("Temp code: %u\n", tempCode);
    }

    temperature = TEMP_MIN + TEMP_STEP * (float)tempCode;

    success = true;
  } while (false);

  if (success) {
    log("Received temperature: %.6f\n", temperature);
  } else {
    log("Failed to process something: %u\n", originalValue);
  }
}

void openDatabase(sqlite3 **db, const char *fullDbPath) {
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
}

bool executeStatement(sqlite *db, const char *stmt) {
    bool success = false;
    int rc = sqlite3_exec(db, stmt, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        log("Failed to execute (%s). Error: %s", stmt, sqlite3_errstr(rc));
    }
    
    bool success = rc == SQLITE_OK;
    return success;
}

void initializeDatabase(sqlite3 *db) {
    executeStatement("PRAGMA auto_vacuum = 0;");
    executeStatement("PRAGMA cache_size = -20000;");
    executeStatement("PRAGMA case_sensitive_like = true;");
    executeStatement("PRAGMA encoding = \"UTF-8\";");
    executeStatement("PRAGMA journal_mode = WAL;");
    executeStatement("PRAGMA locking_mode = NORMAL;");
    executeStatement("PRAGMA synchronous = NORMAL;");
}

void closeDatabase(sqlite3 *db) {
    const int closeResult = sqlite3_close(m_Database);
    if (closeResult != SQLITE_OK) {
        log("Closing database failed! Error: %s", sqlite3_errstr(result));
    }
}

int main(int argc, char *argv[]) {
  if (wiringPiSetup() < 0) {
    log("Failed to initialize wiringPi\n");
    return 1;
  } else {
    log("WiringPi is initialized\n");
  }

  const int PIN = 2;
  tempSwitch = RCSwitch();
  tempSwitch.enableReceive(PIN);
  log("Listening on pin %d...\n", PIN);

  sqlite3 *db;
    
  while (1) {
    if (tempSwitch.available()) {
      int value = tempSwitch.getReceivedValue();
      tempSwitch.resetAvailable();

      handleReading((unsigned int)value);
    }

    fflush(stdout);
  }

  return 0;
}

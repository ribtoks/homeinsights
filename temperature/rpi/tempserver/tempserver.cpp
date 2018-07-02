#include "../../../vendors/rc-switch/RCSwitch.h"
#include <stdio.h>
#include <ctime>
#include "common.h"
#include "readingsDB.h"
#include "tempreading.h"

#define HEADER 0x2D
#define TEMP_MIN -40.0
// #define TEMP_MAX 80
// (TEMP_MAX - TEMP_MIN + 1)/(2^14)
#define TEMP_STEP 0.00738525391

#define DB_ERROR 3

typedef unsigned int uint;

RCSwitch tempSwitch;

bool tryReadMessage(unsigned int value, TempReading &reading) {
    bool success = false;
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
            log("Protocol violation: header mismatch. Received: %u", header);
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
        }

        if (tempCode == 0) {
            log("Skipping temperature code 0");
            break;
        }

        double temperature = TEMP_MIN + TEMP_STEP * (double)tempCode;
        log("Received temperature %.6f from sensor #%u", temperature, id);

        reading.m_SensorID = id;
        reading.m_Temperature = temperature;

        success = true;
    } while (false);

    if (!success) {
        log("Failed to process something: %u", originalValue);
    }

    return success;
}

void handleReading(sqlite3 *db, sqlite3_stmt *insertStatement, unsigned int value) {
    TempReading reading;

    if (tryReadMessage(value, reading)) {
        insertTemperature(db, insertStatement, reading);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        log("Path to DB is missing");
        return 1;
    }
    
    if (wiringPiSetup() < 0) {
        log("Failed to initialize wiringPi");
        return 2;
    } else {
        log("WiringPi is initialized");
    }

    const int IO_PIN = 2;
    tempSwitch = RCSwitch();
    tempSwitch.enableReceive(IO_PIN);
    log("Listening on pin %d...", IO_PIN);

    sqlite3 *db = nullptr;
    if (!openDatabase(&db, argv[1])) {
        log("Failed to open database %s", argv[1]);
        return DB_ERROR;
    } else {
        log("Successfully opened database %s", argv[1]);
    }

    initializeDatabase(db);
    if (!createTemperatureTable(db)) {
        log("Failed to create table in DB");
        return DB_ERROR;
    }

    sqlite3_stmt *insertStatement = 0;
    insertStatement = prepareInsertTempStmt(db);
    if (insertStatement == 0) {
        log("Failed to create INSERT statement");
        return DB_ERROR;
    }

    clock_t cbegin = std::clock();
    
    while (1) {
        if (tempSwitch.available()) {
            int value = tempSwitch.getReceivedValue();
            tempSwitch.resetAvailable();

            handleReading(db, insertStatement, (unsigned int)value);
        }

        clock_t cend = clock();
        int elapsed_secs = cend - cbegin;
        if (elapsed_secs > 10 * 60 * CLOCKS_PER_SEC) {
            log("Still listening...");
        }
    }

    return 0;
}

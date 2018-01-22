#include "../../vendors/rc-switch/RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

#define HEADER 0x2D
#define TEMP_MIN -30.0f
#define TEMP_STEP 0.0049438476f

typedef unsigned int uint;

RCSwitch tempSwitch;

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
      printf("Protocol violation: header mismatch. Received: %u\n", header);
      break;
    }

    uint checksum = tempCode;
    checksum ^= id;
    checksum ^= header;
    checksum ^= (tempCode >> 8);
    checksum = checksum & 0x3F;

    if (checksum != receivedChecksum) {
      printf("Protocol violation: checksum mismatch");
      break;
    } else {
      printf("Temp code: %u\n", tempCode);
    }

    temperature = TEMP_MIN + TEMP_STEP * (float)tempCode;

    success = true;
  } while (false);

  if (success) {
    printf("Received temperature: %.6f\n", temperature);
  } else {
    printf("Failed to process something: %u\n", originalValue);
  }
}

int main(int argc, char *argv[]) {
  if (wiringPiSetup() < 0) {
    printf("Failed to initialize wiringPi\n");
    return 1;
  } else {
    printf("WiringPi is initialized\n");
  }

  const int PIN = 2;
  tempSwitch = RCSwitch();
  tempSwitch.enableReceive(PIN);
  printf("Listening on pin %d...\n", PIN);
    
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

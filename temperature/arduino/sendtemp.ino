#include <RCSwitch.h>
#include <DHT.h>

// -----------------------------------------------------
// -------- change this section before flushing --------
// -----------------------------------------------------
#define SENSOR_ID 1
// #define MY_UNO
#define MY_MINI
// #define MY_DEBUG
// -----------------------------------------------------

RCSwitch tempSwitch = RCSwitch();

#define HEADER 0x2D
#define TEMP_STEP 0.0073852539f
#define TEMP_MIN -40.f
#define TEMP_MAX 80.f

#define DHTTYPE DHT22

#if defined(MY_UNO)
    #define DHT_PIN 2
    #define RADIO_PIN 8
    #define LED_PIN 13
#elif defined(MY_MINI)
    #define DHT_PIN 9
    #define RADIO_PIN 2
    #define LED_PIN 13
#endif

DHT dht(DHT_PIN, DHTTYPE);

unsigned int getTemperatureCode(float temperature) {
  float x = (temperature - TEMP_MIN)/TEMP_STEP;
  unsigned int code = (unsigned int)x;
  return code;
}

unsigned long createTemperatureMessage(byte id, float temperature) {
  unsigned long result = 0;

  // 6 bit header
  result |= HEADER;

  // 6 bit id
  result = result << 6;
  result |= (id & 0x3F);

  // 14 bits - temperature code
  const unsigned int tempCode = getTemperatureCode(temperature);
  result = result << 14;
  result |= tempCode & 0x3FFF;
  
  unsigned int checksum = tempCode;
  checksum ^= id;
  checksum ^= HEADER;
  checksum ^= (tempCode >> 8);
  
  // 6 bits checksum
  result = result << 6;
  result |= (checksum & 0x3F);

  return result;
}

void setup() {
  tempSwitch.enableTransmit(RADIO_PIN);
  dht.begin();

#ifdef MY_DEBUG
  pinMode(LED_PIN, OUTPUT);
#endif
}

void loop() {
  float temp = dht.readTemperature();
  
  unsigned long message = createTemperatureMessage(SENSOR_ID, temp);
  tempSwitch.send(message, 32);

#ifdef MY_DEBUG
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
#endif

#ifdef MY_DEBUG
  delay(5000);
#else
  delay(10*60000);
#endif
}

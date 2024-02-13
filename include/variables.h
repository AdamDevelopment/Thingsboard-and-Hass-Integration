#ifndef VARIABLES_H
#define VARIABLES_H
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>         
#include <DNSServer.h>
#include <WiFiManager.h>
#include <SHT3x.h>
#undef I2C_BUFFER_LENGTH
#include "MAX30105.h"
#include <Arduino.h>
#include <Wire.h>
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "time.h"
#include "esp_sntp.h"
#define JSON_CONFIG_FILE "/test_config.json"


extern const char* ntpServer1;
extern const char* ntpServer2;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const char* time_zone;

// mutexes
extern SemaphoreHandle_t mutex;
extern SemaphoreHandle_t ad8232mutex;

// Sensors addresses
extern SHT3x tempAndHumSensor;
extern MAX30105 PulseAndOxygenSensor;

// Light sleep variables
extern unsigned long lastMeasurementTime;
extern unsigned long lastSleepTime;
extern const unsigned long aqDuration;
extern const unsigned long sleepDuration;
extern bool sleepState;

// Global constants
extern const byte RATE_SIZE;
extern const int LO_PLUS_PIN;
extern const int LO_MINUS_PIN;
extern const int ECG_PIN;
extern const int SDN;

// Global variables
extern WiFiClientSecure espClient;
extern PubSubClient mqttClient;
extern WiFiManager wifiManager;
extern byte rates[];
extern byte rateSpot;
extern long lastBeat;
extern float bpm;
extern int beatAvg;

// misscelanous variables
extern const int IR_TRESHOLD;
extern const unsigned long SPO2_WAIT_TIME;


// definiowanie stanów pomiaru
enum  OXY_MEASURE_STATES {
  INIT,
  COLLECT,
  PROCESS,
  WAIT
};

// spo2 state calculation variables
extern OXY_MEASURE_STATES MAX30102_STATE;
// extern AD_MEASURE_STATES currState;


#define MAX_BRIGHTNESS 255

// sp2o variables
extern uint32_t irBuffer[100];  // infrared LED sensor data
extern uint32_t redBuffer[100]; // red LED sensor data

extern int32_t bufferLength;  // data length
extern int32_t spo2;          // SPO2 value
extern int8_t validSPO2;      // indicator to show if the SPO2 calculation is valid
extern int32_t heartRate;     // heart rate value
extern int8_t validHeartRate; // indicator to show if the heart rate calculation is valid
extern int sampleIndex;
extern unsigned long lastSampleTime;

// sp2o setup variables
extern byte ledBrightness; // Options: 0=Off to 255=50mA
extern byte sampleAverage; // Options: 1, 2, 4, 8, 16, 32
extern byte ledMode;       // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
extern byte sampleRate;    // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
extern int pulseWidth;     // Options: 69, 118, 215, 411
extern int adcRange;       // Options: 2048, 4096, 8192, 16384

#endif // VARIABLES_H

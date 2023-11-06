#ifndef VARIABLES_H
#define VARIABLES_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SHT3x.h>
#undef I2C_BUFFER_LENGTH
#include "MAX30105.h"
#include <Arduino.h>
#include <Wire.h>
#include "heartRate.h"
#include "spo2_algorithm.h"

// Sensors addresses
extern SHT3x tempAndHumSensor;
extern MAX30105 PulseAndSP2OSensor;

// Global constants
extern const char *SSID;
extern const char *PASSWORD;
extern const char *MQTT_SERVER;
extern const int MQTT_PORT;
extern const char *MQTT_TOKEN;
extern const byte RATE_SIZE;
extern const int LO_PLUS_PIN;
extern const int LO_MINUS_PIN;
extern const int ECG_PIN;
extern const int SDN;

// Global variables
extern WiFiClientSecure espClient;
extern PubSubClient mqttClient;
extern byte rates[];
extern byte rateSpot;
extern long lastBeat;
extern float bpm;
extern int beatAvg;

//misscelanous variables
extern const int IR_TRESHOLD;
extern const unsigned long SPO2_WAIT_TIME;
// defining spo2 measurement states
enum Spo2MeasurementState {
  INIT,
  COLLECTING,
  PROCESSING,
  WAITING
};

// spo2 state calculation variables
extern Spo2MeasurementState spo2State;
extern int sampleIndex;
extern unsigned long lastSampleTime;

#define MAX_BRIGHTNESS 255

// sp2o variables
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
extern uint16_t irBuffer[50];  // infrared LED sensor data
extern uint16_t redBuffer[50]; // red LED sensor data
#else
extern uint32_t irBuffer[50];  // infrared LED sensor data
extern uint32_t redBuffer[50]; // red LED sensor data
#endif

extern int32_t bufferLength;  // data length
extern int32_t spo2;          // SPO2 value
extern int8_t validSPO2;      // indicator to show if the SPO2 calculation is valid
extern int32_t heartRate;     // heart rate value
extern int8_t validHeartRate; // indicator to show if the heart rate calculation is valid

// sp2o setup variables
extern byte ledBrightness; // Options: 0=Off to 255=50mA
extern byte sampleAverage; // Options: 1, 2, 4, 8, 16, 32
extern byte ledMode;       // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
extern byte sampleRate;    // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
extern int pulseWidth;     // Options: 69, 118, 215, 411
extern int adcRange;       // Options: 2048, 4096, 8192, 16384

#endif // VARIABLES_H

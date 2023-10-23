#include <WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SHT3x.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// Global constants
const char *SSID = "Nie mam wi-fi";
const char *PASSWORD = "makaronzpomidorami";
const char *MQTT_SERVER = "thingsboardrpi.duckdns.org";
const int MQTT_PORT = 1883;
const char *MQTT_TOKEN = "og3ms9bts9x8a9agm00c";

// Global variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// code for heart rate
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndSP2OSensor;
const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float bpm;
int beatAvg;

// code for sp2o
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte pulseLED = 11; //Must be on PWM pin
byte readLED = 13; //Blinks with each data read

// ... [Rest of the functions from the first code]

void sp2oLoop() {
  // ... [Contents of the loop function from the second code]
}

void setup()
{
  Serial.begin(115200);
  wifiSetup();
  mqttSetup();
  maxSetup();
  // ... [Initialization from the setup function of the second code]
}

void loop()
{
  if (!mqttClient.connected())
  {
    mqttSetup();
  }
  tempAndHumPublish();
  heartRateDetection();
  sp2oLoop();
}

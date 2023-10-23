#include "variables.h"

// Sensors adressess
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndSP2OSensor;

// Global constants
const char *SSID = "Nie mam wi-fi";
const char *PASSWORD = "makaronzpomidorami";
const char *MQTT_SERVER = "thingsboardrpi.duckdns.org";
const int MQTT_PORT = 1883;
const char *MQTT_TOKEN = "og3ms9bts9x8a9agm00c";
const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.

// Global variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);
// byte rates[RATE_SIZE]; // Array of heart rates
// byte rateSpot = 0;
// long lastBeat = 0; // Time at which the last beat occurred
// float bpm;
// int beatAvg;

// sp2o variables
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100];  // infrared LED sensor data
uint16_t redBuffer[100]; // red LED sensor data
#else
uint32_t irBuffer[100];  // infrared LED sensor data
uint32_t redBuffer[100]; // red LED sensor data
#endif

// sp2o calculation variables
int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// sp2o setup variables
byte ledBrightness = 60; // Options: 0=Off to 255=50mA
byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
byte sampleRate = 100;   // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 411;    // Options: 69, 118, 215, 411
int adcRange = 4096;     // Options: 2048, 4096, 8192, 16384
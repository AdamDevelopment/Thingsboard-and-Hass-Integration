#include "variables.h"

// Sensors adressess
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndSP2OSensor;

// Global constants
const byte RATE_SIZE = 15; // Increase this for more averaging. 4 is good.
// AD8232 variables
const int LO_PLUS_PIN = 17;
const int LO_MINUS_PIN = 16;
const int ECG_PIN = 36;
const int SDN = 4;

// misscelanous variables
const int IR_TRESHOLD = 50000;
const unsigned long SPO2_WAIT_TIME = 4000;


// Global variables
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float bpm;
int beatAvg;

// Define the state machine states(for spo2)
Spo2MeasurementState spo2State = INIT;
int sampleIndex = 0;
unsigned long lastSampleTime = 0;

// sp2o variables
uint32_t irBuffer[50];  // infrared LED sensor data
uint32_t redBuffer[50]; // red LED sensor data

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
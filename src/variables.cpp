#include "variables.h"
// Sensors adressess
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndOxygenSensor;


WiFiManager wifiManager;  


const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Global constants
const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
// AD8232 variables
const int LO_PLUS_PIN = 17;
const int LO_MINUS_PIN = 16;
const int ECG_PIN = A0;
const int SDN = 13;
int ecg_value;

// misscelanous variables
const int IR_TRESHOLD = 50000;

SemaphoreHandle_t mutex;
SemaphoreHandle_t publishMutex;
SemaphoreHandle_t publishMutexAD; 

// Global variables
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float bpm;
int beatAvg;
// Object of enum type for storing the current state of the MAX30102 sensor
OXY_MEASURE_STATES MAX30102_STATE = INIT;
// variable for storing the current state of the MAX30102 sensor
int sampleIndex = 0;
// variable for storing the time of the last measurement
unsigned long lastSampleTime = 0;
// variable for storing the time of the last SPO2 measurement
const unsigned long SPO2_WAIT_TIME = 4000;

// variables for MAX30102
uint32_t irBuffer[100];  // infrared diode
uint32_t redBuffer[100]; // red diode

// variables to calculate SPO2
int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;


// initial values for MAX30102
byte ledBrightness = 60; 
byte sampleAverage = 4;  
byte ledMode = 2;        
byte sampleRate = 100;   
int pulseWidth = 411;    
int adcRange = 4096;     

// light sleep variables
unsigned long lastMeasurementTime = 0;
unsigned long lastSleepTime = 0;
const unsigned long aqDuration = 300000; // 5 minutes
const unsigned long sleepDuration = 300000; // 5 minutes
bool sleepState = false;
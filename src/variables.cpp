#include "variables.h"
SemaphoreHandle_t xMutex = NULL;
// Sensors adressess
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndOxygenSensor;

// Global constants
const byte RATE_SIZE = 15; // Increase this for more averaging. 4 is good.
// AD8232 variables
const int LO_PLUS_PIN = 17;
const int LO_MINUS_PIN = 16;
const int ECG_PIN = A0;
const int SDN = 13;

// misscelanous variables
const int IR_TRESHOLD = 50000;


// Global variables
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float bpm;
int beatAvg;

// Obiekt inicjalizacji stanu wejściowego czujnika MAX30102
OXY_MEASURE_STATES MAX30102_STATE = INIT;
// Obiekt inicjalizacji stanu pomiaru
AD_MEASURE_STATES currState = INIT_AD;
// Zmienna do przechowywania indeksu próbki
int sampleIndex = 0;
// Zmienna do przechowywania czasu ostatniej próbki
unsigned long lastSampleTime = 0;
// Zmienna do przechowywania czasu ostatniego pomiaru
const unsigned long SPO2_WAIT_TIME = 4000;

// zmienne do przechowywania danych z czujnika
uint32_t irBuffer[100];  // dioda podczerwona
uint32_t redBuffer[100]; // dioda czerwona

// zmienne do przechowywania danych z czujnika
int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// zmienne inicjalizacyjne dla MAX30102
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
#include <WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SHT3x.h>
#include "MAX30105.h"
#include "heartRate.h"
// #define BAT_ADC_PIN 35
// Sensors adressess
SHT3x tempAndHumSensor(0x44);
MAX30105 PulseAndSP2OSensor;

// Global constants
const char* SSID = "Nie mam wi-fi";
const char* PASSWORD = "makaronzpomidorami";
const char* MQTT_SERVER = "thingsboardrpi.duckdns.org";
const int MQTT_PORT = 1883;
const char* MQTT_TOKEN = "og3ms9bts9x8a9agm00c";
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
// const float ADC_RES = 4095;
// const float V_REF = 3.3;


// Global variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float bpm;
int beatAvg;

// void readBatVol() {
//   int adcValue = analogRead(BAT_ADC_PIN);
//   float voltage = ((float)adcValue / ADC_RES)*V_REF;
//   Serial.print("Battery vol: ");
//   Serial.print(voltage);

//   char payload[100];
//   snprintf(payload, sizeof(payload), "{\"voltage\": %.2f}", voltage);
//   mqttClient.publish("v1/devices/me/telemetry", payload);
//   // bool publishVoltage = mqttClient.publish("v1/devices/me/telemetry",payload);
//   // if (publishVoltage) {
//   //   Serial.println("Publish successful");
//   // } else {
//   //   Serial.println("Publish failed");
//   // }

// }


void wifiSetup(){
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void mqttSetup(){
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32",MQTT_TOKEN, NULL)) {
      Serial.println("Connected to MQTT");
      } else {
      delay(5000);
    }
  }
}
void maxSetup(){
  Serial.begin(115200);
  Serial.println("Initializing...");
  // Initialize sensor
  if (!PulseAndSP2OSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  PulseAndSP2OSensor.setup(); //Configure sensor with default settings
  PulseAndSP2OSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  PulseAndSP2OSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void heartRateDetection(){
  long irValue = PulseAndSP2OSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    bpm = 60 / (delta / 1000.0);
    if (bpm < 255 && bpm > 20)
    {
      rates[rateSpot++] = (byte)bpm; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(bpm);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000) {
    Serial.print(" No finger?");
    return;
  }
  char payload[100];
  snprintf(payload, sizeof(payload), "{\"BPM\": \"%s\"}", String(beatAvg).c_str());
  mqttClient.publish("v1/devices/me/telemetry", payload);
  Serial.println();
}

// Temperature and humidity publish function
void tempAndHumPublish(){
  tempAndHumSensor.UpdateData();
  float temp =tempAndHumSensor.GetTemperature();
  float hum =tempAndHumSensor.GetRelHumidity();

  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", temp, hum);
  mqttClient.publish("v1/devices/me/telemetry", payload);
  Serial.println(temp);
  Serial.println(hum);
}

void setup() {
  Serial.begin(115200);
  tempAndHumSensor.Begin();
  PulseAndSP2OSensor.begin();
  wifiSetup();
  mqttSetup();
  maxSetup();
}

void loop() {
  if (!mqttClient.connected()) {
    mqttSetup();
  }
  tempAndHumPublish();
  heartRateDetection();
  // readBatVol();
} 



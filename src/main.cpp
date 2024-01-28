#include "variables.h"
#include "functions.h"

void setup()
{
  Serial.begin(115200);  
  WifiManagerSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();
}

void loop()
{
  unsigned long currentTime = millis();
  if (sleepState) {
    if (currentTime - lastSleepTime >= sleepDuration) {
      
      // Wybudzenie z trybu light sleep
      sleepState = false;
      lastMeasurementTime = currentTime;
      // Ponowne nawiązanie połączeń i wykonanie pomiarów
      if (WiFi.status() != WL_CONNECTED) {
        WifiManagerSetup();
      }
      if (!mqttClient.connected()) {
        mqttSetup();
        
      }
      tempAndHumPublish();
      heartRateDetection();
      spo2Measurement();
      ad8232Publish();
    }
  } else {
    // Wykonuj pomiary
    if (currentTime - lastMeasurementTime >= aqDuration) {
      // Przejście w tryb light sleep
      lightSleep();
      sleepState = true;
      lastSleepTime = currentTime;
    } else {
      // Kontynuacja pomiarów
      if (WiFi.status() != WL_CONNECTED) {
        WifiManagerSetup();
      }
      if (!mqttClient.connected()) {
        mqttSetup();
        
      }
      tempAndHumPublish();
      heartRateDetection();
      spo2Measurement();
      ad8232Publish();
    }
  }
}


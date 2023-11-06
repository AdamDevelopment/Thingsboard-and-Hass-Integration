#include "variables.h"
#include "functions.h"

void setup()
{
  Serial.begin(115200);  
  wifiSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();
}

void loop()
{
  if (!mqttClient.connected())
  {
    mqttSetup();
  }
  tempAndHumPublish();
  heartRateDetection();
  spo2Measurement();
  ad8232Publish();
}
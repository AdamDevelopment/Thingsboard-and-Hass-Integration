#include "variables.h"
#include "functions.h"

void setup()
{
  Serial.begin(115200);  
  WifiManagerSetup();
  // wifiSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();
}

void loop()
{
  if (!WiFi.status() == WL_CONNECTED)
  {
    WifiManagerSetup();
  }
  if (!mqttClient.connected())
  {
    mqttSetup();
  }
  tempAndHumPublish();
  heartRateDetection();
  spo2Measurement();
  ad8232Publish();
  // manageDeepSleep(false, 60 * 1000, 5 * 60 * 1000);
}
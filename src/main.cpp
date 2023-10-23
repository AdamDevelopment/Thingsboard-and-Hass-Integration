#include "variables.h"
#include "functions.h"

void setup()
{
  Serial.begin(115200);
  wifiSetup();
  mqttSetup();
  maxSetup();
}

void loop()
{
  if (!mqttClient.connected())
  {
    mqttSetup();
  }
  tempAndHumPublish();
  // heartRateDetection();
  sp2oMeasurement();
}

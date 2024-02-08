#include "variables.h"
#include "functions.h"

void Task1(void *pvParameters)
{

  for (;;)
  {
    tempAndHumPublish();
    heartRateDetection();
    MAX30102_SPO2_MEASUREMENT();
  }
}

void Task2(void *pvParameters)
{
  for (;;)
  {
    ad8232Publish();
  }
}

void setup()
{
  Serial.begin(115200);
  WifiManagerSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();
  xTaskCreatePinnedToCore(
      Task1,   /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,   /* Stack size in words */
      NULL,    /* Task input parameter */
      1,       /* Priority of the task */
      NULL,    /* Task handle. */
      0);      /* Core where the task should run */

  xTaskCreatePinnedToCore(
      Task2,   /* Function to implement the task */
      "Task2", /* Name of the task */
      10000,   /* Stack size in words */
      NULL,    /* Task input parameter */
      1,       /* Priority of the task */
      NULL,    /* Task handle. */
      1);      /* Core where the task should run */
}

void loop()
{

}

#include "variables.h"
#include "functions.h"

// void Task1(void *pvParameters)
// {

//   for (;;)
//   {
//     if (xSemaphoreTake(mutex, (TickType_t)100) == pdTRUE)
//     {
//       tempAndHumPublish();
//       heartRateDetection();
//       MAX30102_SPO2_MEASUREMENT();
//       xSemaphoreGive(mutex);
//     }
//   }
// }
// void Task2(void *pvParameters)
// {
//   for (;;)
//   {
//     if (xSemaphoreTake(ad8232mutex, (TickType_t)10) == pdTRUE)
//     {
//       ad8232Publish();
//       xSemaphoreGive(ad8232mutex);
//     }
//   }
//   vTaskDelay(pdMS_TO_TICKS(10));
// }

void setup()
{
  Serial.begin(115200);

  // Inicjalizacja LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount LittleFS, formatting...");
    LittleFS.format();
    Serial.println("LittleFS formatted");
    LittleFS.begin();
  }

  Serial.println("Mounting File System...");

  if (!loadConfigFile())
  {
    Serial.println("No configuration file found or failed to load, launching WiFi setup");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WifiManagerSetup(); // Ustawienia WiFi Manager
  mqttSetup();
  maxSetup(); // Zakładam, że to Twoja funkcja inicjalizująca dodatkowe komponenty
  // ad8232Setup();
  // xTaskCreatePinnedToCore(
  //     Task1,
  //     "Task1",
  //     10000,
  //     NULL,
  //     1,
  //     NULL,
  //     0);

  // xTaskCreatePinnedToCore(
  //     Task2,
  //     "Task2",
  //     10000,
  //     NULL,
  //     2,
  //     NULL,
  //     1);
}

void loop()
{
  if (!WL_CONNECTED) {
    Serial.println("WiFi not connected");
    resetDeviceSettings();
  }
  getTimeStamp();
  tempAndHumPublish();
  heartRateDetection();
}
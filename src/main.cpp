#include "variables.h"
#include "functions.h"
#define DELAY_SENSORS pdMS_TO_TICKS(30000)  // Opóźnienie dla Task1
#define DELAY_AD pdMS_TO_TICKS(5)          // Opóźnienie dla Task3
#define SEMAPHORE_TICKS pdMS_TO_TICKS(50)   // Opóźnienie dla semafora
#define SEMAPHORE_TICKS_AD pdMS_TO_TICKS(7) // Opóźnienie dla semafora
int TimeForADreading = 100 * 300000;        // 30s

void Task2(void *pvParameters)
{
  uint64_t TimeADreading = esp_timer_get_time();
  for (;;)
  {
    tempAndHumPublish();
    heartRateDetection();
    MAX30102_SPO2_MEASUREMENT();
    // Oczekiwanie na możliwość publikacji danych
    if ((esp_timer_get_time() - TimeADreading) >= TimeForADreading)
    {
      if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE)
      {
        // Publikuj tylko, gdy nie ma priorytetowych danych EKG do wysłania
        publishAllSensorsData(); // Wysyłanie zebranych danych
        xSemaphoreGive(publishMutex);
        TimeADreading = esp_timer_get_time();
      }
    }
  }
}

void Task3(void *pvParameters)
{
  for (;;)
  {

    if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE)
    {
      publishAD(); // Wysyłanie danych z AD8232
      xSemaphoreGive(publishMutex);
    }

    vTaskDelay(DELAY_AD);
  }
}

void setup()
{
  publishMutex = xSemaphoreCreateMutex();
  TaskHandle_t task2Handle;
  TaskHandle_t task3Handle;
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WifiManagerSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();

  xTaskCreatePinnedToCore(Task2, "PublishTask", 8192, NULL, 1, &task2Handle, 0); // Task2 również na rdzeniu 0
  xTaskCreatePinnedToCore(Task3, "ADTask", 10000, NULL, 2, &task3Handle, 1);     // Task3 na rdzeniu 1 z najwyższym priorytetem
}

void loop()
{
}

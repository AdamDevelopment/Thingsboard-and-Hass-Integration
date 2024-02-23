#include "variables.h"
#include "functions.h"
#define DELAY_AD pdMS_TO_TICKS(5)          // Opóźnienie dla Task3
int TimeForADreading = 100 * 100000;        // 10s

void Task2(void *pvParameters)
{
  uint64_t TimeADreading = esp_timer_get_time();
  for (;;)
  {
    tempAndHumPublish();
    heartRateDetection();
    MAX30102_SPO2_MEASUREMENT();
    // Oczekiwanie na możliwość publikacji danych (10 sekund)
    if ((esp_timer_get_time() - TimeADreading) >= TimeForADreading)
    {
      if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE) // Czekanie na semafor
      {
        
        publishAllSensorsData(); // Wysyłanie zebranych danych
        xSemaphoreGive(publishMutex); // Zwolnienie semafora
        TimeADreading = esp_timer_get_time();
      }
    }
  }
}

void Task3(void *pvParameters)
{
  for (;;)
  {

    if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE) // Czekanie na semafor
    {
      publishAD(); // Wysyłanie danych z AD8232
      xSemaphoreGive(publishMutex); // Zwolnienie semafora
    }

    vTaskDelay(DELAY_AD); // Opóźnienie dla Task3
  }
}

void setup()
{
  publishMutex = xSemaphoreCreateMutex(); // Tworzenie semafora
  TaskHandle_t task2Handle; 
  TaskHandle_t task3Handle;
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); // Ustawienie czasu z serwera NTP celem
                                                                         // synchronizacji czasu w ESP32
  WifiManagerSetup(); 
  mqttSetup();
  maxSetup();
  ad8232Setup();

  xTaskCreatePinnedToCore(Task2, "PublishTask", 8192, NULL, 1, &task2Handle, 0); // Task2 na rdzeniu 0
  xTaskCreatePinnedToCore(Task3, "ADTask", 10000, NULL, 2, &task3Handle, 1);     // Task3 na rdzeniu 1 z najwyższym priorytetem
}

void loop()
{
}

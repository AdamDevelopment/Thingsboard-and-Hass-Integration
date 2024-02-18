#include "variables.h"
#include "functions.h"
#define DELAY_SENSORS pdMS_TO_TICKS(1000)  // Opóźnienie dla Task1
#define DELAY_AD pdMS_TO_TICKS(10)          // Opóźnienie dla Task3
#define SEMAPHORE_TICKS pdMS_TO_TICKS(100) // Opóźnienie dla semafora

void Task1(void *pvParameters)
{
  for (;;)
  {
    // Zbieranie danych z sensorów
    tempAndHumPublish();
    heartRateDetection();
    MAX30102_SPO2_MEASUREMENT();

    vTaskDelay(DELAY_SENSORS);
  }
}

void Task2(void *pvParameters)
{
  for (;;)
  {
    // Oczekiwanie na możliwość publikacji danych
    if (xSemaphoreTake(publishMutex, SEMAPHORE_TICKS) == pdTRUE)
    {
      // Publikuj tylko, gdy nie ma priorytetowych danych EKG do wysłania
      publishAllSensorsData(); // Wysyłanie zebranych danych
      xSemaphoreGive(publishMutex);
    }
    vTaskDelay(DELAY_SENSORS); // Ten task może być uruchamiany rzadziej, zależnie od potrzeb
  }
}

void Task3(void *pvParameters)
{
  for (;;)
  {

    if (xSemaphoreTake(publishMutex, SEMAPHORE_TICKS) == pdTRUE)
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
  TaskHandle_t task1Handle;
  TaskHandle_t task2Handle;
  TaskHandle_t task3Handle;
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WifiManagerSetup();
  mqttSetup();
  maxSetup();
  ad8232Setup();

  xTaskCreatePinnedToCore(Task1, "SensorTask", 8192, NULL, 1, &task1Handle, 0);  // Task1 na rdzeniu 0
  xTaskCreatePinnedToCore(Task2, "PublishTask", 4096, NULL, 1, &task2Handle, 0); // Task2 również na rdzeniu 0
  xTaskCreatePinnedToCore(Task3, "ADTask", 10000, NULL, 2, &task3Handle, 1);     // Task3 na rdzeniu 1 z najwyższym priorytetem
}

void loop()
{
}

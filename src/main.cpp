#include "variables.h"
#include "functions.h"
#define DELAY_AD pdMS_TO_TICKS(5)          // delay for Task3
int TimeForADreading = 100 * 100000;        // 10s

void Task2(void *pvParameters)
{
  uint64_t TimeADreading = esp_timer_get_time();
  for (;;)
  {
    tempAndHumPublish();
    heartRateDetection();
    MAX30102_SPO2_MEASUREMENT();
    // Waiting for the time to send the data
    if ((esp_timer_get_time() - TimeADreading) >= TimeForADreading)
    {
      if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE) // waiting for the semaphore
      {
        
        publishAllSensorsData(); // Sending all data
        xSemaphoreGive(publishMutex); // releasing the semaphore
        TimeADreading = esp_timer_get_time();
      }
    }
  }
}

void Task3(void *pvParameters)
{
  for (;;)
  {

    if (xSemaphoreTake(publishMutex, portMAX_DELAY) == pdTRUE) // waiting for semaphore
    {
      publishAD(); // Sending AD data
      xSemaphoreGive(publishMutex); // Releasing the semaphore
    }

    vTaskDelay(DELAY_AD); // Delay for Task3
  }
}

void setup()
{
  publishMutex = xSemaphoreCreateMutex(); // Creating a semaphore
  TaskHandle_t task2Handle; 
  TaskHandle_t task3Handle;
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); // Setting up time synchronization in ESP32
  WifiManagerSetup(); 
  mqttSetup();
  maxSetup();
  ad8232Setup();

  xTaskCreatePinnedToCore(Task2, "PublishTask", 8192, NULL, 1, &task2Handle, 0); // Task2 on core 0 with priority 1
  xTaskCreatePinnedToCore(Task3, "ADTask", 10000, NULL, 2, &task3Handle, 1);     // Task2 on core 1 with priority 2
}

void loop()
{
}

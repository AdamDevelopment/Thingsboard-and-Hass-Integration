#include "variables.h"
#include "functions.h"
#include "server_cert.h"
#include "secrets.h"

char mqttServer[40] = "";
char mqttPort[6] = "1883"; // MQTT port default
char mqttToken[40] = "";
char mqttDeviceName[40] = "";
WiFiManagerParameter custom_mqtt_server("server", "MQTT Server Address", "", 40);
WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", "1883", 6);
WiFiManagerParameter custom_mqtt_token("token", "MQTT Token", "", 20);
WiFiManagerParameter custom_device_name("device", "Device Name", "", 20);
bool shouldSaveConfig = false;
int connectionAttempts = 0;
const int maxConnectionAttempts = 1;

void WifiManagerSetup()
{
  Serial.println("Starting WifiManagerSetup...");
  wifiManager.setSaveConfigCallback(saveConfigCallback); // Ustaw callback jako pierwszą czynność

  // wifiManager.resetSettings(); // Resetowanie ustawień WiFiManagera przed próbą połączenia

  wifiManager.setConfigPortalTimeout(180);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_token);
  wifiManager.addParameter(&custom_device_name);

  if (!wifiManager.autoConnect("AutoConnectAP"))
  {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart(); // Resetuje urządzenie, jeśli nie udało się połączyć
  }
  else
  {
    Serial.println("Connected to wifi");
    strcpy(mqttServer, custom_mqtt_server.getValue());
    strcpy(mqttPort, custom_mqtt_port.getValue());
    strcpy(mqttToken, custom_mqtt_token.getValue());
    strcpy(mqttDeviceName, custom_device_name.getValue());
    if (shouldSaveConfig)
    {
      Serial.println("Saving config because shouldSaveConfig is true");
      saveConfigFile();
    }
    else
    {
      Serial.println("Not saving config because shouldSaveConfig is false");
    }
  }
}
void mqttSetup()
{
  Serial.println("Setting up MQTT...");
  espClient.setCACert(mqttserver_pem);
  mqttClient.setServer(mqttServer, atoi(mqttPort));

  if (!mqttClient.connected())
  {
    Serial.println("Próba połączenia z MQTT...");
    if (mqttClient.connect(mqttDeviceName, mqttToken, NULL))
    {
      Serial.println("Połączono z serwerem MQTT!");
      connectionAttempts = 0;
    }
    else
    {
      Serial.print("Nie udało się połączyć z MQTT. Stan: ");
      Serial.println(mqttClient.state());
      connectionAttempts++;
      if (connectionAttempts >= maxConnectionAttempts)
      {
        Serial.println("Przekroczono maksymalną liczbę prób połączenia z MQTT, resetowanie...");
        resetDeviceSettings();
      }
    }
  }
}
void saveConfigCallback()
{
  Serial.println("Zapisywanie konfiguracji...");
  shouldSaveConfig = true;
}
void saveConfigFile()
{
  Serial.println(F("Saving configuration..."));
  StaticJsonDocument<512> json;
  json["mqttServer"] = mqttServer;
  json["mqttPort"] = mqttPort;
  json["mqttToken"] = mqttToken;
  json["mqttDeviceName"] = mqttDeviceName;

  serializeJsonPretty(json, Serial); // Wypisanie konfiguracji do konsoli przed zapisaniem

  File configFile = LittleFS.open("/test_config.json", "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  else
  {
    Serial.println("Config saved successfully");
  }
  configFile.close();
}

bool loadConfigFile()
{
  Serial.println("Mounting File System...");
  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount FS");
    return false;
  }

  if (LittleFS.exists(JSON_CONFIG_FILE))
  {
    File configFile = LittleFS.open(JSON_CONFIG_FILE, "r");
    if (!configFile)
    {
      Serial.println("Failed to open config file");
      return false;
    }

    StaticJsonDocument<512> json;
    DeserializationError error = deserializeJson(json, configFile);
    if (error)
    {
      Serial.println("Failed to read file, using default configuration");
      return false;
    }

    strlcpy(mqttServer, json["mqttServer"], sizeof(mqttServer));
    strlcpy(mqttPort, json["mqttPort"], sizeof(mqttPort));
    strlcpy(mqttToken, json["mqttToken"], sizeof(mqttToken));
    strlcpy(mqttDeviceName, json["mqttDeviceName"], sizeof(mqttDeviceName));
  }
  return true;
}
void resetDeviceSettings()
{
  Serial.println("Resetowanie ustawień urządzenia...");

  wifiManager.resetSettings(); // Reset ustawień WiFi
  delay(1000);                 // Krótkie opóźnienie

  if (LittleFS.begin())
  {
    Serial.println("Formatowanie LittleFS...");
    LittleFS.format();
    Serial.println("LittleFS sformatowany.");
    LittleFS.end();
  }

  ESP.restart(); // Restart urządzenia
}

float tempAndHumPublish()
{
  tempAndHumSensor.Begin();
  tempAndHumSensor.UpdateData();
  float temp = tempAndHumSensor.GetTemperature();
  char payload[100];
  Serial.print("Temperature: ");
  Serial.println(temp);
  // snprintf(payload, sizeof(payload), "{\"temperature\": %.2f}", temp);
  // mqttClient.publish("v1/devices/me/telemetry", payload);
  return temp;
}
// void getTimeStamp()
// {
//   struct timeval now;
//   gettimeofday(&now, NULL);
//   long long timestamp = (now.tv_sec * 1000LL + now.tv_usec / 1000); // Znacznik czasu w milisekundach.
//   int randomVal = random(0, 100);                                   // Generowanie losowej wartości.
//   char payload[256];                                                // Bufor na ciąg JSON, odpowiednio duży.

//   // Formatowanie ciągu JSON zgodnie z dokumentacją ThingsBoard.
//   // Wartości liczbowe nie są otoczone cudzysłowami.
//   snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"testVal\":%d}}", timestamp, randomVal);

//   // Wysyłanie danych telemetrycznych do ThingsBoard.
//   Serial.printf("Payload: %s\n", payload);
//   mqttClient.publish("v1/devices/me/telemetry", payload);
// }
void maxSetup()
{
  Serial.println("Inicjalizacja sensora");
  // Inicjalizacja sensora MAX30102
  if (!PulseAndOxygenSensor.begin(Wire, I2C_SPEED_FAST)) // Ustawienie domyslnego adres oraz szybkości komunikacji I2C
  {
    Serial.println("nie znaleziono sensora MAX30102");
    while (1)
      ;
  }
  // Ustawienie parametrów sensora
  ledBrightness;
  sampleAverage;
  ledMode;
  sampleRate;
  pulseWidth;
  adcRange;
  PulseAndOxygenSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  PulseAndOxygenSensor.setPulseAmplitudeRed(0x0A);
  PulseAndOxygenSensor.setPulseAmplitudeGreen(0);
}

// ad8232 setup function
// void ad8232Setup()
// {
//   pinMode(LO_PLUS_PIN, INPUT);
//   pinMode(LO_MINUS_PIN, INPUT);
//   pinMode(SDN, OUTPUT);
//   digitalWrite(SDN, HIGH);
// }

// Heart rate detection function
int heartRateDetection()
{

  long irValue = PulseAndOxygenSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    bpm = 60 / (delta / 1000.0);
    if (bpm < 255 && bpm > 20)
    {
      rates[rateSpot++] = (byte)bpm; // Store this reading in the array
      rateSpot %= RATE_SIZE;         // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
    return beatAvg;
  }

  Serial.print("Avg BPM=");
  Serial.print(beatAvg);
  char attributesPayload[50];
  if (irValue < IR_TRESHOLD)
  {
    Serial.println(" No finger?");
    beatAvg = 0;
    // Opublikuj atrybut 'value' jako false, ponieważ nie wykryto palca
    snprintf(attributesPayload, sizeof(attributesPayload), "{\"value\":false}");
    if (!mqttClient.publish("v1/devices/me/attributes", attributesPayload))
    {
      Serial.println("Attributes publish failed due to no finger");
    }
  }
  else
  {
    // Opublikuj atrybut 'value' jako true, ponieważ wykryto prawidłowy pomiar
    snprintf(attributesPayload, sizeof(attributesPayload), "{\"value\":true}");
    if (!mqttClient.publish("v1/devices/me/attributes", attributesPayload))
    {
      Serial.println("Attributes publish failed");
    }
  }
  return beatAvg;
}

// void lightSleep()
// {
//   uint32_t SLEEP_DURATION = 300000000;
//   Serial.println("Light Sleep mode activated");
//   delay(1000);
//   Serial.println("Going to sleep now");
//   esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
//   esp_light_sleep_start();
// }

int32_t MAX30102_SPO2_MEASUREMENT()
{

  // Zdefiniowanie zmiennej czasu w celu zachowania ciągłości pomiaru
  unsigned long currentTime = millis(); // Pobiera aktualny czas w milisekundach od startu programu

  switch (MAX30102_STATE) // Rozpoczęcie obsługi stanów pomiaru czujnika MAX30102
  {
  case INIT:                      // Stan inicjalizacji: przygotowanie do nowego cyklu pomiarowego
    bufferLength = 100;           // Ustawienie długości bufora na 100 próbek
    sampleIndex = 0;              // Zerowanie indeksu próbek, start od początku
    lastSampleTime = currentTime; // Zapisanie bieżącego czasu jako punkt odniesienia
    MAX30102_STATE = COLLECT;     // Przejście do stanu zbierania danych
    break;
  case COLLECT:                     // Stan zbierania danych: odczyt wartości z czujnika
    if (sampleIndex < bufferLength) // Sprawdzenie, czy bufor nie jest pełny
    {
      if (PulseAndOxygenSensor.available()) // Sprawdzenie, czy czujnik ma dostępną nową próbkę
      {
        redBuffer[sampleIndex] = PulseAndOxygenSensor.getIR(); // Zapisanie wartości diory IR do bufora
        irBuffer[sampleIndex] = PulseAndOxygenSensor.getRed(); // Zapisanie wartości z diody czerwonej do bufora
        PulseAndOxygenSensor.nextSample();                     // Przygotowanie do odczytu kolejnej próbki
        sampleIndex++;                                         // Inkrementacja indeksu próbki
      }
    }
    else // Jeśli bufor jest pełny
    {
      MAX30102_STATE = PROCESS; // Przejście w stan przetwarzania danych
    }
    break;
  case PROCESS: // Stan przetwarzania danych: obliczanie SpO2 i tętna
    // Inicjalizacja zmiennych potrzebnych do obliczeń
    maxim_heart_rate_and_oxygen_saturation(irBuffer,
                                           bufferLength,
                                           redBuffer,
                                           &spo2,
                                           &validSPO2,
                                           &heartRate,
                                           &validHeartRate);
    Serial.print("Heart Rate: "); // Wyświetlenie wartości tętna
    if (validSPO2 && spo2 != -999) // Sprawdzenie, czy pomiar SpO2 jest ważny
    {
      Serial.print("SpO2: "); // Wyświetlenie wartości SpO2
      Serial.println(spo2);
    }
    else // Jeśli nie wykryto palca
    {
      spo2 = 0; // Przypisanie wartości 0 do zmiennej SpO2
      Serial.println("Nie wykryto palca"); // Informacja o braku palca
    }
    sampleIndex = 0;              // Resetowanie indeksu próbki dla kolejnego cyklu
    lastSampleTime = currentTime; // Aktualizacja czasu ostatniej próbki
    MAX30102_STATE = WAIT;        // Przejście w stan oczekiwania
    break;
  case WAIT:                                            // Stan oczekiwania
    if (currentTime - lastSampleTime >= SPO2_WAIT_TIME) // Sprawdzenie, czy upłynął wymagany czas oczekiwania (4000ms)
    {
      MAX30102_STATE = INIT; // Powrót do stanu inicjalizacji w celu rozpoczęcia nowego cyklu pomiarowego
    }
    break;
  }
  return spo2;
}

// void ad8232Publish()
// {

//   if (digitalRead(LO_PLUS_PIN) == 1 || digitalRead(LO_MINUS_PIN) == 1)
//   {
//     Serial.println("Check the sensor connections.");
//   }
//   else
//   {
//     Serial.print("ECG: ");
//     Serial.println(analogRead(ECG_PIN));
//     char payload[100];

//     snprintf(payload, sizeof(payload), "{\"ECG\": %d}", analogRead(ECG_PIN));
//     mqttClient.publish("v1/devices/me/telemetry", payload);
//   }
//   vTaskDelay(pdMS_TO_TICKS(1));
// }

void publishAllSensorsData()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  long long timestamp = (now.tv_sec * 1000LL + now.tv_usec / 1000); // Znacznik czasu w milisekundach.
  float temp = tempAndHumPublish();
  int heartRate = heartRateDetection();
  int32_t spo2_value = MAX30102_SPO2_MEASUREMENT();
  Serial.printf("Timestamp: %lld\n", timestamp);
  Serial.printf("Temperature: %.2f\n", temp);
  Serial.printf("Heart Rate: %d\n", heartRate);
  Serial.printf("SpO2: %ld\n", spo2_value);

  // Konstrukcja payloadu z danymi z sensorów
  char payload[256];
  snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"temperature\":%.2f, \"SPO2\":%ld, \"BPM\":%d}}", timestamp, temp, spo2_value, heartRate);

  Serial.println(payload); // Debug
  mqttClient.publish("v1/devices/me/telemetry", payload);
}

// getTimeStamp();
// ad8232Publish();
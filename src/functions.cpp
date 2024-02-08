#include "variables.h"
#include "functions.h"
#include "server_cert.h"
#include "secrets.h"

void lightSleep()
{
  uint32_t SLEEP_DURATION = 300000000;
  Serial.println("Light Sleep mode activated");
  delay(1000);
  Serial.println("Going to sleep now");
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
  esp_light_sleep_start();
}

void WifiManagerSetup()
{
  // Zdefiniowanie serwera głównego i tokenu urządzenia
  // char mainServer[28] = MAIN_SERVER;
  // char accToken[22] = ACCESS_TOKEN;
  // // zainicjowanie obiektu klasy WiFiManager
  WiFiManager wifiManager;

  // // Deklaracja niestandardowych parametrów dla serwera i tokenu
  // WiFiManagerParameter custom_server("server", "Server Address", "", 40);
  // WiFiManagerParameter custom_token("token", "Device token", "", 40);

  // Dodanie niestandardowych parametrów do WiFiManager
  // wifiManager.addParameter(&custom_server);
  // wifiManager.addParameter(&custom_token);

  // resetowanie ustawień (opcjonalne)
  // wifiManager.resetSettings();

  // Stworzenie punktu dostępowego
  wifiManager.autoConnect("Opaska pomiarowa");
  // Serial.println(custom_server.getValue());
  // Serial.println(custom_token.getValue());
  // // Sprawdzenie, czy wprowadzony serwer i token są zgodne z oczekiwanymi wartościami
  // if (strcmp(custom_server.getValue(), mainServer) != 0 || strcmp(custom_token.getValue(), accToken) != 0)
  // {
  //   Serial.println("Błędne dane. Resetowanie punktu dostępowego...");
  //   ESP.restart(); // Zrestartuj Punkt Dostępowy
  // }else{
  //   Serial.println("Połączono");
  // }
}

void mqttSetup()
{
  // Zdefiniowanie zmiennych protkołu MQTT (adres, port, token)
  const char *mqttServer = MQTT_SERVER;
  const int mqttPort = MQTT_PORT;
  const char *mqttToken = MQTT_TOKEN;
  const char *mqttDeviceName = MQTT_DEVICE_NAME;
  // Ustawienie zabezpieczenia TLS dla klienta MQTT (opcjonalne)
  espClient.setCACert(mqttserver_pem);
  // Ustawienie serwera i portu MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  // Sprawdzenie połączenia z serwerem MQTT
  while (!mqttClient.connected())
  {
    Serial.println("Łączenie z serwerem MQTT...");
    // Łączenie z serwerem MQTT
    if (mqttClient.connect(MQTT_DEVICE_NAME, mqttToken, NULL))
    {
      Serial.println("Połączono");
    }
    else
    {
      Serial.print("Połączenie nieudane, kod błędu=");
      // Wyświetlenie kodu błędu (-1, -2, -3, -4, -5)
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

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
void ad8232Setup()
{
  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  pinMode(SDN, OUTPUT);
  digitalWrite(SDN, HIGH);
}

// Heart rate detection function
void heartRateDetection()
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
  }

  Serial.print("Avg BPM=");
  Serial.print(beatAvg);

  char payload[100];
  if (irValue < IR_TRESHOLD)
  {
    Serial.println(" No finger?");
    beatAvg = 0;
    snprintf(payload, sizeof(payload), "{\"BPM\": \"%d\"}", 0);
    if (!mqttClient.publish("v1/devices/me/telemetry", payload))
    {
      Serial.println("Telemetry publish failed due to no finger");
    }
    return;
  }
  else
  {
    snprintf(payload, sizeof(payload), "{\"BPM\": \"%d\"}", beatAvg);
    Serial.println();
    if (!mqttClient.publish("v1/devices/me/telemetry", payload))
    {
      Serial.println("Telemetry publish failed");
    }
  }
}

void tempAndHumPublish()
{

  tempAndHumSensor.Begin();
  tempAndHumSensor.UpdateData();
  float temp = tempAndHumSensor.GetTemperature();

  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.2f}", temp);

  mqttClient.publish("v1/devices/me/telemetry", payload);
  Serial.print("Temperature: ");
  Serial.println(temp);
}

void MAX30102_SPO2_MEASUREMENT()
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
    char payload[100];             // Przygotowanie paczki danych do wysłania na serwer
    if (validSPO2 && spo2 != -999) // Sprawdzenie, czy pomiar SpO2 jest ważny
    {
      snprintf(payload, sizeof(payload), "{\"SPO2\": \"%d\"}", spo2); // Formatowanie danych SpO2 do wysłania na serwer
      mqttClient.publish("v1/devices/me/telemetry", payload);         // Publikowanie danych SpO2
      Serial.print("SPO2=");                                          // Wyświetlenie wartości SpO2
      Serial.print(spo2, DEC);
      Serial.println();
    }
    else // Jeśli nie wykryto palca
    {
      Serial.println("Nie wykryto palca");                         // Informacja o braku palca
      snprintf(payload, sizeof(payload), "{\"SPO2\": \"%d\"}", 0); // Wysyłanie wartości 0 jako SpO2
      mqttClient.publish("v1/devices/me/telemetry", payload);      // Publikowanie danych
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
}

void ad8232Publish()
{

  if (digitalRead(LO_PLUS_PIN) == 1 || digitalRead(LO_MINUS_PIN) == 1)
  {
    Serial.println("Check the sensor connections.");
  }
  else
  {
    Serial.print("ECG: ");
    Serial.println(analogRead(ECG_PIN));
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"ECG\": %d}", analogRead(ECG_PIN));
    mqttClient.publish("v1/devices/me/telemetry", payload);
  }
  delay(1);
}

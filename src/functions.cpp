#include "variables.h"
#include "functions.h"
#include "server_cert.h"

void wifiSetup()
{
  espClient.setCACert(mqttserver_pem);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void mqttSetup()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  while (!mqttClient.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32", MQTT_TOKEN, NULL))
    {
      Serial.println("Connected to MQTT");
    }
    else
    {
      Serial.println("Connection to MQTT failed");
      delay(5000);
    }
  }
}
// spo2 and heart rate setup function
void maxSetup()
{
  Serial.println("Initializing...");
  // Initialize sensor
  if (!PulseAndSP2OSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  ledBrightness;                                                                                     // Options: 0=Off to 255=50mA
  sampleAverage;                                                                                     // Options: 1, 2, 4, 8, 16, 32
  ledMode;                                                                                           // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  sampleRate;                                                                                        // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  pulseWidth;                                                                                        // Options: 69, 118, 215, 411
  adcRange;                                                                                          // Options: 2048, 4096, 8192, 16384
  PulseAndSP2OSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with default settings
  PulseAndSP2OSensor.setPulseAmplitudeRed(0x0A);                                                     // Turn Red LED to low to indicate sensor is running
  PulseAndSP2OSensor.setPulseAmplitudeGreen(0);                                                      // Turn off Green LED
}

// ad8232 setup function
void ad8232Setup()
{
  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  pinMode(SDN, OUTPUT);
  digitalWrite(SDN, HIGH);
  analogReadResolution(12);
}

// Heart rate detection function
void heartRateDetection()
{
  long irValue = PulseAndSP2OSensor.getIR();

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
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(bpm);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  char payload[100];
  if (irValue < IR_TRESHOLD)
  {
    Serial.println("No finger?");
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
  float hum = tempAndHumSensor.GetRelHumidity();

  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", temp, hum);
  mqttClient.publish("v1/devices/me/telemetry", payload);
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(hum);
}
// spo2 state machine function
void spo2Measurement()
{
  unsigned long currentMillis = millis(); // for constant loop speed(before it was 1000ms)

  switch (spo2State)
  {
  case INIT:
    bufferLength = 50; // initialize buffer length because variable 'n_ir_buffer_length' is used as divisor (0) in a library, and its causing "divide by zero" error
    sampleIndex = 0;
    lastSampleTime = currentMillis;
    spo2State = COLLECTING;
    break;
  case COLLECTING:
    if (sampleIndex < bufferLength)
    {
      if (PulseAndSP2OSensor.available())
      {
        redBuffer[sampleIndex] = PulseAndSP2OSensor.getIR();
        irBuffer[sampleIndex] = PulseAndSP2OSensor.getRed();
        PulseAndSP2OSensor.nextSample();
        sampleIndex++;
      }
    }
    else
    {
      spo2State = PROCESSING;
    }
    break;
  case PROCESSING:
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    char payload[100];
    if (validSPO2 && spo2 != -999)
    {
      snprintf(payload, sizeof(payload), "{\"SPO2\": \"%d\"}", spo2);
      mqttClient.publish("v1/devices/me/telemetry", payload);
      Serial.print("SPO2=");
      Serial.print(spo2, DEC);
      Serial.println();
    }
    else
    {
      Serial.println("No finger?");
      snprintf(payload, sizeof(payload), "{\"SPO2\": \"%d\"}", 0);
      mqttClient.publish("v1/devices/me/telemetry", payload);
    }
    sampleIndex = 0;
    lastSampleTime = currentMillis;
    spo2State = WAITING;
    break;
  case WAITING:
    if (currentMillis - lastSampleTime >= SPO2_WAIT_TIME)
    {
      spo2State = INIT;
    }
    break;
  }
}
// ad8232 publish function
void ad8232Publish()
{
  if (digitalRead(LO_PLUS_PIN) == 1 || digitalRead(LO_MINUS_PIN) == 1)
  {
    Serial.println("Check the sensor connections.");
  }
  else
  {
    int ecgValue = analogRead(ECG_PIN);
    Serial.println(ecgValue);
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"ECG\": \"%s\"}", String(ecgValue).c_str());
    mqttClient.publish("v1/devices/me/telemetry", payload);
  }
  delay(1); // delay for better readings (recommended by the manufacturer)
}

// might use it later
// Removed the redundant Serial.println() if it's not needed

// // spo2 measurement function
// void spo2Measurement()
// {
//   bufferLength = 50; // buffer length of 50 stores 4 seconds of samples running at 25sps
//   // read the first 50 samples, and determine the signal range
//   for (byte i = 0; i < bufferLength; i++)
//   {
//     redBuffer[i] = PulseAndSP2OSensor.getIR();
//     irBuffer[i] = PulseAndSP2OSensor.getRed();
//     PulseAndSP2OSensor.nextSample(); // We're finished with this sample so move to next sample
//   }
//   // calculate SpO2 after first 50 samples (first 4 seconds of samples)
//   maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

//   // dumping the first 12 sets of samples in the memory and shift the last 38 sets of samples to the top
//   for (byte i = 12; i < 50; i++)
//   {
//     redBuffer[i - 12] = redBuffer[i];
//     irBuffer[i - 12] = irBuffer[i];
//   }
//   byte i = 38; // Declare i outside the loop for subsequent use
//   // take 12 sets of samples before calculating the heart rate.
//   for (; i < 50; i++)
//   {
//     while (PulseAndSP2OSensor.available() == false)
//       PulseAndSP2OSensor.check();
//     redBuffer[i] = PulseAndSP2OSensor.getIR();
//     irBuffer[i] = PulseAndSP2OSensor.getRed();
//     PulseAndSP2OSensor.nextSample();
//   }

//   // print the SpO2 every 4 second
//   Serial.print("SPO2=");
//   Serial.print(spo2, DEC);
//   Serial.println();

//   // After gathering 25 new samples recalculate HR and SP02
//   maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

//   char payload[100];
//   snprintf(payload, sizeof(payload), "{\"SPO2\": \"%s\"}", String(spo2).c_str());
//   mqttClient.publish("v1/devices/me/telemetry", payload);
// }

// Temperature and humidity publish function
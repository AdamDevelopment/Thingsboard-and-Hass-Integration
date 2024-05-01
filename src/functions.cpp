#include "variables.h"
#include "functions.h"
#include "server_cert.h"
#include "secrets.h" - 

char mqttServer[40] = "";
char mqttPort[6] = "1883"; // Default MQTT port
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
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Resetting WiFiManager settings before attempting to connect
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_token);
  wifiManager.addParameter(&custom_device_name);

  if (!wifiManager.autoConnect("Measurement Band"))
  {
    Serial.println("Failed to connect to WiFi");
    ESP.restart(); // Resets the device if the connection failed
  }
  else
  {
    Serial.println("Connected to Wi-Fi");
    strcpy(mqttServer, custom_mqtt_server.getValue());
    strcpy(mqttPort, custom_mqtt_port.getValue());
    strcpy(mqttToken, custom_mqtt_token.getValue());
    strcpy(mqttDeviceName, custom_device_name.getValue());
    if (shouldSaveConfig)
    {
      saveConfigFile();
    }
    else
    {
      Serial.println("Failed to save configuration");
    }
  }
}

void mqttSetup()
{
  // Setting up TLS security for the MQTT client
  espClient.setCACert(server_cert_pem);
  // Setting the MQTT server and port from previously entered data
  mqttClient.setServer(mqttServer, atoi(mqttPort));
  // Checking connection to the MQTT server
  if (!mqttClient.connected())
  {
    // Connecting to the MQTT server
    Serial.println("Attempting to connect to MQTT...");
    if (mqttClient.connect(mqttDeviceName, mqttToken, NULL))
    {
      Serial.println("Connected to the MQTT server!");
      connectionAttempts = 0;
    }
    else
    {
      // Displaying connection error information
      Serial.print("Failed to connect to MQTT. Status: ");
      Serial.println(mqttClient.state());
      connectionAttempts++;
      // Resetting the device if the maximum number of connection attempts is exceeded
      if (connectionAttempts >= maxConnectionAttempts)
      {
        Serial.println("Exceeded maximum number of MQTT connection attempts, resetting...");
        resetDeviceSettings();
      }
    }
  }
}

void saveConfigCallback()
{
  Serial.println("Saving configuration...");
  shouldSaveConfig = true;
}

void saveConfigFile()
{
  Serial.println("Saving configuration...");
  StaticJsonDocument<512> json;
  json["mqttServer"] = mqttServer;
  json["mqttPort"] = mqttPort;
  json["mqttToken"] = mqttToken;
  json["mqttDeviceName"] = mqttDeviceName;

  serializeJsonPretty(json, Serial); // Printing configuration to console before saving

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  if (serializeJson(json, configFile) == 0)
  {
    Serial.println("Failed to write to file");
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

  if (LittleFS.exists("/config.json"))
  {
    File configFile = LittleFS.open("/config.json", "r");
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
  Serial.println("Resetting device settings...");

  wifiManager.resetSettings(); // Reset WiFi settings
  delay(1000);                 // Short delay

  if (LittleFS.begin())
  {
    Serial.println("Formatting LittleFS...");
    LittleFS.format();
    Serial.println("LittleFS formatted.");
    LittleFS.end();
  }

  ESP.restart(); // Restart the device
}

float tempAndHumPublish()
{
  tempAndHumSensor.Begin();
  tempAndHumSensor.UpdateData();
  float temp = tempAndHumSensor.GetTemperature();
  return temp;
}

void maxSetup()
{
  Serial.println("Initializing sensor");
  // Initialization of the MAX30102 sensor
  if (!PulseAndOxygenSensor.begin(Wire, I2C_SPEED_FAST)) // Setting default address and I2C communication speed
  {
    Serial.println("MAX30102 sensor not found");
    while (1)
      ;
  }
  PulseAndOxygenSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  PulseAndOxygenSensor.setPulseAmplitudeRed(0x0A);
  PulseAndOxygenSensor.setPulseAmplitudeGreen(0);
}

void ad8232Setup()
{
  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  pinMode(SDN, OUTPUT);
  digitalWrite(SDN, HIGH);
  analogReadResolution(12);
}

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
      Serial.println("BPM: ");
      Serial.println(bpm);
      rates[rateSpot++] = (byte)bpm; // Store this reading in the array
      rateSpot %= RATE_SIZE;         // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];

      beatAvg /= RATE_SIZE;
      Serial.println("Beat Average: ");
      Serial.println(beatAvg);
    }
  }
  return beatAvg;
}

int32_t MAX30102_SPO2_MEASUREMENT()
{
  // Defining time variable to maintain continuity of measurement
  unsigned long currentTime = millis(); // Gets current time in milliseconds since the program started

  switch (MAX30102_STATE) // Start handling sensor measurement states
  {
  case INIT:                      // Initialization state: prepare for a new measurement cycle
    bufferLength = 100;           // Set buffer length to 100 samples
    sampleIndex = 0;              // Zero the sample index
    lastSampleTime = currentTime; // Record current time as a reference point
    MAX30102_STATE = COLLECT;     // Move to data collection state
    break;
  case COLLECT:                     // Reading values from the sensor
    if (sampleIndex < bufferLength) // Check if the buffer is not full
    {
      if (PulseAndOxygenSensor.available()) // Check if a new sample is available from the sensor
      {
        redBuffer[sampleIndex] = PulseAndOxygenSensor.getIR();   // Save the IR diode value to the buffer
        irBuffer[sampleIndex] = PulseAndOxygenSensor.getRed();   // Save the red diode value to the buffer
        PulseAndOxygenSensor.nextSample();                       // Prepare to read the next sample
        sampleIndex++;                                           // Increment the sample index
      }
    }
    else // If the buffer is full
    {
      MAX30102_STATE = PROCESS; // Move to data processing state
    }
    break;
  case PROCESS: // Calculating SpO2 and heart rate
    // Initialize variables needed for calculations
    maxim_heart_rate_and_oxygen_saturation(irBuffer,
                                           bufferLength,
                                           redBuffer,
                                           &spo2,
                                           &validSPO2,
                                           &heartRate,
                                           &validHeartRate);
    if (validSPO2 && spo2 != -999) // Check if SpO2 measurement is valid
    {
      Serial.print("SpO2: "); // Display the SpO2 value
      Serial.println(spo2);
    }
    else // If no finger detected
    {
      spo2 = 0;                            // Assign value 0 to SpO2 variable
      Serial.println("Finger not detected"); // Information on no finger detected
    }
    sampleIndex = 0;              // Reset the sample index for the next cycle
    lastSampleTime = currentTime; // Update the time of the last sample
    MAX30102_STATE = WAIT;        // Move to waiting state
    break;
  case WAIT:                                            // Waiting state
    if (currentTime - lastSampleTime >= SPO2_WAIT_TIME) // Check if the required waiting time has passed (4000ms)
    {
      MAX30102_STATE = INIT; // Return to initialization state to start a new measurement cycle
    }
    break;
  }
  return spo2;
}
// Global variables (MAX30102 sensor initialization flags)
bool MaxInitialized = false;
void publishAllSensorsData()
{

  struct timeval now;
  gettimeofday(&now, NULL);
  long long timestamp = (now.tv_sec * 1000LL + now.tv_usec / 1000); // Timestamp in milliseconds
  float temp = tempAndHumPublish();                                 // Read temperature and humidity values
  int heartBpm = heartRateDetection();                              // Read heart rate values
  int32_t spo2_value = MAX30102_SPO2_MEASUREMENT();                 // Read SpO2 values
  long irValue = PulseAndOxygenSensor.getIR();                      // Read IR diode values
  // Check presence of sensors
  bool maxPresence = (PulseAndOxygenSensor.readPartID() == 0x15); // Read MAX30102 sensor identifier
  int shtError = tempAndHumSensor.GetError();                     // Check for error from SHT35 sensor
  bool shtPresent = (shtError == 0);                              // Flag for presence of SHT35 sensor
  // Buffers
  char shtPayload[50];        // Buffer for SHT35 sensor presence attributes
  char maxPayload[50];        // Buffer for MAX30102 sensor presence attributes
  char attributesPayload[50]; // Buffer for finger presence attributes
  char payload[512];          // Buffer for measurement data
  // Check presence of MAX30102 and SHT35 sensors and publish attributes
  if (maxPresence && !MaxInitialized) {
    Serial.println("Initializing MAX30102 sensor.");
    maxSetup(); // Reinitialize MAX30102 sensor
    MaxInitialized = true; // Set initialization flag
    snprintf(maxPayload, sizeof(maxPayload), "{\"maxPresent\":true}");
    mqttClient.publish("v1/devices/me/attributes", maxPayload);
  } else if (!maxPresence) {
    Serial.println("MAX30102 is not connected.");
    snprintf(maxPayload, sizeof(maxPayload), "{\"maxPresent\":false}");
    MaxInitialized = false; // Reset initialization flag
    mqttClient.publish("v1/devices/me/attributes", maxPayload);
  } else {
    snprintf(maxPayload, sizeof(maxPayload), "{\"maxPresent\":true}");
    mqttClient.publish("v1/devices/me/attributes", maxPayload);
  }
  if (!shtPresent) {
    Serial.println("SHT35 is not connected.");
    snprintf(shtPayload, sizeof(shtPayload), "{\"shtPresent\":false}");
    mqttClient.publish("v1/devices/me/attributes", shtPayload);
  } else {
    snprintf(shtPayload, sizeof(shtPayload), "{\"shtPresent\":true}");
    mqttClient.publish("v1/devices/me/attributes", shtPayload);
  }
  // Check finger presence
  if (irValue < IR_TRESHOLD)
  {
    // Publish 'value' attribute as false, no finger detected
    snprintf(attributesPayload, sizeof(attributesPayload), "{\"value\":false}");
    // Publish measurement data
    snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"temperature\":%.2f, \"SPO2\":0, \"BPM\":0}}", timestamp, temp);
    if (!mqttClient.publish("v1/devices/me/attributes", attributesPayload) || !mqttClient.publish("v1/devices/me/telemetry", payload))
    {
      Serial.println("Failed to send attributes and measurement data");
    }
  }
  else
  {
    // Publish 'value' attribute as true, finger detected
    snprintf(attributesPayload, sizeof(attributesPayload), "{\"value\":true}");
    // Publish measurement data
    snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"temperature\":%.2f, \"SPO2\":%d, \"BPM\":%d}}", timestamp, temp, spo2_value, heartBpm);
    if (!mqttClient.publish("v1/devices/me/attributes", attributesPayload) || !mqttClient.publish("v1/devices/me/telemetry", payload))
    {
      Serial.println("Failed to send attributes and measurement data");
    }
  }
}

void publishAD()
{
  char payload[100];
  char ADAttrPayload[50]; // Additional buffer for finger presence attributes
  struct timeval now;
  gettimeofday(&now, NULL);
  long long timestamp = (now.tv_sec * 1000LL + now.tv_usec / 1000); // Timestamp in milliseconds
  ecg_value = analogRead(ECG_PIN);

  if (digitalRead(LO_MINUS_PIN) == 1 || digitalRead(LO_PLUS_PIN) == 1)
  {
    snprintf(ADAttrPayload, sizeof(ADAttrPayload), "{\"value2\":false}");
    snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"ECG\":%d}}", timestamp, 0);
  }
  else
  {
    snprintf(ADAttrPayload, sizeof(ADAttrPayload), "{\"value2\":true}");
    snprintf(payload, sizeof(payload), "{\"ts\":%lld,\"values\":{\"ECG\":%d}}", timestamp, ecg_value);
  }
  if (!mqttClient.publish("v1/devices/me/attributes", ADAttrPayload) || !mqttClient.publish("v1/devices/me/telemetry", payload))
  {
    Serial.println("Failed to send attributes");
  }
}

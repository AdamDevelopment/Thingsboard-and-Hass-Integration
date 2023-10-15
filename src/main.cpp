#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SHT3x.h>

// WiFi credentials
const char* ssid = "Nie mam wi-fi";
const char* password = "makaronzpomidorami";

// ThingsBoard MQTT broker settings
const char* mqttServer = "thingsboardrpi.duckdns.org";
const int mqttPort = 1883;
const char* mqttToken = "og3ms9bts9x8a9agm00c";

WiFiClient espClient;
PubSubClient client(espClient);
SHT3x Sensor_H(0x44);

void setup() {
  Serial.begin(115200);
  Sensor_H.Begin();
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32",mqttToken, NULL)) {
      Serial.println("Connected to MQTT");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      if (client.connect("ESP32",mqttToken, NULL)) {
        Serial.println("Reconnected to MQTT");
      } else {
        delay(5000);
      }
    }
  }
  Sensor_H.UpdateData();
  float temp = Sensor_H.GetTemperature();
  float hum = Sensor_H.GetRelHumidity();

  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", temp, hum);
  client.publish("v1/devices/me/telemetry", payload);
  Serial.println(temp);
  Serial.println(hum);
  delay(5000); // Send data every 5 seconds
}

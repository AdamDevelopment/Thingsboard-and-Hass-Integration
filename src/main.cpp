#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

const char* mqttServer = "thingsboardrpi.duckdns.org";
const int mqttPort = 1883;
const char* mqttUsername = "your_mqtt_username";
const char* mqttPassword = "your_mqtt_password";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  // Connect to MQTT
  reconnect();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle MQTT messages received here
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
      // Subscribe to MQTT topics if needed
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  // Add your data publishing logic here
}

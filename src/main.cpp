#include <WiFi.h>

const char* ssid = "Nie mam wi-fi";
const char* password = "makaronzpomidorami";
const char* thingsboardServer = "thingsboardrpi.duckdns.org";
const char* accessToken = "95qx4nyy0no8vig0ib0a";

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to Wi-Fi
  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  // Replace these values with your sensor data
  String data = "{ \"temperature\": 25.5, \"humidity\": 50.2 }"; // JSON format

  // Create an HTTP request
  String url = "/api/v1/" + String(accessToken) + "/telemetry"; // ThingsBoard telemetry endpoint

  // Send the HTTP POST request to ThingsBoard
  if (client.connect(thingsboardServer, 80)) {
    Serial.println("Connected to ThingsBoard");
    client.print("POST " + url + " HTTP/1.1\r\n");
    client.print("Host: " + String(thingsboardServer) + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(data.length()) + "\r\n");
    client.print("\r\n");
    client.print(data);
    client.stop();
    Serial.println("Data sent to ThingsBoard");
  } else {
    Serial.println("Unable to connect to ThingsBoard");
  }

  // Wait for a fixed interval before sending the next update
  delay(15000); // Send data every 15 seconds
}

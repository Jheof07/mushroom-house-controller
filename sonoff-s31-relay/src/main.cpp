#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ZTE_2.4G_jT5nRx_EXT";
const char* password = "4kjL9URf";

#define RELAY_PIN 12

ESP8266WebServer server(80);

void handleOn() {
  digitalWrite(RELAY_PIN, LOW);   // Sonoff relay is active LOW
  server.send(200, "text/plain", "Humidifier ON");
}

void handleOff() {
  digitalWrite(RELAY_PIN, HIGH);
  server.send(200, "text/plain", "Humidifier OFF");
}

void handleRoot() {
  server.send(200, "text/plain", "Sonoff S31 Humidifier Controller");
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Start OFF

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (MDNS.begin("humidifier")) {
    Serial.println("mDNS started: http://humidifier.local");
  }

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();
}

void loop() {
  server.handleClient();
  MDNS.update();
}

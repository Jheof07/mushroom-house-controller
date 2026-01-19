#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ZTE_2.4G_jT5nRx_EXT";
const char* password = "4kjL9URf";

#define RELAY_PIN 12   

ESP8266WebServer server(80);

// Logical control functions
void humidifierOn() {
  digitalWrite(RELAY_PIN, HIGH);   
  server.send(200, "text/plain", "Humidifier ON");
}

void humidifierOff() {
  digitalWrite(RELAY_PIN, LOW);  
  server.send(200, "text/plain", "Humidifier OFF");
}

void handleRoot() {
  server.send(200, "text/plain", "Sonoff S31 Humidifier Controller");
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  humidifierOff();   // Start safely OFF

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (MDNS.begin("humidifier")) {
    Serial.println("mDNS started: http://humidifier.local");
  }

  // URL routes
  server.on("/", handleRoot);
  server.on("/on", humidifierOn);    // Logical ON
  server.on("/off", humidifierOff);  // Logical OFF

  server.begin();
}

void loop() {
  server.handleClient();
  MDNS.update();
}

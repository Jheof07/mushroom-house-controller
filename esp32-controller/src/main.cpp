#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

const char* ssid = "ZTE_2.4G_jT5nRx_EXT";
const char* password = "4kjL9URf";

const char* sonoffHost = "humidifier.local";

float ON_TEMP  = 28.0;
float OFF_TEMP = 26.0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

bool humidifierState = false;

void controlHumidifier(bool turnOn) {
  HTTPClient http;
  String url = String("http://") + sonoffHost + (turnOn ? "/on" : "/off");

  http.begin(url);
  int httpCode = http.GET();
  http.end();

  humidifierState = turnOn;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  dht.begin();

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }

  if (MDNS.begin("mushroom-esp32")) {
    Serial.println("mDNS started");
  }

  lcd.clear();
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  float temp = dht.readTemperature();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temp,1);
  lcd.print("C");

  if (!humidifierState && temp >= ON_TEMP) {
    controlHumidifier(true);
  }
  if (humidifierState && temp <= OFF_TEMP) {
    controlHumidifier(false);
  }

  lcd.setCursor(0,1);
  lcd.print("Humidifier: ");
  lcd.print(humidifierState ? "ON " : "OFF");

  delay(3000);
}

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

#define ON_THRESHOLD  27.0
#define OFF_THRESHOLD 26.0

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Replace with your Sonoff S31 MAC address
uint8_t receiverMac[] = {0x48, 0x3F, 0xDA, 0x28, 0x2F, 0xDB};

typedef struct struct_message {
  float temperature;
  bool humidifierState;
} struct_message;

struct_message dataToSend;
bool currentState = false;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Temp Monitor");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    lcd.setCursor(0,0);
    lcd.print("DHT Error      ");
    delay(2000);
    return;
  }

  // Hysteresis logic
  if (temp >= ON_THRESHOLD) currentState = true;
  else if (temp <= OFF_THRESHOLD) currentState = false;

  dataToSend.temperature = temp;
  dataToSend.humidifierState = currentState;
  esp_now_send(receiverMac, (uint8_t *)&dataToSend, sizeof(dataToSend));

  // LCD Display
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temp,1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.print(currentState ? "ON " : "OFF");

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" | Humidifier: ");
  Serial.println(currentState ? "ON" : "OFF");

  delay(3000);
}

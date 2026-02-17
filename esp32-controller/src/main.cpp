#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>

#define DHTTYPE DHT11
#define DHTPIN 4

#define TEMP_ON_THRESHOLD  27.0
#define TEMP_OFF_THRESHOLD 26.0

#define HUM_ON_THRESHOLD  65.0
#define HUM_OFF_THRESHOLD 70.0

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Sonoff MAC Addresses
uint8_t receiverMac1[] = {0x48, 0x3F, 0xDA, 0x28, 0xA8, 0xF5};
uint8_t receiverMac2[] = {0x48, 0x3F, 0xDA, 0x28, 0x2F, 0xDB};

typedef struct struct_message {
  float temperature;
  float humidity;
  bool tempRelayState;
  bool humidifierRelayState;
} struct_message;

struct_message dataToSend;
bool tempRelayState = false;
bool humidityRelayState = false;

// Send Status Callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Fail");
  }
}

void addPeer(uint8_t *mac) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;     // auto channel
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Peer Added Successfully");
  } else {
    Serial.println("Failed to Add Peer");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize DHT
  dht.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // WiFI in Station Mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Add both receivers
  addPeer(receiverMac1);
  addPeer(receiverMac2);

  Serial.println("System Ready.");

}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    lcd.setCursor(0,0);
    lcd.print("DHT Error       ");
    Serial.println("DHT Read Failed");
    delay(2000);
    return;
  }

  // Temperature Hysteresis
  if (temp >= TEMP_ON_THRESHOLD) tempRelayState = true;
  else if (temp <= TEMP_OFF_THRESHOLD) tempRelayState = false;

  // Humidity Hysteresis
  if (hum <= HUM_ON_THRESHOLD) humidityRelayState = true;
  else if (hum >= HUM_OFF_THRESHOLD) humidityRelayState = false;

  // Prepare Data
  dataToSend.temperature = temp;
  dataToSend.humidity = hum;
  dataToSend.tempRelayState = tempRelayState;
  dataToSend.humidifierRelayState = humidityRelayState;

  //Send to Both Sonoff
  esp_err_t result1 = esp_now_send(receiverMac1, (uint8_t *)&dataToSend, sizeof(dataToSend));
  esp_err_t result2 = esp_now_send(receiverMac2, (uint8_t *)&dataToSend, sizeof(dataToSend));

  if (result1 != ESP_OK) Serial.println("Error sending to Receiver 1");
  if (result2 != ESP_OK) Serial.println("Error sending to Receiver 2");

  // LCD Display
  lcd.setCursor(0,0);
  lcd.print("T: ");
  lcd.print(temp,1);
  lcd.print((char)223);
  lcd.print("C H:");
  lcd.print(hum,0);
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.print(tempRelayState ? "ON " : "OFF");
  lcd.print(" H:");
  lcd.print(humidityRelayState ? "ON " : "OFF");

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print("  Hum: "); Serial.print(hum);
  Serial.print("  T_Relay: "); Serial.print(tempRelayState);
  Serial.print("  H_Relay: "); Serial.println(humidityRelayState);

  delay(3000);
}

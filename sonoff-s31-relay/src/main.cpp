#include <espnow.h>
#include <ESP8266WiFi.h>

#define RELAY_PIN 12

typedef struct struct_message {
  float temperature;
  bool humidifierState;
} struct_message;

struct_message incomingData;

void OnDataRecv(uint8_t *mac, uint8_t *incomingDataRaw, uint8_t len) {
  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

  digitalWrite(RELAY_PIN, incomingData.humidifierState ? HIGH : LOW);
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {}

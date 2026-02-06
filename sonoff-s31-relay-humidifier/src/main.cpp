#include <ESP8266WiFi.h>

extern "C" {
  #include <espnow.h>
}

#define RELAY_PIN 12

typedef struct struct_message {
  float temperature;
  float humidity;
  bool humidifierState;
  bool fanState;
} struct_message;

struct_message incomingData;

// MUST use this exact signature for ESP8266
void OnDataRecv(uint8_t *mac, uint8_t *incomingDataRaw, uint8_t len) {

  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

  digitalWrite(RELAY_PIN, incomingData.humidifierState ? HIGH : LOW);
}

void setup() {

  Serial.begin(115200);   // ⭐ Always add this for debugging
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();   // ⭐ Prevents ESP-NOW conflicts

  // ✅ ALWAYS check init result
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  // ❌ REMOVE THIS (old API)
  // esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Receiver Ready");
}

void loop() {}

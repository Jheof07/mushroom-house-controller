#include <ESP8266WiFi.h>
#include <espnow.h>

#define RELAY_PIN 12
#define TIMEOUT_MS 10000

typedef struct struct_message {
  float temperature;
  float humidity;
  bool tempRelayState;
  bool humidifierRelayState;
} struct_message;

struct_message incomingData;

unsigned long lastReceiveTime;
bool currentRelayState = false;

void OnDataRecv(uint8_t *mac, uint8_t *incomingDataRaw, uint8_t len) {

  if (len == sizeof(struct_message)) {

    memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

    bool newState = incomingData.humidifierRelayState;

    if (newState != currentRelayState) {
      digitalWrite(RELAY_PIN, newState ? HIGH : LOW);
      currentRelayState = newState;
    }

    lastReceiveTime = millis();

    Serial.print("Temp: ");
    Serial.print(incomingData.temperature);
    Serial.print("  Hum: ");
    Serial.print(incomingData.humidity);
    Serial.print("  Relay: ");
    Serial.println(newState);
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  lastReceiveTime = millis();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver Ready.");
}

void loop() {

  if (millis() - lastReceiveTime > TIMEOUT_MS) {

    if (currentRelayState == true) {
      digitalWrite(RELAY_PIN, LOW);
      currentRelayState = false;
      Serial.println("Failsafe: Relay OFF");
    }
  }
}

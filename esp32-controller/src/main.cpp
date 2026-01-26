#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>

#define DHTTYPE DHT11
#define DHTPIN 4
#define TEMP_ON_THRESHOLD 28.0
#define TEMP_OFF_THRESHOLD 26.0
#define HUMI_ON_THRESHOLD 70.0
#define HUMI_OFF_THRESHOLD 90.0

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
uint8_t sonoff_fan_MAC[6]= {0x48, 0x3E, 0xDA, 0x28, 0xA8, 0xF5};
uint8_t sonoff_humidifier_MAC[6]= {0x48, 0x3F, 0xDA, 0x28, 0xA8, 0xF5};

typedef struct struct_message {
  float temperature;
  float humidity;
  bool humidifierState;
  bool fanState;
} struct_message;

struct_message dataToSend_sonoff_fan;
struct_message dataToSend_sonoff_humidifier;

bool currentState_fan = false;
bool currentState_humidifier = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();

  WiFi.mode(WIFI_STA);

  if(esp_now_init() != ESP_OK){
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t sonoff_fan = {};
  memcpy(sonoff_fan.peer_addr, sonoff_fan_MAC, 6);
  sonoff_fan.channel = 0;
  sonoff_fan.encrypt = false;

  if(esp_now_add_peer(&sonoff_fan) != ESP_OK){
    Serial.println("Failed to add Sonoff Fan");
    return;
  }

  esp_now_peer_info_t sonoff_humidifier = {};
  memcpy(sonoff_humidifier.peer_addr, sonoff_humidifier_MAC, 6);
  sonoff_humidifier.channel = 0;
  sonoff_humidifier.encrypt = false;
  
  if(esp_now_add_peer(&sonoff_humidifier) != ESP_OK){
    Serial.println("Failed to add Sonoff Humidifier");
    return;
  }
}

void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if(isnan(temp) || isnan(humidity)){
    lcd.setCursor(0,0);
    lcd.print("DHT Error       ");
    Serial.println("DHT Read Failed");
    delay(2000);
    return;
  }

  //Hysterisis Logic
  if (temp >= TEMP_ON_THRESHOLD)
    currentState_fan = true;
  else if (temp <= TEMP_OFF_THRESHOLD)
    currentState_fan = false;

  if (humidity <= HUMI_ON_THRESHOLD)
    currentState_humidifier = true;
  else if (humidity >= HUMI_OFF_THRESHOLD)
    currentState_humidifier = false;

  // Sending to Sonoff Fan  
  dataToSend_sonoff_fan.temperature = temp;
  dataToSend_sonoff_fan.humidity = humidity;
  dataToSend_sonoff_fan.humidifierState = currentState_humidifier;
  dataToSend_sonoff_fan.fanState = currentState_fan;
  esp_now_send(sonoff_fan_MAC, (uint8_t*)&dataToSend_sonoff_fan, sizeof(dataToSend_sonoff_fan));


  // Sending to Sonoff Humidifier
  dataToSend_sonoff_humidifier.temperature = temp;
  dataToSend_sonoff_humidifier.humidity = humidity;
  dataToSend_sonoff_humidifier.humidifierState = currentState_humidifier;
  dataToSend_sonoff_humidifier.fanState = currentState_fan;
  esp_now_send(sonoff_humidifier_MAC, (uint8_t*)&dataToSend_sonoff_humidifier, sizeof(dataToSend_sonoff_humidifier));
  
  // To display in LCD
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humi: ");
  lcd.print(humidity, 1);
  lcd.print("%");

  // To print in the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temp, 1);
  Serial.print("°C");
  Serial.print("   ");
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");
  
  delay(2000);
  lcd.clear();
}


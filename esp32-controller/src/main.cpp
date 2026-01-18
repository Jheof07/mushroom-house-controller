#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mushroom House");
  lcd.setCursor(0, 1);
  lcd.print("Temp Monitor");
  delay(2000);
}

void loop() {
  float temperature = dht.readTemperature();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  Serial.print("Temperature: ");
  Serial.println(temperature);

  delay(2000);
}

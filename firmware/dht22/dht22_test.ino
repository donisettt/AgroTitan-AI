#include <DHT.h>

#define DHT_PIN 14
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);

  dht.begin();

  Serial.println();
  Serial.println("AgroTitan-AI DHT22 Test Started");
  Serial.println("DHT22 DATA Pin: GPIO14");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT22!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C");

    Serial.print(" | Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  delay(2000);
}
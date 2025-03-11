#include <DHT.h>

DHT dht(26, DHT22);
const int smokesensor = 4;  

void setup() {
  Serial.begin(115200);
  dht.begin(); 
}

void loop() {
  // Read MQ-135 analog value
  int digitalNumber = analogRead(smokesensor);

  // Read temperature & humidity from DHT22
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Print readings
  Serial.print("MQ-135 Value: ");
  Serial.println(digitalNumber);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" % ");  

  delay(2000);
}

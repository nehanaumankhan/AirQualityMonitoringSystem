#include <DHT.h>
DHT dht(32, DHT22); 
void setup(){
  dht.begin();
  delay(2000);
  Serial.begin(115200);
}
void loop(){
  //Start of Program 
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" % ");  
  delay(2000);
}
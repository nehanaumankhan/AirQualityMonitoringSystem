const int smokesensor = 4;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int digitalNumber = analogRead(smokesensor);
  Serial.print("MQ-135 Value: ");
  Serial.println(digitalNumber);
  delay(1000);
}

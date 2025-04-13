const int smokesensor = 33;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int digitalNumber = analogRead(smokesensor);
  Serial.print("MQ-135 Value: ");
  Serial.println(digitalNumber);
  delay(1000);
}

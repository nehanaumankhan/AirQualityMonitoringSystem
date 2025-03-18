#include <Arduino.h>
#include <DHT.h>
#include "PMS.h"

// DHT22 Setup
#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Setup
const int smokesensor = 33;

// PMS5003 Setup
#define PMS_READ_INTERVAL 9  // Interval between readings
PMS pms(Serial2);  // Use Serial2 for PMS5003 communication

// Initialize the PMS5003 sensor
bool pms5003_init() {
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Start Serial2 (RX2=16, TX2=17)
  delay(1000);  // Wait for the sensor to initialize
  return true;
}

// Read data from the PMS5003 sensor
bool pms5003_read(uint16_t *pmSp1_0, uint16_t *pmSp2_5, uint16_t *pmSp10_0,
                  uint16_t *pmAe1_0, uint16_t *pmAe2_5, uint16_t *pmAe10_0) {
  PMS::DATA data;
  while (Serial2.available()) Serial2.read();
  if (pms.readUntil(data, 2U * PMS::SINGLE_RESPONSE_TIME)) {
    *pmSp1_0 = data.PM_SP_UG_1_0;
    *pmSp2_5 = data.PM_SP_UG_2_5;
    *pmSp10_0 = data.PM_SP_UG_10_0;
    *pmAe1_0 = data.PM_AE_UG_1_0;
    *pmAe2_5 = data.PM_AE_UG_2_5;
    *pmAe10_0 = data.PM_AE_UG_10_0;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  if (pms5003_init()) {
    Serial.println("PMS5003 initialized successfully!");
  } else {
    Serial.println("Failed to initialize PMS5003!");
  }
}

void loop() {
  // Read MQ-135 analog value
  int digitalNumber = analogRead(smokesensor);

  // Read temperature & humidity from DHT22
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read PMS5003 sensor data
  uint16_t pmSp1_0, pmSp2_5, pmSp10_0, pmAe1_0, pmAe2_5, pmAe10_0;
  if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    Serial.println("\nPMS5003 Data:");
    Serial.printf("PM1.0 (Standard): %d µg/m³\n", pmSp1_0);
    Serial.printf("PM2.5 (Standard): %d µg/m³\n", pmSp2_5);
    Serial.printf("PM10.0 (Standard): %d µg/m³\n", pmSp10_0);
    Serial.printf("PM1.0 (Environmental): %d µg/m³\n", pmAe1_0);
    Serial.printf("PM2.5 (Environmental): %d µg/m³\n", pmAe2_5);
    Serial.printf("PM10.0 (Environmental): %d µg/m³\n", pmAe10_0);
  } else {
    Serial.println("Failed to read data from PMS5003!");
  }

  // Print MQ-135 and DHT22 readings
  Serial.println("\nSensor Readings:");
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

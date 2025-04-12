#include <Arduino.h>
#include "PMS.h"

// Pin Definitions
#define PMS_READ_INTERVAL 9  // Interval between readings
#define PMS_READ_DELAY 1     // Delay before reading data

// Global Variables
uint8_t pms_tick_count = PMS_READ_INTERVAL;
PMS pms(Serial2);  // Use Serial2 for communication

/***************************************************/
// Initialize the PMS5003 sensor
bool pms5003_init(void) {
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Start Serial2 (RX2=16, TX2=17)
  delay(1000);  // Wait for the sensor to initialize
  pms_tick_count = PMS_READ_INTERVAL;  // Reset tick count
  return true;
}

/***************************************************/
// Read data from the PMS5003 sensor
bool pms5003_read(uint16_t *pmSp1_0, uint16_t *pmSp2_5, uint16_t *pmSp10_0,
                  uint16_t *pmAe1_0, uint16_t *pmAe2_5, uint16_t *pmAe10_0) {
  bool result = false;

  // Validate input pointers
  if ((pmSp1_0 == nullptr) || (pmSp2_5 == nullptr) || (pmSp10_0 == nullptr) ||
      (pmAe1_0 == nullptr) || (pmAe2_5 == nullptr) || (pmAe10_0 == nullptr)) {
    return false;  // Invalid pointers
  }

  PMS::DATA data;

  // Clear the serial buffer
  while (Serial2.available()) {
    Serial2.read();
  }

  // Read data from the sensor
  if (pms.readUntil(data, 2U * PMS::SINGLE_RESPONSE_TIME)) {
    // Assign values to the output pointers
    *pmSp1_0 = data.PM_SP_UG_1_0;
    *pmSp2_5 = data.PM_SP_UG_2_5;
    *pmSp10_0 = data.PM_SP_UG_10_0;
    *pmAe1_0 = data.PM_AE_UG_1_0;
    *pmAe2_5 = data.PM_AE_UG_2_5;
    *pmAe10_0 = data.PM_AE_UG_10_0;
    result = true;  // Data read successfully
  }

  return result;
}

/***************************************************/
// Setup function
void setup() {
  Serial.begin(115200);  // Start Serial Monitor for debugging
  if (pms5003_init()) {
    Serial.println("PMS5003 initialized successfully!");
  } else {
    Serial.println("Failed to initialize PMS5003!");
  }
}

/***************************************************/
// Main loop
void loop() {
  uint16_t pmSp1_0, pmSp2_5, pmSp10_0, pmAe1_0, pmAe2_5, pmAe10_0;

  if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    // Print the data to the Serial Monitor
    Serial.println("PMS5003 Data:");
    Serial.printf("PM1.0 (Standard): %d µg/m³\n", pmSp1_0);
    Serial.printf("PM2.5 (Standard): %d µg/m³\n", pmSp2_5);
    Serial.printf("PM10.0 (Standard): %d µg/m³\n", pmSp10_0);
    Serial.printf("PM1.0 (Environmental): %d µg/m³\n", pmAe1_0);
    Serial.printf("PM2.5 (Environmental): %d µg/m³\n", pmAe2_5);
    Serial.printf("PM10.0 (Environmental): %d µg/m³\n", pmAe10_0);
  } else {
    Serial.println("Failed to read data from PMS5003!");
  }

  delay(2000);  // Wait before the next reading
}

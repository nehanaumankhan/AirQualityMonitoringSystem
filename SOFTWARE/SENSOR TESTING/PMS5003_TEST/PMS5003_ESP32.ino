#include <HardwareSerial.h>

HardwareSerial pmsSerial(2); // Use UART2 (GPIO16 = RX2, GPIO17 = TX2 by default)

uint8_t buffer[32]; // Buffer to store sensor data

void setup() {
    Serial.begin(115200); // Start Serial Monitor
    pmsSerial.begin(9600, SERIAL_8N1, 16, 17); // PMS5003 uses 9600 baud rate, GPIO16 as RX, GPIO17 as TX
}

void loop() {
    if (pmsSerial.available() >= 32) { // PMS5003 sends 32-byte data frame
        for (int i = 0; i < 32; i++) {
            buffer[i] = pmsSerial.read();
        }

        if (buffer[0] == 0x42 && buffer[1] == 0x4D) { // Check header bytes 'BM'
            uint16_t pm1_0 = (buffer[10] << 8) | buffer[11];
            uint16_t pm2_5 = (buffer[12] << 8) | buffer[13];
            uint16_t pm10 = (buffer[14] << 8) | buffer[15];

            Serial.print("PM1.0: "); Serial.print(pm1_0);
            Serial.print(" µg/m³, PM2.5: "); Serial.print(pm2_5);
            Serial.print(" µg/m³, PM10: "); Serial.println(pm10);
        }
    }
    delay(1000);
}
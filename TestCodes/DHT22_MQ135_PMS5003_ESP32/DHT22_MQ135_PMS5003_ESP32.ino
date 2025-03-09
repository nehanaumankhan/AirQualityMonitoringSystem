#include <HardwareSerial.h>
#include <DHT.h>

// PMS5003 Serial Port connections
#define RXD2 16 // PMS5003 TX → ESP32 RX
#define TXD2 17 // PMS5003 RX → ESP32 TX

// MQ135 & DHT22 Sensors
#define MQ135_PIN 4
#define DHT_PIN 26
#define DHT_TYPE DHT22

// Initialize Serial & Sensors
HardwareSerial pmsSerial(2);
DHT dht(DHT_PIN, DHT_TYPE);

// PMS5003 Data Structure
struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
};
struct pms5003data data;

// Function to wake up PMS5003
void wakeUpPMS5003() {
    uint8_t wakeupCmd[] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x73}; 
    pmsSerial.write(wakeupCmd, sizeof(wakeupCmd));
    delay(1000);
}

// Function to read PMS5003 data
boolean readPMSdata(Stream *s) {
    if (!s->available()) {
        return false;
    }

    if (s->peek() != 0x42) { // Check start byte
        s->read();
        return false;
    }

    if (s->available() < 32) {
        return false;
    }

    uint8_t buffer[32];
    uint16_t sum = 0;
    s->readBytes(buffer, 32);

    // Checksum calculation
    for (uint8_t i = 0; i < 30; i++) {
        sum += buffer[i];
    }

    uint16_t buffer_u16[15];
    for (uint8_t i = 0; i < 15; i++) {
        buffer_u16[i] = buffer[2 + i * 2 + 1];
        buffer_u16[i] += (buffer[2 + i * 2] << 8);
    }

    memcpy((void *)&data, (void *)buffer_u16, 30);

    if (sum != data.checksum) {
        Serial.println("⚠️ PMS5003 Checksum Failure");
        return false;
    }
    return true;
}

void setup() {
    Serial.begin(115200);
    pmsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
    dht.begin();

    wakeUpPMS5003(); // Wake up PMS5003
    delay(3000); // Allow sensors to stabilize
}

void loop() {
    Serial.println("\n--- SENSOR READINGS ---");

    // Wake up PMS5003 every 60 seconds to prevent sleep
    static unsigned long lastWakeupTime = 0;
    if (millis() - lastWakeupTime > 60000) {
        wakeUpPMS5003();
        lastWakeupTime = millis();
    }

    // Read PMS5003 Data
    if (readPMSdata(&pmsSerial)) {
        Serial.println("---------------------------------------");
        Serial.println("Concentration Units (standard)");
        Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
        Serial.print("\tPM 2.5: "); Serial.print(data.pm25_standard);
        Serial.print("\tPM 10: "); Serial.println(data.pm100_standard);
        Serial.println("---------------------------------------");
    } else {
        Serial.println("⚠️ PMS5003 Data Not Available! (Check Wiring & Power)");
    }

    // Read MQ135 Sensor Data
    int mq135Value = analogRead(MQ135_PIN);
    Serial.print("MQ-135 Value: ");
    Serial.println(mq135Value);

    // Read Temperature & Humidity from DHT22
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity)) {
        Serial.println("⚠️ Failed to read from DHT22 Sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.print(" C ");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" % ");  
    }

    delay(2000);
}

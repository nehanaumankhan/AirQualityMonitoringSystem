#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <time.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DHTPIN 33
#define DHTTYPE DHT11
#define MQ135_PIN 32

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int mq135Value = 0;
int pm1_0Value = 0;
int pm2_5Value = 0;
int pm10_0Value = 0;

unsigned long otaUpdatePrevMillis = 0;
const unsigned long otaUpdateInterval = 1 * 60 * 1000;

#define API_KEY "AIzaSyBPexHJP0j9EArRWKPDbZecvkjBQsZxD6I"
#define DATABASE_URL "https://airqualitymonitoringsyst-6c370-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String databasePath = "/sensor_data";
unsigned long elapsedMillis = 0;
const unsigned long update_interval = 10000;
bool isAuthenticated = false;

WiFiClient wifiClient;

void Wifi_Init() {
  WiFiManager wm;
  if (!wm.autoConnect("AQMS_AAN", "12345678")) {
    Serial.println("Failed to connect and hit timeout. Restarting...");
    delay(3000);
    ESP.restart();
  }
  Serial.println("Connected to WiFi successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void firebase_init() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);

  Serial.println("------------------------------------");
  Serial.println("Signing up new user...");

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign-up successful");
    isAuthenticated = true;
  } else {
    Serial.printf("Sign-up failed: %s\n", config.signer.signupError.message.c_str());
    isAuthenticated = false;
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
}

void readPMS5003() {
  const int HEADER_HIGH = 0x42;
  const int HEADER_LOW = 0x4D;

  if (Serial2.available() >= 32) {
    if (Serial2.read() == HEADER_HIGH && Serial2.read() == HEADER_LOW) {
      uint8_t frame[30];
      uint16_t checksum = HEADER_HIGH + HEADER_LOW;

      for (int i = 0; i < 30; i++) {
        frame[i] = Serial2.read();
        checksum += frame[i];
      }

      uint16_t receivedChecksum = (frame[28] << 8) + frame[29];
      if (checksum == receivedChecksum) {
        pm1_0Value  = (frame[4] << 8) + frame[5];  // PM1.0
        pm2_5Value  = (frame[6] << 8) + frame[7];  // PM2.5
        pm10_0Value = (frame[8] << 8) + frame[9];  // PM10
        Serial.printf("PM1.0: %d µg/m³\n", pm1_0Value);
        Serial.printf("PM2.5: %d µg/m³\n", pm2_5Value);
        Serial.printf("PM10: %d µg/m³\n", pm10_0Value);
      } else {
        Serial.println("Checksum mismatch in PMS5003 data");
      }
    }
  }
}

void updateSensorReadings() {
  Serial.println("------------------------------------");
  Serial.println("Reading Sensor data...");

  float newTemp = dht.readTemperature();
  float newHumidity = dht.readHumidity();
  int newMQ135 = analogRead(MQ135_PIN);
  readPMS5003();

  if (!isnan(newTemp) && !isnan(newHumidity)) {
    temperature = newTemp;
    humidity = newHumidity;
    mq135Value = newMQ135;
    Serial.printf("Temperature: %.2f°C\n", temperature);
    Serial.printf("Humidity: %.2f%%\n", humidity);
    Serial.printf("MQ135 Value: %d\n", mq135Value);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

String getFormattedTimestamp() {
  time_t now = Firebase.getCurrentTime();
  if (now > 0) {
    now += 5 * 3600;  // UTC+5
    struct tm *timeinfo = gmtime(&now);

    char isoBuffer[25];
    strftime(isoBuffer, sizeof(isoBuffer), "%Y-%m-%dT%H:%M:%S+05:00", timeinfo);

    char readableBuffer[50];
    strftime(readableBuffer, sizeof(readableBuffer), "%A, %d %B %Y %H:%M:%S", timeinfo);

    Serial.print("Timestamp (ISO 8601, PKT): ");
    Serial.println(isoBuffer);
    Serial.print("Timestamp (Readable): ");
    Serial.println(readableBuffer);

    return String(isoBuffer);
  }
  return "0000-00-00T00:00:00+05:00";
}

void uploadSensorData() {
  if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
    elapsedMillis = millis();
    updateSensorReadings();

    if (isnan(temperature) || isnan(humidity)) return;

    String timestamp = getFormattedTimestamp();
    if (timestamp == "0000-00-00T00:00:00+05:00") {
      Serial.println("Failed to get current time from Firebase");
      return;
    }

    FirebaseJson sensorData;
    sensorData.set("temperature", temperature);
    sensorData.set("humidity", humidity);
    sensorData.set("mq135", mq135Value);
    sensorData.set("pm1_0", pm1_0Value);
    sensorData.set("pm2_5", pm2_5Value);
    sensorData.set("pm10_0", pm10_0Value);
    sensorData.set("timestamp", timestamp);

    if (Firebase.RTDB.pushJSON(&fbdo, databasePath.c_str(), &sensorData)) {
      Serial.println("Data pushed successfully!");
      Serial.print("PATH: ");
      Serial.println(fbdo.dataPath());
      Serial.print("TYPE: ");
      Serial.println(fbdo.dataType());
      Serial.print("VALUE: ");
      printResult(fbdo);
      Serial.println("------------------------------------");

      String key = fbdo.pushName();
      Serial.print("Unique key for this entry: ");
      Serial.println(key);
    } else {
      Serial.println("Data push FAILED");
      Serial.print("REASON: ");
      Serial.println(fbdo.errorReason());
      Serial.println("------------------------------------");
    }
  }
}

String getChipId() {
  String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  ChipIdHex += String((uint32_t)(ESP.getEfuseMac()), HEX);
  return ChipIdHex;
}

void update() {
  String url = "http://otadrive.com/deviceapi/update?";
  url += "k=1c98d716-c478-4e73-9c5c-44b1dd1f35fd";  // api key
  url += "&v=1.0.0.3";  // UPDATED version
  url += "&s=" + getChipId();

  WiFiClient client;
  httpUpdate.update(client, url, "1.0.0.3");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // PMS5003 RX=16, TX=17
  Wifi_Init();
  firebase_init();
  dht.begin();
  pinMode(MQ135_PIN, INPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && millis() - otaUpdatePrevMillis >= otaUpdateInterval) {
    update();
    otaUpdatePrevMillis = millis();
  }
  Serial.print("Time since OTA call: ");
  Serial.print((millis() - otaUpdatePrevMillis) / 60000);
  Serial.println(" minute(s)\nFirmware Version 1.0.0.3 successfully uploaded ALHAMDULILLAH!");

  uploadSensorData();

}
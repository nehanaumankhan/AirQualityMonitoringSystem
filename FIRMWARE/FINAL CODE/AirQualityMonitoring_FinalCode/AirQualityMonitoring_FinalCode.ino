#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include <time.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PMS.h>

// DHT22 Setup
#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Setup
#define MQ135_PIN 33 // MQ135 sensor connected to GPIO33 (ADC1)

// PMS5003 Setup
PMS pms(Serial2);  // Use Serial2 for PMS5003 communication
#define PMS_READ_INTERVAL 9  // Interval between readings

float temperature = 0;
float humidity = 0;
int mq135Value = 0; // Variable to store MQ135 sensor reading
uint16_t pm1_0 = 0, pm2_5 = 0, pm10_0 = 0; // Variables for PMS5003 data

#define WIFI_SSID "HUAWEI-2.4G"
#define WIFI_PASSWORD "malikza01"

#define API_KEY "AIzaSyBPexHJP0j9EArRWKPDbZecvkjBQsZxD6I"
#define DATABASE_URL "https://airqualitymonitoringsyst-6c370-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String databasePath = "/sensor_data";
unsigned long elapsedMillis = 0;
unsigned long update_interval = 10000;
bool isAuthenticated = false;

void Wifi_Init() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
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

bool pms5003_init() {
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Start Serial2 (RX2=16, TX2=17)
  delay(1000);  // Wait for the sensor to initialize
  return true;
}

bool pms5003_read(uint16_t *pm1_0, uint16_t *pm2_5, uint16_t *pm10_0) {
  PMS::DATA data;
  while (Serial2.available()) Serial2.read();
  if (pms.readUntil(data, 2U * PMS::SINGLE_RESPONSE_TIME)) {
    *pm1_0 = data.PM_SP_UG_1_0;
    *pm2_5 = data.PM_SP_UG_2_5;
    *pm10_0 = data.PM_SP_UG_10_0;
    return true;
  }
  return false;
}

void updateSensorReadings() {
    Serial.println("------------------------------------");
    Serial.println("Reading Sensor data...");

    // Read temperature and humidity from DHT22
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read MQ135 sensor value
    mq135Value = analogRead(MQ135_PIN);

    // Read PMS5003 sensor data
    if (pms5003_read(&pm1_0, &pm2_5, &pm10_0)) {
        Serial.println("PMS5003 Data:");
        Serial.printf("PM1.0: %d µg/m³\n", pm1_0);
        Serial.printf("PM2.5: %d µg/m³\n", pm2_5);
        Serial.printf("PM10.0: %d µg/m³\n", pm10_0);
    } else {
        Serial.println("Failed to read data from PMS5003!");
    }

    // Check if sensor readings are valid
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    // Print sensor readings to Serial Monitor
    Serial.printf("Temperature: %.2f°C\n", temperature);
    Serial.printf("Humidity: %.2f%%\n", humidity);
    Serial.printf("MQ135 Value: %d\n", mq135Value);
}

String getFormattedTimestamp() {
    time_t now = Firebase.getCurrentTime();
    if (now > 0) {
        struct tm *timeinfo = gmtime(&now);
        char buffer[25]; // Enough space for formatted string
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
        return String(buffer);
    }
    return "0000-00-00T00:00:00Z";  // Fallback timestamp if failed
}

void uploadSensorData() {
    if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
        elapsedMillis = millis();
        updateSensorReadings();

        String timestamp = getFormattedTimestamp();
        if (timestamp == "0000-00-00T00:00:00Z") {
            Serial.println("Failed to get current time from Firebase");
            return;
        }

        // Create a JSON object with sensor data
        FirebaseJson sensorData;
        sensorData.set("temperature", temperature);
        sensorData.set("humidity", humidity);
        sensorData.set("mq135", mq135Value); // Add MQ135 sensor value
        sensorData.set("pm1_0", pm1_0);     // Add PM1.0 value
        sensorData.set("pm2_5", pm2_5);     // Add PM2.5 value
        sensorData.set("pm10_0", pm10_0);   // Add PM10.0 value
        sensorData.set("timestamp", timestamp);

        // Define the Firebase path
        String nodePath = databasePath + "/" + timestamp;

        // Upload data to Firebase
        if (Firebase.setJSON(fbdo, nodePath.c_str(), sensorData)) {
            Serial.println("Data upload successful!");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.print("VALUE: ");
            printResult(fbdo);
            Serial.println("------------------------------------");
        } else {
            Serial.println("Data upload FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
            Serial.println("------------------------------------");
        }
    }
}

void setupOTA() {
    // Set the hostname for the ESP32 (optional)
    ArduinoOTA.setHostname("esp32-air-quality-monitor");

    // Define OTA event handlers
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    // Start OTA service
    ArduinoOTA.begin();
    Serial.println("OTA Initialized");
}

void setup() {
    Serial.begin(115200);
    Wifi_Init();
    firebase_init();
    dht.begin();
    pinMode(MQ135_PIN, INPUT); // Initialize MQ135 sensor pin as input
    pms5003_init(); // Initialize PMS5003 sensor
    setupOTA(); // Initialize OTA
}

void loop() {
    ArduinoOTA.handle(); // Handle OTA updates
    uploadSensorData();  // Upload sensor data to Firebase
    delay(1000);
}
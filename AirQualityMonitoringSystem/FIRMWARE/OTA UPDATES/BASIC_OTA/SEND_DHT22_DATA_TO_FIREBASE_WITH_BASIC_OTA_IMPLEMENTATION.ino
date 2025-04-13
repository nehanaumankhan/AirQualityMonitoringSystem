#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include <time.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

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

void updateSensorReadings() {
    Serial.println("------------------------------------");
    Serial.println("Reading Sensor data...");
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    Serial.printf("Temperature: %.2f°C\n", temperature);
    Serial.printf("Humidity: %.2f%%\n", humidity);
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

        FirebaseJson sensorData;
        sensorData.set("temperature", temperature);
        sensorData.set("humidity", humidity);
        sensorData.set("timestamp", timestamp);

        String nodePath = databasePath + "/" + timestamp;
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

    // Set a password for OTA updates (optional)
    // ArduinoOTA.setPassword("admin");

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
    setupOTA(); // Initialize OTA
}

void loop() {
    ArduinoOTA.handle(); // Handle OTA updates
    uploadSensorData();  // Upload sensor data to Firebase
    delay(1000);
}
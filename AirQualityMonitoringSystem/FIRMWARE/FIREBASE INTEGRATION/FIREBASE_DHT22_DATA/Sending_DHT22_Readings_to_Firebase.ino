#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"
#include <time.h>  // Include time library for formatting

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

void setup() {
    Serial.begin(115200);
    Wifi_Init();
    firebase_init();
    dht.begin();
}

void loop() {
    uploadSensorData();
    delay(1000);
}
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"

#define DHTPIN 26
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

String databasePath = ""; 
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
    Serial.println("Sign up new user...");
    
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Success");
        isAuthenticated = true;
        databasePath = "/sensor_data"; // Path to store sensor data
    } else {
        Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str());
        isAuthenticated = false;
    }
    
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
}

void updateSensorReadings() {
    Serial.println("------------------------------------");
    Serial.println("Reading Sensor data ...");
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    // Check if any reads failed and exit early (to try again).
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    Serial.printf("Temperature reading: %.2f \n", temperature);
    Serial.printf("Humidity reading: %.2f \n", humidity);
}

void setup() {
    // Initialise serial communication for local diagnostics
    Serial.begin(115200);
    // Initialise Connection with location WiFi
    Wifi_Init();
    // Initialise firebase configuration and signup anonymously
    firebase_init();
    // Initialise DHT library
    dht.begin();
}

void uploadSensorData() {
    if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
        elapsedMillis = millis();
        updateSensorReadings();

        // Get the current timestamp
        if (Firebase.getCurrentTime(fbdo)) {
            String timestamp = fbdo.to<String>();

            // Create a JSON object for the sensor data
            FirebaseJson sensorData;
            sensorData.set("temperature", temperature);
            sensorData.set("humidity", humidity);
            sensorData.set("timestamp", timestamp);

            // Upload the sensor data to Firebase
            String nodePath = databasePath + "/" + timestamp; // Use timestamp as the node name
            if (Firebase.setJSON(fbdo, nodePath.c_str(), sensorData)) {
                Serial.println("PASSED"); 
                Serial.println("PATH: " + fbdo.dataPath());
                Serial.println("TYPE: " + fbdo.dataType());
                Serial.println("ETag: " + fbdo.ETag());
                Serial.print("VALUE: ");
                printResult(fbdo); // See addons/RTDBHelper.h
                Serial.println("------------------------------------");
                Serial.println();
            } else {
                Serial.println("FAILED");
                Serial.println("REASON: " + fbdo.errorReason());
                Serial.println("------------------------------------");
                Serial.println();
            }
        } else {
            Serial.println("Failed to get current time");
        }
    }
}

void loop() {
    uploadSensorData();
}
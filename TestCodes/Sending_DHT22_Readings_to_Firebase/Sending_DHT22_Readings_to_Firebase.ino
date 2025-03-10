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
FirebaseJson temperature_json;
FirebaseJson humidity_json;

#define DEVICE_UID "1X"

#define WIFI_SSID "HUAWEI-2.4G"
#define WIFI_PASSWORD "malikza01"

#define API_KEY "AIzaSyBPexHJP0j9EArRWKPDbZecvkjBQsZxD6I"
#define DATABASE_URL "https://airqualitymonitoringsyst-6c370-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String device_location = "ESP32_1";  // Define device location
String databasePath = ""; 
String fuid = ""; 
unsigned long elapsedMillis = 0; 
unsigned long update_interval = 10000; 
int count = 0; 
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
        databasePath = "/" + device_location;
        fuid = auth.token.uid.c_str();
    } else {
        Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str());
        isAuthenticated = false;
    }
    
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
}

void updateSensorReadings(){
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
    temperature_json.set("value", temperature);
    humidity_json.set("value", humidity);
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
    // Initialise temperature and humidity json data
    temperature_json.add("deviceuid", DEVICE_UID);
    temperature_json.add("name", "DHT22-Temp");
    temperature_json.add("type", "Temperature");
    temperature_json.add("location", device_location);
    temperature_json.add("value", temperature);
    // Print out initial temperature values
    String jsonStr;
    temperature_json.toString(jsonStr, true);
    Serial.println(jsonStr);
    humidity_json.add("deviceuid", DEVICE_UID);
    humidity_json.add("name", "DHT22-Hum");
    humidity_json.add("type", "Humidity");
    humidity_json.add("location", device_location);
    humidity_json.add("value", humidity);
    // Print out initial humidity values
    String jsonStr2;
    humidity_json.toString(jsonStr2, true);
    Serial.println(jsonStr2);
}

void uploadSensorData() {
    if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
        elapsedMillis = millis();
        updateSensorReadings();

        // Get the current timestamp
        if (Firebase.getCurrentTime(fbdo)) {
            String timestamp = fbdo.to<String>();
            temperature_json.set("timestamp", timestamp);
            humidity_json.set("timestamp", timestamp);
        } else {
            Serial.println("Failed to get current time");
            return;
        }

        String temperature_node = databasePath + "/temperature"; 
        String humidity_node = databasePath + "/humidity";

        if (Firebase.setJSON(fbdo, temperature_node.c_str(), temperature_json)) {
            Serial.println("PASSED"); 
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.println("ETag: " + fbdo.ETag());
            Serial.print("VALUE: ");
            printResult(fbdo); //see addons/RTDBHelper.h
            Serial.println("------------------------------------");
            Serial.println();
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
        }

        if (Firebase.setJSON(fbdo, humidity_node.c_str(), humidity_json)) {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.println("ETag: " + fbdo.ETag()); 
            Serial.print("VALUE: ");
            printResult(fbdo); //see addons/RTDBHelper.h
            Serial.println("------------------------------------");
            Serial.println();
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
        }
    }
}

void loop() {
    uploadSensorData();
}
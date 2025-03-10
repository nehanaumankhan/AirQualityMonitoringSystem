#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

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

void database_test() {
    if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
        elapsedMillis = millis();
        Serial.println("------------------------------------");
        Serial.println("Set int test...");
        
        String node = databasePath + "/value";  // Fixed path issue
        if (Firebase.set(fbdo, node.c_str(), count++)) {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.println("ETag: " + fbdo.ETag());
            Serial.print("VALUE: ");
            printResult(fbdo);
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

void setup() {
    Serial.begin(115200);
    Wifi_Init();
    firebase_init();
}

void loop() {
    database_test();
}
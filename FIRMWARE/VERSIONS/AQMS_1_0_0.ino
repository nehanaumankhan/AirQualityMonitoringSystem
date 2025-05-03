#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <WiFiManager.h>  // Added
#include <time.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
unsigned long otaUpdatePrevMillis = 0;
const unsigned long otaUpdateInterval = 1 * 60 * 1000;

// Firebase credentials
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
  
  // Try auto connect
  if (!wm.autoConnect("AQMS_AAN", "12345678")) {  // SSID and Password for configuration portal
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
  // config.time_zone = "Asia/Karachi";  // Pakistan Standard Time

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
  float newTemp = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (!isnan(newTemp) && !isnan(newHumidity)) {
    temperature = newTemp;
    humidity = newHumidity;
    Serial.printf("Temperature: %.2f°C\n", temperature);
    Serial.printf("Humidity: %.2f%%\n", humidity);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

String getFormattedTimestamp() {
  time_t now = Firebase.getCurrentTime();
  if (now > 0) {
    now += 5 * 3600;  // Add 5 hours for UTC+5 (Pakistan)
    struct tm *timeinfo = gmtime(&now);

    char isoBuffer[25];
    strftime(isoBuffer, sizeof(isoBuffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);

    char readableBuffer[50];
    strftime(readableBuffer, sizeof(readableBuffer), "%A, %d %B %Y %H:%M:%S", timeinfo);

    Serial.print("Timestamp (ISO 8601): ");
    Serial.println(isoBuffer);
    Serial.print("Timestamp (Readable): ");
    Serial.println(readableBuffer);

    return String(isoBuffer);  // return ISO 8601 for Firebase logging
  }
  return "0000-00-00T00:00:00Z";
}

void uploadSensorData() {
  if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready()) {
    elapsedMillis = millis();
    updateSensorReadings();

    if (isnan(temperature) || isnan(humidity)) return;

    String timestamp = getFormattedTimestamp();
    if (timestamp == "0000-00-00T00:00:00Z") {
      Serial.println("Failed to get current time from Firebase");
      return;
    }

    FirebaseJson sensorData;
    sensorData.set("temperature", temperature);
    sensorData.set("humidity", humidity);
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

String getChipId(){
  String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  ChipIdHex += String((uint32_t)(ESP.getEfuseMac()), HEX);
  return ChipIdHex;
}
void update(){
  String url = "http://otadrive.com/deviceapi/update?";
  url += "bf508c6b-8638-415f-a52f-b48d4188d172";  // api key
  url += "&v=1.0.0.0"; // device version
  url += "&s=" + getChipId();

  WiFiClient client;
  httpUpdate.update(client, url, "1.0.0.0");
}
void setup() {
  Serial.begin(115200);
  Wifi_Init();    // WiFiManager based WiFi setup
  firebase_init();
  dht.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && millis() - otaUpdatePrevMillis >= otaUpdateInterval) {
    update();
    otaUpdatePrevMillis = millis(); // Reset the timer
  }
  Serial.print("Time since OTA call: ");
  Serial.print((millis() - otaUpdatePrevMillis)/60000);
  Serial.println(" minute(s)");
  Serial.println();
  Serial.println("Version 2 successfully uploaded alhamdulillah!");

  uploadSensorData();
  delay(1000);
}
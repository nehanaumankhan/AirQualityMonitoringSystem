# Air Quality Monitoring System

This repository contains the code and documentation for an Air Quality Monitoring System built using the ESP32 microcontroller and various environmental sensors. The system collects data from sensors (DHT22, MQ135, and PMS5003) and sends it to a Firebase Realtime Database for real-time monitoring. Over-the-Air (OTA) updates are also supported for remote firmware updates.

---

## Features
- **Sensor Integration**: Reads data from DHT22 (Temperature and Humidity), MQ135 (Air Quality), and PMS5003 (PM2.5) sensors.
- **Firebase Integration**: Sends sensor data to a Firebase Realtime Database with fields for Temperature, Humidity, MQ135 Value, PM2.5 Value, and Timestamps.
- **Over-the-Air (OTA) Updates**: Supports OTA programming for remote firmware updates.
- **Mobile Application**: A Flutter-based mobile application (work in progress) for real-time data visualization and control.

---

## Hardware Components
1. **Microcontroller**: ESP32 (ESP-WROOM-32)
2. **Sensors**:
   - DHT22 (Temperature and Humidity Sensor)
   - MQ135 (Air Quality Sensor)
   - PMS5003 (PM2.5 Sensor)
3. **Additional Components**:
   - Breadboard and Jumper Wires (for prototyping)
   - Resistors and Capacitors (as required by the sensors)
4. **Future Hardware Upgrades**:
   - Custom PCB for a compact design.
   - 3D-printed or metal casing for protection and portability.

---

## Firmware and Libraries
- **Arduino IDE**: Used for programming the ESP32.
- **Libraries**:
  - `DHT Sensor Library` for DHT22.
  - `MQ135` for air quality sensing.
  - `PMS Library` for PMS5003.
  - `Firebase ESP Client` for Firebase integration.
  - `WiFi` and `HTTPClient` for internet connectivity.
  - `ArduinoOTA` for OTA updates.

---

## Repository Structure
Organize your repository into the following folders and files:

Air-Quality-Monitoring-System/
├── README.md
├── Hardware/
│ ├── Circuit_Diagram.png
│ ├── PCB_Design (Future)
│ └── Casing_Design (Future)
├── Firmware/
│ ├── 01_Hello_World/
│ │ └── Hello_World.ino
│ ├── 02_Sensor_Testing/
│ │ ├── DHT22_Test.ino
│ │ ├── MQ135_Test.ino
│ │ └── PMS5003_Test.ino
│ ├── 03_Sensor_Integration/
│ │ ├── DHT22_MQ135_Integration.ino
│ │ └── All_Sensors_Integration.ino
│ ├── 04_Firebase_Integration/
│ │ ├── Firebase_Random_Data.ino
│ │ └── Firebase_DHT22_Data.ino
│ ├── 05_OTA_Updates/
│ │ ├── Basic_OTA.ino
│ │ ├── OTA_Update_With_MQ135.ino
│ │ └── OTA_Update_With_PM2.5.ino
│ └── 06_Final_Code/
│ └── Air_Quality_Monitoring_Final.ino
├── Application/
│ └── (Flutter-based mobile application - Work in Progress)
└── Documentation/
├── Firebase_Setup_Guide.md
├── OTA_Setup_Guide.md
└── Sensor_Calibration_Guide.md


---

## Setup Instructions
1. **Hardware Setup**:
   - Connect the sensors to the ESP32 as per the circuit diagram.
   - Use a breadboard and jumper wires for prototyping.
2. **Firmware Setup**:
   - Install the required libraries in the Arduino IDE.
   - Update the `WiFi credentials`, `Firebase API key`, and `Firebase project URL` in the code.
3. **Uploading Code**:
   - Start with the `Hello_World.ino` sketch to test the ESP32.
   - Gradually test and integrate sensors using the respective sketches.
   - Use the final code (`Air_Quality_Monitoring_Final.ino`) for the complete system.
4. **OTA Updates**:
   - Follow the `OTA_Setup_Guide.md` to enable OTA updates.
   - Use the `OTA_Update_With_PM2.5.ino` sketch to incorporate PM2.5 sensor data into the system.

---

## Mobile Application
A Flutter-based mobile application is under development by **Amina Shahzad**. The application will provide real-time data visualization and control for the Air Quality Monitoring System.

---

## Future Work
- Design a custom PCB for the system.
- Create a proper casing for the hardware.
- Complete the Flutter-based mobile application for real-time monitoring.

---

## Contributors
- **Ayat Nauman Khan**
- **Neha Nauman Khan**
- **Ayesha Ahmed**
- **Amina Shahzad**
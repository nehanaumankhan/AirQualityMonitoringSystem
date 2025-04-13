import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';

class FirebaseService {
  final DatabaseReference _database = FirebaseDatabase.instance.ref();

  Stream<Map<String, dynamic>> getSensorData() {
    return _database.child("sensor_data").onValue.map((event) {
      final data = event.snapshot.value as Map<dynamic, dynamic>?;

      if (data == null || data.isEmpty) {
        debugPrint("⚠️ No data found in Firebase!");
        return {
          "timestamp": "",
          "temperature": 0,
          "humidity": 0,
          "mq135": 0,
          "pm1_0": 0,
          "pm2_5": 0,
          "pm10_0": 0,
        };
      }

      // Extract the latest timestamp key
      var latestKey = data.keys.last;
      var latestData = data[latestKey];

      debugPrint("✅ Latest Data Extracted: $latestData");

      return {
        "timestamp": latestKey,
        "temperature": latestData['temperature'] ?? 0,
        "humidity": latestData['humidity'] ?? 0,
        "mq135": latestData['mq135'] ?? 0,
        "pm1_0": latestData['pm1_0'] ?? 0,
        "pm2_5": latestData['pm2_5'] ?? 0,
        "pm10_0": latestData['pm10_0'] ?? 0,
      };
    });
  }
}

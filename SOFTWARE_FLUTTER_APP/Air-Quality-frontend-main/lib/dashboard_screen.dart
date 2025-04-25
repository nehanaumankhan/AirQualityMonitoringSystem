import 'dart:async';
import 'alert.dart';

import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';

import 'package:syncfusion_flutter_gauges/gauges.dart';
import 'package:fl_chart/fl_chart.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  _DashboardScreenState createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {

final GlobalKey<ThresholdAlertWidgetState> _thresholdAlertKey = GlobalKey<ThresholdAlertWidgetState>();


  Map<String, dynamic> sensorData = {
    "temperature": 0.0,
    "humidity": 0.0,
    "mq135": 0.0,
    "pm1_0": 0.0,
    "pm2_5": 0.0,
    "pm10_0": 0.0,
  };

  

  @override
  void initState() {
    super.initState();
    fetchSensorData();
    
    
  }
  
List<double> sensorDataHistory = List.filled(7, 0.0); // 7 values initialized to 0.0






void fetchSensorData() {
  DatabaseReference ref = FirebaseDatabase.instance.ref("sensor_data");

  ref.orderByKey().limitToLast(1).onValue.listen((event){
      if (event.snapshot.value != null && event.snapshot.value is Map) {
        Map<String, dynamic> data = Map<String, dynamic>.from(
          event.snapshot.value as Map,
        );

        debugPrint("📡 Raw Firebase Data: $data"); // Debugging output

        // Convert entries to list and parse timestamps
        List<Map<String, dynamic>> sortedEntries = data.entries.map((e) {
          try {
            DateTime timestamp = DateTime.parse(e.key); // Parse key as DateTime
            return {
              'timestamp': timestamp,
              'temperature': (e.value as Map)['temperature']?.toDouble() ?? 0.0,
              'humidity': (e.value as Map)['humidity']?.toDouble() ?? 0.0,
              'mq135': (e.value as Map)['mq135']?.toDouble() ?? 0.0,
              'pm1_0': (e.value as Map)['pm1_0']?.toDouble() ?? 0.0,
              'pm2_5': (e.value as Map)['pm2_5']?.toDouble() ?? 0.0,
              'pm10_0': (e.value as Map)['pm10_0']?.toDouble() ?? 0.0,
            };
          } catch (error) {
            debugPrint("⚠️ Invalid timestamp: ${e.key}");
            return null;
          }
        }).where((e) => e != null).toList().cast<Map<String, dynamic>>();

        // Sort by timestamp (latest first)
        sortedEntries.sort((a, b) => b['timestamp'].compareTo(a['timestamp']));

        if (sortedEntries.isNotEmpty) {
          var latestEntry = sortedEntries.first;

          setState(() {
            sensorData = {
              "temperature": latestEntry["temperature"],
              "humidity": latestEntry["humidity"],
              "mq135": latestEntry["mq135"],
              "pm1_0": latestEntry["pm1_0"],
              "pm2_5": latestEntry["pm2_5"],
              "pm10_0": latestEntry["pm10_0"],
            };
           
           sensorDataHistory = sortedEntries.take(7).map((entry) => (entry["temperature"] as num).toDouble()).toList();
 
              processSensorData();
              
            debugPrint("✅ Updated Sensor Data: $sensorData");
            debugPrint("✅ Temperature History: $sensorDataHistory");
            WidgetsBinding.instance.addPostFrameCallback((_) {
  _thresholdAlertKey.currentState?.checkThresholds();
});

          
          });
        } else {
          debugPrint("⚠️ No valid entries found in Firebase data!");
        }
      } else {
        debugPrint("⚠️ No data found in Firebase or invalid format!");
      }
    },
    onError: (error) {
      debugPrint("❌ Firebase Error: $error");
    },
  );
}
Map<String, double> processedSensorData = {}; // Store converted data

 void processSensorData() {
    processedSensorData = sensorData.map((key, value) {
      if (value is num) {
        return MapEntry(key, value.toDouble());
      } else {
        return MapEntry(key, 0.0); // Handle non-numeric values
      }
    });
    debugPrint("Processed Sensor Data: $processedSensorData");  
  }




  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.white,
      appBar: AppBar(
        title: const Text("Sensor Dashboard"),
        backgroundColor: Colors.blueAccent,
      ),
    body: Stack(
      children: [
        ListView(
          padding: const EdgeInsets.all(8.0),
          children: [
            _buildGauges(),
            //const SizedBox(height: 8),
            //_buildGraphs(),
            //const SizedBox(height: 16),
          ],
        ),
        ThresholdAlertWidget(key: _thresholdAlertKey,
  sensorData: processedSensorData,
), // On top
      ],
    ),
  );
}


  Widget _buildGauges() {
    return SizedBox(
      height: 680,
      child: GridView.extent(
        maxCrossAxisExtent: 240,
        crossAxisSpacing: 15,
        mainAxisSpacing: 10,
        
        
        children: [
          _buildGauge("Temp", sensorData["temperature"], 0, 60, Colors.red),
          _buildGauge("Humidity", sensorData["humidity"], 20, 100, Colors.blue),
          _buildGauge("MQ135", sensorData["mq135"], 3000, 6000, Colors.green),
          _buildGauge("PM1.0", sensorData["pm1_0"], 0, 80, Colors.orange),
          _buildGauge("PM2.5", sensorData["pm2_5"], 0, 100, Colors.purple),
          _buildGauge("PM10", sensorData["pm10_0"], 0, 100, Colors.teal),
        ],
      ),
    );
  }

 Widget _buildGauge(
    String label,
    double value,
    double min,
    double max,
    Color color,
  )  {
    var status = getStatusMessage(label, value);
  List<GaugeRange> getGaugeRanges(String label) {
    switch (label) {
      case "Temp":
        return [
          GaugeRange(startValue: min, endValue: 17, color: Colors.blue, startWidth: 8, endWidth: 8), // Hypothermia
          GaugeRange(startValue: 17, endValue: 34, color: Colors.green, startWidth: 8, endWidth: 8), // Normal
          GaugeRange(startValue: 34, endValue: 42, color: Colors.orange, startWidth: 8, endWidth: 8), // Fever
          GaugeRange(startValue: 42, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Critical
        ];
      case "Humidity":
        return [
          GaugeRange(startValue: min, endValue: 30, color: Colors.red, startWidth: 8, endWidth: 8), // Too Dry
          GaugeRange(startValue: 30, endValue: 40, color: Colors.orange, startWidth: 8, endWidth: 8), // Warning Dry
          GaugeRange(startValue: 40, endValue: 60, color: Colors.green, startWidth: 8, endWidth: 8), // Optimal
          GaugeRange(startValue: 60, endValue: 80, color: Colors.orange, startWidth: 8, endWidth: 8), // Warning Humid
          GaugeRange(startValue: 80, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Too Humid
        ];
      case "MQ135":
        return [
          GaugeRange(startValue: min, endValue: 4000, color: Colors.green, startWidth: 8, endWidth: 8), // Normal
          GaugeRange(startValue: 4001, endValue: 5000, color: Colors.orange, startWidth: 8, endWidth: 8), // Moderate Air Pollution
          GaugeRange(startValue: 5001, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Dangerous
        ];
      case "PM1.0":
        return [
          GaugeRange(startValue: min, endValue: 30, color: Colors.green, startWidth: 8, endWidth: 8), // Good Air Quality
          GaugeRange(startValue: 31, endValue: 50, color: Colors.orange, startWidth: 8, endWidth: 8), // Moderate
          GaugeRange(startValue: 51, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Unhealthy
        ];
      case "PM2.5":
        return [
          GaugeRange(startValue: min, endValue: 35, color: Colors.green, startWidth: 8, endWidth: 8), // Good
          GaugeRange(startValue: 36, endValue: 60, color: Colors.orange, startWidth: 8, endWidth: 8), // Moderate
          GaugeRange(startValue: 61, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Hazardous
        ];
      case "PM10":
        return [
          GaugeRange(startValue: min, endValue: 50, color: Colors.green, startWidth: 8, endWidth: 8), // Good
          GaugeRange(startValue: 51, endValue: 80, color: Colors.orange, startWidth: 8, endWidth: 8), // Moderate
          GaugeRange(startValue: 81, endValue: max, color: Colors.red, startWidth: 8, endWidth: 8), // Hazardous
        ];
      default:
        return [];
    }
  }
    
    return Card(
      elevation: 10,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 8),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              label,
              style: const TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.bold,
                color: Colors.black87,
              ),
            ),
            SizedBox(
              height: 95,
              child: SfRadialGauge(
                axes: <RadialAxis>[
                  RadialAxis(
                    minimum: min,
                    maximum: max,
                    showLabels: true,
                    showTicks: true,
                    axisLabelStyle: const GaugeTextStyle(fontSize: 8),
                    ranges: getGaugeRanges(label),
                    
                    pointers: <GaugePointer>[
                      NeedlePointer(
                        value: value,
                        enableAnimation: true,
                        needleColor: color,
                        knobStyle: const KnobStyle(
                          color: Colors.white,
                          borderColor: Colors.black,
                          borderWidth: 0.2,
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ),
            Text(
              value.toStringAsFixed(1),
              style: TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.bold,
                color: color,
              ),
            ),
            SizedBox(height: 4), // Spacing between value and status text
          Text(
            status["message"], // Displaying status message
            style: TextStyle(
              fontSize: 12,
              fontWeight: FontWeight.w500,
              color: status["color"], // Dynamically change text color
            ),
          ),
          ],
        ),
      ),
    );
  }

  LineChartBarData _buildLineChart(double value, Color color) {
    return LineChartBarData(
      spots: List.generate(
        7,
        (index) => FlSpot(
          index.toDouble(),
          value * (0.8 + (index * 0.1)),
        ),
      ),
      isCurved: true,
      color: color,
      barWidth: 3,
      belowBarData: BarAreaData(
        show: true,
        gradient: LinearGradient(
          // ignore: deprecated_member_use
          colors: [color.withOpacity(0.4), Colors.transparent],
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
        ),
      ),
      dotData: FlDotData(
        show: true,
        getDotPainter: (spot, percent, barData, index) {
          return FlDotCirclePainter(
            radius: 4,
            color: color,
            strokeWidth: 1,
            strokeColor: Colors.white,
          );
        },
      ),
      showingIndicators: [2],
    );
  }


Widget _buildGraphs() {
  return Column(
    children: [
      
      const Text(
        "Real-Time Sensor Data",
        style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
      ),
      SizedBox(height: 8),
      SizedBox(
  height: 800,
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 8),
          child: GridView.count(
            crossAxisCount: 3, // 2 columns in the grid
            childAspectRatio: 1.2, // Adjust ratio to fit better
            shrinkWrap: true,
            physics: NeverScrollableScrollPhysics(),
            children: [
              _buildSingleGraph("Temperature (°C)", sensorData["temperature"], Colors.red, 0, 60),
              _buildSingleGraph("Humidity (%)", sensorData["humidity"], Colors.blue, 0, 100),
              _buildSingleGraph("MQ135 (ppm)", sensorData["mq135"], Colors.green, 3000, 6000),
              _buildSingleGraph("PM1.0 (µg/m³)", sensorData["pm1_0"], Colors.orange, 0, 80),
              _buildSingleGraph("PM2.5 (µg/m³)", sensorData["pm2_5"], Colors.purple, 0, 80),
              _buildSingleGraph("PM10.0 (µg/m³)", sensorData["pm10_0"], Colors.teal, 0, 80),
            ],
          ),
        ),
      ),
    ],
  );
}


Widget _buildSingleGraph(String title, double value, Color color, double minY, double maxY) {
  return Card(
    elevation: 20,
    margin: EdgeInsets.all(8),
    child: Padding(
      padding: const EdgeInsets.all(8.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            title,
            style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
          ),
          SizedBox(height: 8),
          Expanded(
            child: LineChart(
              LineChartData(
                minY: minY,
                maxY: maxY,
                gridData: FlGridData(show: true),
                titlesData: FlTitlesData(
                  leftTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      interval: (maxY - minY) / 6,
                      reservedSize: 32,
                      getTitlesWidget: (value, meta) {
                        return Text(
                          value.toStringAsFixed(0), // ✅ Round axis labels to 2 decimals
                          style: TextStyle(fontSize: 10),
                        );
                      },
                    ),
                  ),
                  bottomTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      interval:1,
                      reservedSize: 22,
                      getTitlesWidget: (value, meta) {
                        return Text(
                           (value.toInt()+1).toString(),
                          style: TextStyle(fontSize: 10),
                        );
                      },
                    ),
                  ),
                  rightTitles: AxisTitles(
      sideTitles: SideTitles(showTitles: false), // ❌ Hide right labels
    ),
    topTitles: AxisTitles(
      sideTitles: SideTitles(showTitles: false), // ❌ Hide top labels
    ),
                ),
                borderData: FlBorderData(
                  show: true,
                  border: Border.all(color: Colors.black12, width: 1),
                ),
                lineBarsData: [_buildLineChart(value, color)],
                // ✅ Tooltip to round indicator values
                lineTouchData: LineTouchData(
                  touchTooltipData: LineTouchTooltipData(
                    tooltipBgColor: Colors.black87,
                    getTooltipItems: (touchedSpots) {
                      return touchedSpots.map((spot) {
                        return LineTooltipItem(
                          spot.y.toStringAsFixed(2), // ✅ Round value to 2 decimal places
                          const TextStyle(color: Colors.white, fontSize: 12),
                        );
                      }).toList();
                    },
                  ),
                ),
              ),
            ),
          ),
        ],
      ),
    ),
  );
}




 

 
}

 Map<String, dynamic> getStatusMessage(String label, double value) {
    if (label == "Temp") {
      if (value < 14) return {"message": "Hypothermia Risk", "color": Colors.blue};
      if (value >= 14 && value <= 26.5) return {"message": "Mild cold", "color": Colors.blue};
      if (value >= 27 && value <= 37.5) return {"message": "Normal", "color": Colors.green};
      if (value > 37.5 && value <= 38.5) return {"message": "High Temperature", "color": Colors.orange};
      return {"message": "High Temperature Alert", "color": Colors.red};
    }
    if (label == "Humidity") {
      if (value < 30) return {"message": "Too Dry", "color": Colors.orange};
      if (value >= 30 && value <= 60) return {"message": "Moderate", "color": Colors.green};
      if (value > 60 && value <= 80) return {"message": "Humid Warning", "color": Colors.orange};
      return {"message": "Excessive Humidity", "color": Colors.red};
    }
    if (label == "MQ135") {
      if (value < 4000) return {"message": "Good ", "color": Colors.green};
      if (value >= 4000 && value <= 5000) return {"message": "Moderate ", "color": Colors.orange};
      return {"message": "Unhealthy Air", "color": Colors.red};
    }
    if (label == "PM1.0") {
      if (value <= 30) return {"message": "Good", "color": Colors.green};
      if (value > 30 && value <= 50) return {"message": "Moderate", "color": Colors.orange};
      return {"message": "Hazardous", "color": Colors.red};
    }
    if (label == "PM2.5") {
      if (value <= 35) return {"message": "Good", "color": Colors.green};
      if (value > 35 && value <= 60) return {"message": "Moderate", "color": Colors.orange};
      return {"message": "Unhealthy", "color": Colors.red};
    }
    if (label == "PM10") {
      if (value <= 50) return {"message": "Good", "color": Colors.green};
      if (value > 50 && value <= 80) return {"message": "Moderate", "color": Colors.orange};
      return {"message": "Hazardous", "color": Colors.red};
    }
    return {"message": "Unknown", "color": Colors.grey};
  }
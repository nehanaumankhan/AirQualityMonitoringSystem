import 'alert.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';
import 'package:syncfusion_flutter_gauges/gauges.dart';
import 'package:fl_chart/fl_chart.dart';
import 'graph.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  _DashboardScreenState createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  final GlobalKey<ThresholdAlertWidgetState> _thresholdAlertKey =
      GlobalKey<ThresholdAlertWidgetState>();

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

  List<double> sensorDataHistory = List.filled(7, 0.0);

  void fetchSensorData() {
    DatabaseReference ref = FirebaseDatabase.instance.ref("sensor_data");

    ref.orderByKey().limitToLast(1).onValue.listen((event) {
      if (event.snapshot.value != null && event.snapshot.value is Map) {
        Map<String, dynamic> data = Map<String, dynamic>.from(
          event.snapshot.value as Map,
        );

        debugPrint("📡 Raw Firebase Data: $data");

        List<Map<String, dynamic>> sortedEntries = data.entries.map((e) {
          try {
            DateTime timestamp = DateTime.parse(e.key);
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

            sensorDataHistory = sortedEntries
                .take(7)
                .map((entry) => (entry["temperature"] as num).toDouble())
                .toList();
            processSensorData();
          });
        }
      }
    });
  }

  Map<String, double> processedSensorData = {};

  void processSensorData() {
    processedSensorData = sensorData.map((key, value) {
      return MapEntry(key, value.toDouble());
    });
    debugPrint("Processed Sensor Data: $processedSensorData");
  }

@override
Widget build(BuildContext context) {
  return Scaffold(
    backgroundColor: Colors.white,
 appBar: AppBar(
  title: Text(
    "Sensor Dashboard",
    style: TextStyle(
      color: Colors.white, // Change text color here
      fontWeight: FontWeight.bold,
      fontSize: 25,
    ),
  ),backgroundColor: const Color.fromARGB(255, 0, 6, 16),
  toolbarHeight: 80,
   ),
    body: Stack(
      children: [
        // Background image
        Container(
          decoration: BoxDecoration(
            image: DecorationImage(
              image: AssetImage("assets/bk.jpg"), // Set your image here
              fit: BoxFit.cover, // Ensure the image covers the whole screen
            ),
          ),
        ),
        // List of gauges
        ListView(
          padding: const EdgeInsets.all(8.0),
          children: [
            SizedBox(height: 60), // Adjust spacing
            _buildGauges(),
          ],
        ),
        // Threshold alert widget
        ThresholdAlertWidget(
          key: _thresholdAlertKey,
          sensorData: processedSensorData,
        ),
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
        mainAxisSpacing: 15,
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
  ) {
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

    return GestureDetector(
  onTap: () {
    navigateToGraphScreen(label);
  },child: SizedBox(height: 220,
  child: Card(
    
    elevation: 10,
    color: const Color.fromARGB(255, 12, 1, 72),
    shape: RoundedRectangleBorder(
      borderRadius: BorderRadius.circular(12),
    
    ),
    child: Padding(
      padding: const EdgeInsets.symmetric(vertical: 0, horizontal: 8),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          // Label + Info icon in a Row
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                label,
                style: const TextStyle(
                  fontSize: 14,
                  fontWeight: FontWeight.bold,
                  color: Color.fromARGB(221, 234, 232, 232),
                ),
              ),
              IconButton(
                icon: const Icon(Icons.info_outline, size: 16, color: Colors.white),
                padding: EdgeInsets.zero,
                constraints: const BoxConstraints(),
                onPressed: () {
                  showDialog(
                    context: context,
                    builder: (_) => AlertDialog(
                      title: Text('$label Info',style: TextStyle(color: Colors.white), ),
                    
                      content: Text(getSensorInfo(label),style: TextStyle(color: Colors.white),),backgroundColor: Color.fromARGB(255, 0, 1, 2),
                      actions: [
                        TextButton(
                          onPressed: () => Navigator.of(context).pop(),
                          child: const Text('Close'),
                        ),
                      ],
                    ),
                  );
                },
              ),
            ],
          ),
          SizedBox(
            height: 96,
            child: SfRadialGauge(
              axes: <RadialAxis>[
                RadialAxis(
                  labelOffset:10,
                  interval: (max - min) / 2, // Shows 3 labels: min, mid, max
                  minimum: min,
                  maximum: max,
                  showLabels: false,
                  showTicks: false,
                  axisLabelStyle: const GaugeTextStyle(fontSize: 10,color: Colors.white),  axisLineStyle: const AxisLineStyle(
    thickness: 0.08,
    thicknessUnit: GaugeSizeUnit.factor,
  ),
  annotations: <GaugeAnnotation>[
    GaugeAnnotation(
      widget: Text('${min.toStringAsFixed(0)}', style: const TextStyle(color: Colors.white, fontSize: 8)),
      angle: 130,
      positionFactor: 0.55,
    ),
    GaugeAnnotation(
      widget: Text('${((min + max) / 2).toStringAsFixed(0)}', style: const TextStyle(color: Colors.white, fontSize: 8)),
      angle: 280,
      positionFactor: 0.55,
    ),
    GaugeAnnotation(
      widget: Text('${max.toStringAsFixed(0)}', style: const TextStyle(color: Colors.white, fontSize: 8)),
      angle: 50,
      positionFactor: 0.55,
    ),
  ],

                  
                  ranges: getGaugeRanges(label),
                  pointers: <GaugePointer>[
                    NeedlePointer(
                      value: value,
                      enableAnimation: true,
                      needleColor: color,
                      knobStyle: const KnobStyle(
                        color: Color.fromARGB(255, 240, 239, 242),
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
              fontSize: 12,
              fontWeight: FontWeight.bold,
              color: color,
            ),
          ),
         // const SizedBox(height: 2),
          Text(
            status["message"],
            style: TextStyle(
              fontSize: 12,
              fontWeight: FontWeight.w500,
              color: status["color"],
            ),
          ),
        ],
      ),
    ),
  ),),
);

  }



  String getSensorInfo(String label) {
  switch (label) {
    case "Temp":
      return "Measures ambient temperature. Useful to detect changes in room or environmental temperature.";
    case "Humidity":
      return "Measures the amount of moisture in the air. Helps monitor indoor air quality and comfort.";
    case "MQ135":
      return "Air quality sensor that detects gases like NH3, NOx, alcohol, benzene, smoke, and CO2.";
    case "PM1.0":
      return "Detects fine particulate matter less than 1.0 micrometers in diameter.";
    case "PM2.5":
      return "Measures airborne particles with a diameter less than 2.5µm, which can be harmful to health.";
    case "PM10":
      return "Monitors larger particulate matter up to 10µm in size, often from dust, pollen, or smoke.";
    default:
      return "Sensor information not available.";
  }
}


  void navigateToGraphScreen(String label) {
    debugPrint(label);
    debugPrint("yesss");
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => GraphScreen(label: label, sensorData: sensorData),
      ),
    );
  }

  // Function to return gauge ranges based on the label
  

  // Dummy function to return status
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
}



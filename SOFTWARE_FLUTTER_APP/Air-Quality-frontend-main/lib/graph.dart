
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:fl_chart/fl_chart.dart'; 
class GraphScreen extends StatelessWidget {
  final String label;
  final Map<String, dynamic> sensorData;

  GraphScreen({required this.label, required this.sensorData});

 @override
Widget build(BuildContext context) {
  return Scaffold(
    appBar: AppBar(
      title: Text(
        "${getGraphTitle(label)} Graph",
        style: const TextStyle(color: Colors.white),
      ),
      backgroundColor: const Color.fromARGB(255, 48, 14, 221),
    ),
    body: Stack(
      children: [
        // 🔹 Background Image
        Positioned.fill(
          child: Image.asset(
            "assets/bk.jpg", // Ensure this image exists in your assets folder
            fit: BoxFit.cover,
          ),
        ),

        // 🔹 Foreground content
        Center(
          child: SingleChildScrollView(
            padding: const EdgeInsets.all(8.0),
            child: graph(label),
          ),
        ),
      ],
    ),
  );
}

 graph(label)
 {if (label == "Temp") {
    return _buildSingleGraph("Temperature (°C)", sensorData["temperature"], Colors.red, 0, 60);
  } else if (label == "Humidity") {
    return _buildSingleGraph("Humidity (%)", sensorData["humidity"], Colors.blue, 0, 100);
  } else if (label == "MQ135") {
    return _buildSingleGraph("MQ135 (ppm)", sensorData["mq135"], Colors.green, 3000, 6000);
  } else if (label == "PM1.0") {
    return _buildSingleGraph("PM1.0 (µg/m³)", sensorData["pm1_0"], Colors.orange, 0, 80);
  } else if (label == "PM2.5") {
    return _buildSingleGraph("PM2.5 (µg/m³)", sensorData["pm2_5"], Colors.purple, 0, 80);
  } else if (label == "PM10") {
    return _buildSingleGraph("PM10.0 (µg/m³)", sensorData["pm10_0"], Colors.teal, 0, 80);
  }

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


  Widget _buildSingleGraph(String title, double value, Color color, double minY, double maxY) {
  return Card(
    elevation: 20,
    color: const Color.fromARGB(255, 19, 19, 19),
    margin: EdgeInsets.all(8),
    child: Padding(
      padding: const EdgeInsets.all(8.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            title,
            
            style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold,color: Colors.white),
          ),
          SizedBox(height: 8),
          SizedBox(
            height: 280, // 🔧 Adjust this height as needed
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
                          value.toStringAsFixed(0),
                          style: TextStyle(fontSize: 10,color: Colors.white),
                          
                        );
                      },
                    ),
                  ),
                  bottomTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      interval: 1,
                      reservedSize: 22,
                      getTitlesWidget: (value, meta) {
                        return Text(
                          (value.toInt() + 1).toString(),
                          style: TextStyle(fontSize: 10,color: Colors.white),
                          
                        );
                      },
                    ),
                  ),
                  rightTitles: AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                  topTitles: AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                ),
                borderData: FlBorderData(
                  show: true,
                  border: Border.all(color: Colors.white, width: 1),
                ),
                lineBarsData: [_buildLineChart(value, color)],
                lineTouchData: LineTouchData(
                  touchTooltipData: LineTouchTooltipData(
                    tooltipBgColor: Colors.black87,
                    getTooltipItems: (touchedSpots) {
                      return touchedSpots.map((spot) {
                        return LineTooltipItem(
                          spot.y.toStringAsFixed(2),
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
String getGraphTitle(String label) {
  switch (label) {
    case "Temp":
      return "Temperature (°C)";
    case "Humidity":
      return "Humidity (%)";
    case "MQ135":
      return "MQ135 (ppm)";
    case "PM1.0":
      return "PM1.0 (µg/m³)";
    case "PM2.5":
      return "PM2.5 (µg/m³)";
    case "PM10":
      return "PM10.0 (µg/m³)";
    default:
      return label;
  }
}

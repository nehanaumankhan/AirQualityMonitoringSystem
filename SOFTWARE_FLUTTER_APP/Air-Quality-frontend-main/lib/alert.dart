import 'package:flutter/material.dart';

class ThresholdAlertWidget extends StatefulWidget {
  final Map<String, double> sensorData;

  const ThresholdAlertWidget({super.key, required this.sensorData});

  @override
  // ignore: library_private_types_in_public_api
  ThresholdAlertWidgetState createState() => ThresholdAlertWidgetState();
}

class ThresholdAlertWidgetState extends State<ThresholdAlertWidget> {
  List<String> alertMessages = [];

  @override
  void initState() {
    super.initState();
       WidgetsBinding.instance.addPostFrameCallback((_) {
    checkThresholds();
  });
  }

  void checkThresholds() {
     debugPrint("🚨 Running threshold check with data: ${widget.sensorData}");
    if(widget.sensorData.isEmpty) return ;
    alertMessages.clear();

    if (widget.sensorData["temperature"]! < 37.5) {
      alertMessages.add("Temperature is High (Fever Warning)");
    }
    if (widget.sensorData["humidity"]! < 30) {
      alertMessages.add("Humidity is Low (Too Dry)");
    } else if (widget.sensorData["humidity"]! > 60) {
      alertMessages.add("Humidity is High (Humid Warning)");
    }
    if (widget.sensorData["mq135"]! > 5000) {
      alertMessages.add("Air Quality is Unhealthy");
    }
    if (widget.sensorData["pm1_0"]! > 50) {
      alertMessages.add("PM1.0 is High (Hazardous)");
    }
    if (widget.sensorData["pm2_5"]! > 70) {
      alertMessages.add("PM2.5 is High (Unhealthy)");
    }
    if (widget.sensorData["pm10_0"]! > 80) {
      alertMessages.add("PM10 is High (Hazardous)");
    }

    if (alertMessages.isNotEmpty) {
      showAlertDialog();
    }
  }

  void showAlertDialog() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text("Threshold Alerts"),
          content: SingleChildScrollView(
            child: ListBody(
              children: alertMessages.map((message) => Text(message)).toList(),
            ),
          ),
          actions: <Widget>[
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: const Text("OK"),
            ),
          ],
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return const SizedBox.shrink(); // Widget does not need to display anything
  }
}

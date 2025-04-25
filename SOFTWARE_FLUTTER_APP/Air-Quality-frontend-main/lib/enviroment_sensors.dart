import 'package:firebase_database/firebase_database.dart';

class EnvironmentSensors {
  final DatabaseReference _databaseRef =
      FirebaseDatabase.instance.ref().child('sensor_data');

  Stream<double> get temperature => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['temperature'].toDouble());

  Stream<double> get humidity => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['humidity'].toDouble());

  Stream<int> get mq135 => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['mq135'] as int);

  Stream<int> get pm1_0 => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['pm1_0'] as int);

  Stream<int> get pm2_5 => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['pm2_5'] as int);

  Stream<int> get pm10_0 => _databaseRef
      .orderByChild('timestamp')
      .limitToLast(1)
      .onValue
      .map((event) => (event.snapshot.value as Map).values.first['pm10_0'] as int);
}

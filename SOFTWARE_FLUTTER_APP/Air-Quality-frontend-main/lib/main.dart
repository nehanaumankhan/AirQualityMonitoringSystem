import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';

import 'firebase_options.dart';
import 'try.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  try{
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  ); debugPrint("🔥 Firebase initialized successfully!");
  } catch (e) {
    debugPrint("❌ Firebase initialization error: $e");
  }
  runApp(MyApp());
}
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Sensor Dashboard',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: DashboardScreen(),
      debugShowCheckedModeBanner: false,
      
    );
  }
}

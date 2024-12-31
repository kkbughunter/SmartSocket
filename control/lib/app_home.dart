import 'package:flutter/material.dart';
import 'package:control/service/home_service.dart';

class AppHome extends StatefulWidget {
  final String userId;

  const AppHome({super.key, required this.userId});

  @override
  State<AppHome> createState() => _AppHomeState();
}

class _AppHomeState extends State<AppHome> {
  String data = 'Loading...';

  @override
  void initState() {
    super.initState();
    fetchUserName();
  }

  void fetchUserName() async {
    String name = await HomeService.getName(widget.userId);
    setState(() {
      data = name;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(data),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text("hello sir"),
          ],
        ),
      ),
    );
  }
}

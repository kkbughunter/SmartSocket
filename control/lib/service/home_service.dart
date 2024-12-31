import 'package:firebase_database/firebase_database.dart';

class HomeService {
  static final FirebaseDatabase _database = FirebaseDatabase.instance;

  static Future<String> getName(String userId) async {
    try {
      final ref = _database.ref('users/$userId');
      final snapshot = await ref.child('name').get();
      return snapshot.value != null
          ? snapshot.value.toString()
          : 'No data found';
    } catch (e) {
      return 'Error fetching name';
    }
  }
}

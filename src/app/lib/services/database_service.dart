import 'package:firebase_database/firebase_database.dart';
import '../config/app_constants.dart';
import '../models/parcel_model.dart';

class DatabaseService {
  final DatabaseReference _db = FirebaseDatabase.instance.ref();

  Future<void> addParcel(ParcelModel parcel) async {
    await _db
        .child(AppConstants.parcelsPath)
        .child(parcel.parcelId)
        .set(parcel.toMap());
  }

  Future<List<ParcelModel>> getParcels() async {
    final snapshot = await _db.child(AppConstants.parcelsPath).get();
    if (!snapshot.exists) return [];
    final map = snapshot.value as Map<dynamic, dynamic>;
    return map.entries
        .map((e) => ParcelModel.fromMap(e.key.toString(), e.value as Map<dynamic, dynamic>))
        .toList()
      ..sort((a, b) => b.timestamp.compareTo(a.timestamp));
  }

  Stream<DatabaseEvent> parcelsStream() {
    return _db.child(AppConstants.parcelsPath).onValue;
  }

  Future<ParcelModel?> getParcel(String parcelId) async {
    final snapshot =
        await _db.child(AppConstants.parcelsPath).child(parcelId).get();
    if (!snapshot.exists) return null;
    return ParcelModel.fromMap(
        parcelId, snapshot.value as Map<dynamic, dynamic>);
  }
}

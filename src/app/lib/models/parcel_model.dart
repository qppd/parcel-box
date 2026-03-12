class ParcelModel {
  final String parcelId;
  final String receiverName;
  final String contactNumber;
  final String lockerNumber;
  final int timestamp;
  final String status;

  ParcelModel({
    required this.parcelId,
    required this.receiverName,
    required this.contactNumber,
    required this.lockerNumber,
    required this.timestamp,
    required this.status,
  });

  Map<String, dynamic> toMap() {
    return {
      'parcel_id': parcelId,
      'qr_code': parcelId,
      'receiver_name': receiverName,
      'contact_number': contactNumber,
      'locker_number': lockerNumber,
      'timestamp': timestamp,
      'status': status,
    };
  }

  factory ParcelModel.fromMap(String key, Map<dynamic, dynamic> map) {
    return ParcelModel(
      parcelId: (map['parcel_id'] ?? key).toString(),
      receiverName: (map['receiver_name'] ?? '').toString(),
      contactNumber: (map['contact_number'] ?? '').toString(),
      lockerNumber: (map['locker_number'] ?? '').toString(),
      timestamp: (map['timestamp'] ?? 0) as int,
      status: (map['status'] ?? 'pending').toString(),
    );
  }
}

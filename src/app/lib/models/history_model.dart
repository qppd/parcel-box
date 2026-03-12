class HistoryModel {
  final String id;
  final String parcelId;
  final String event;
  final int timestamp;
  final String deviceId;

  HistoryModel({
    required this.id,
    required this.parcelId,
    required this.event,
    required this.timestamp,
    required this.deviceId,
  });

  factory HistoryModel.fromMap(String key, Map<dynamic, dynamic> map) {
    return HistoryModel(
      id: key,
      parcelId: (map['parcel_id'] ?? '').toString(),
      event: (map['event'] ?? '').toString(),
      timestamp: (map['timestamp'] ?? 0) as int,
      deviceId: (map['device_id'] ?? '').toString(),
    );
  }
}

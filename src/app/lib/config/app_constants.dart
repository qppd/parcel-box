class AppConstants {
  static const String appName = 'ParcelBox';
  static const String appVersion = '1.0.0';

  // Firebase database paths
  static const String parcelsPath = 'parcels';
  static const String historyPath = 'history';
  static const String deviceStatusPath = 'device_status';
  static const String locksStatusPath = 'locks_status';

  // Parcel statuses
  static const String statusPending = 'pending';
  static const String statusDelivered = 'delivered';
  static const String statusFailed = 'failed';

  // Locker numbers
  static const List<String> lockerNumbers = ['1', '2'];
}

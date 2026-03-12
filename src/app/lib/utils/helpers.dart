import 'package:intl/intl.dart';
import 'package:flutter/material.dart';
import '../config/app_constants.dart';

String formatTimestamp(int timestamp) {
  if (timestamp == 0) return 'N/A';
  final date = DateTime.fromMillisecondsSinceEpoch(timestamp);
  return DateFormat('MMM dd, yyyy – hh:mm a').format(date);
}

Color statusColor(String status) {
  switch (status) {
    case AppConstants.statusPending:
      return Colors.orange;
    case AppConstants.statusDelivered:
      return Colors.green;
    case AppConstants.statusFailed:
      return Colors.red;
    default:
      return Colors.grey;
  }
}

IconData statusIcon(String status) {
  switch (status) {
    case AppConstants.statusPending:
      return Icons.hourglass_bottom;
    case AppConstants.statusDelivered:
      return Icons.check_circle;
    case AppConstants.statusFailed:
      return Icons.cancel;
    default:
      return Icons.help_outline;
  }
}

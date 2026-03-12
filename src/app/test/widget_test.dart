import 'package:flutter_test/flutter_test.dart';

import 'package:parcel_box_app/main.dart';

void main() {
  testWidgets('App renders', (WidgetTester tester) async {
    await tester.pumpWidget(const ParcelBoxApp());
    expect(find.text('ParcelBox'), findsAny);
  });
}

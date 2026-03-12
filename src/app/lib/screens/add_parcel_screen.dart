import 'package:flutter/material.dart';
import '../config/app_constants.dart';
import '../config/app_theme.dart';
import '../models/parcel_model.dart';
import '../services/database_service.dart';

class AddParcelScreen extends StatefulWidget {
  const AddParcelScreen({super.key});

  @override
  State<AddParcelScreen> createState() => _AddParcelScreenState();
}

class _AddParcelScreenState extends State<AddParcelScreen> {
  final _formKey = GlobalKey<FormState>();
  final _parcelIdController = TextEditingController();
  final _receiverController = TextEditingController();
  final _contactController = TextEditingController();
  final _dbService = DatabaseService();
  String _selectedLocker = AppConstants.lockerNumbers.first;
  bool _loading = false;

  @override
  void dispose() {
    _parcelIdController.dispose();
    _receiverController.dispose();
    _contactController.dispose();
    super.dispose();
  }

  Future<void> _saveParcel() async {
    if (!_formKey.currentState!.validate()) return;
    setState(() => _loading = true);
    try {
      final parcel = ParcelModel(
        parcelId: _parcelIdController.text.trim(),
        receiverName: _receiverController.text.trim(),
        contactNumber: _contactController.text.trim(),
        lockerNumber: _selectedLocker,
        timestamp: DateTime.now().millisecondsSinceEpoch,
        status: AppConstants.statusPending,
      );
      await _dbService.addParcel(parcel);
      if (!mounted) return;
      Navigator.pushReplacementNamed(
        context,
        '/qr-result',
        arguments: parcel.parcelId,
      );
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error: $e'), backgroundColor: AppTheme.errorColor),
      );
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Add Parcel')),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(24),
        child: Form(
          key: _formKey,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'Parcel Details',
                style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 4),
              Text(
                'Fill in the delivery information',
                style: TextStyle(color: Colors.grey.shade600),
              ),
              const SizedBox(height: 24),
              TextFormField(
                controller: _parcelIdController,
                decoration: const InputDecoration(
                  labelText: 'Reference Number / Parcel ID',
                  prefixIcon: Icon(Icons.qr_code),
                ),
                validator: (v) {
                  if (v == null || v.trim().isEmpty) return 'Enter a parcel ID';
                  if (v.trim().length < 5) return 'Parcel ID must be at least 5 characters';
                  return null;
                },
              ),
              const SizedBox(height: 16),
              TextFormField(
                controller: _receiverController,
                decoration: const InputDecoration(
                  labelText: 'Receiver Name',
                  prefixIcon: Icon(Icons.person),
                ),
                validator: (v) {
                  if (v == null || v.trim().isEmpty) return 'Enter receiver name';
                  return null;
                },
              ),
              const SizedBox(height: 16),
              TextFormField(
                controller: _contactController,
                keyboardType: TextInputType.phone,
                decoration: const InputDecoration(
                  labelText: 'Contact Number',
                  prefixIcon: Icon(Icons.phone),
                ),
                validator: (v) {
                  if (v == null || v.trim().isEmpty) return 'Enter contact number';
                  return null;
                },
              ),
              const SizedBox(height: 16),
              DropdownButtonFormField<String>(
                initialValue: _selectedLocker,
                decoration: const InputDecoration(
                  labelText: 'Locker Number',
                  prefixIcon: Icon(Icons.lock),
                ),
                items: AppConstants.lockerNumbers
                    .map((n) => DropdownMenuItem(value: n, child: Text('Locker $n')))
                    .toList(),
                onChanged: (v) {
                  if (v != null) setState(() => _selectedLocker = v);
                },
              ),
              const SizedBox(height: 32),
              SizedBox(
                width: double.infinity,
                child: ElevatedButton.icon(
                  onPressed: _loading ? null : _saveParcel,
                  icon: _loading
                      ? const SizedBox(
                          height: 20,
                          width: 20,
                          child: CircularProgressIndicator(color: Colors.white, strokeWidth: 2),
                        )
                      : const Icon(Icons.save),
                  label: Text(_loading ? 'Saving...' : 'Save & Generate QR'),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

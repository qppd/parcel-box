import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import '../config/app_constants.dart';
import '../models/parcel_model.dart';
import '../widgets/parcel_card.dart';

class HistoryScreen extends StatefulWidget {
  const HistoryScreen({super.key});

  @override
  State<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends State<HistoryScreen> {
  final _searchController = TextEditingController();
  String _searchQuery = '';

  @override
  void dispose() {
    _searchController.dispose();
    super.dispose();
  }

  List<ParcelModel> _filter(List<ParcelModel> parcels) {
    if (_searchQuery.isEmpty) return parcels;
    final q = _searchQuery.toLowerCase();
    return parcels
        .where((p) =>
            p.parcelId.toLowerCase().contains(q) ||
            p.receiverName.toLowerCase().contains(q))
        .toList();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Parcel History')),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(16),
            child: TextField(
              controller: _searchController,
              decoration: InputDecoration(
                hintText: 'Search by parcel ID or receiver name',
                prefixIcon: const Icon(Icons.search),
                suffixIcon: _searchQuery.isNotEmpty
                    ? IconButton(
                        icon: const Icon(Icons.clear),
                        onPressed: () {
                          _searchController.clear();
                          setState(() => _searchQuery = '');
                        },
                      )
                    : null,
              ),
              onChanged: (v) => setState(() => _searchQuery = v),
            ),
          ),
          Expanded(
            child: StreamBuilder<DatabaseEvent>(
              stream: FirebaseDatabase.instance
                  .ref()
                  .child(AppConstants.parcelsPath)
                  .onValue,
              builder: (context, snapshot) {
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return const Center(child: CircularProgressIndicator());
                }
                if (snapshot.hasError) {
                  return Center(child: Text('Error: ${snapshot.error}'));
                }
                final data = snapshot.data?.snapshot.value;
                if (data == null) {
                  return const _EmptyState();
                }
                final map = data as Map<dynamic, dynamic>;
                final parcels = map.entries
                    .map((e) => ParcelModel.fromMap(
                        e.key.toString(), e.value as Map<dynamic, dynamic>))
                    .toList()
                  ..sort((a, b) => b.timestamp.compareTo(a.timestamp));

                final filtered = _filter(parcels);
                if (filtered.isEmpty) {
                  return const _EmptyState(isSearchResult: true);
                }

                return ListView.builder(
                  padding: const EdgeInsets.only(bottom: 16),
                  itemCount: filtered.length,
                  itemBuilder: (_, i) => ParcelCard(
                    parcel: filtered[i],
                    onTap: () => Navigator.pushNamed(
                      context,
                      '/qr-result',
                      arguments: filtered[i].parcelId,
                    ),
                  ),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}

class _EmptyState extends StatelessWidget {
  final bool isSearchResult;
  const _EmptyState({this.isSearchResult = false});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(
            isSearchResult ? Icons.search_off : Icons.inbox_outlined,
            size: 64,
            color: Colors.grey.shade400,
          ),
          const SizedBox(height: 12),
          Text(
            isSearchResult ? 'No matching parcels' : 'No parcels yet',
            style: TextStyle(fontSize: 16, color: Colors.grey.shade600),
          ),
        ],
      ),
    );
  }
}

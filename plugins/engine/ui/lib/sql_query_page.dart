import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FTextField's `controller` moved into a `control:`
// FTextFieldControl object.

/// SQL query viewer — a free-text SQL box, a "Run" action, and the results
/// rendered via [CDataTable]. Backed by `POST /api/v1/query`
/// (`plugins/engine/src/core/handler/query.cppm`), which only accepts
/// `SELECT` statements — anything else comes back as a 400 from the engine,
/// surfaced here as the query's error text rather than special-cased
/// client-side, so the one enforcement point stays server-side.
class SqlQueryPage extends StatefulWidget {
  const SqlQueryPage({super.key});

  @override
  State<SqlQueryPage> createState() => _SqlQueryPageState();
}

class _SqlQueryPageState extends State<SqlQueryPage> {
  final _api = EngineApiClient();
  final _sqlController = TextEditingController(
    text: 'SELECT * FROM workflow_definitions;',
  );
  List<Map<String, dynamic>> _rows = [];
  bool _running = false;
  bool _ranOnce = false;
  String? _error;

  @override
  void dispose() {
    _sqlController.dispose();
    super.dispose();
  }

  Future<void> _run() async {
    final sql = _sqlController.text.trim();
    if (sql.isEmpty) return;
    setState(() {
      _running = true;
      _error = null;
    });
    try {
      final rows = await _api.runQuery(sql);
      setState(() {
        _rows = rows;
        _ranOnce = true;
      });
    } catch (e) {
      setState(() => _error = '$e');
    } finally {
      if (mounted) setState(() => _running = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(breadcrumbs: const ['SQL Query']),
      child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            FTextField.multiline(
              control: FTextFieldControl.managed(controller: _sqlController),
              hint: 'SELECT * FROM ...',
              minLines: 3,
              maxLines: 6,
            ),
            const SizedBox(height: 12),
            FButton(
              onPress: _running ? null : _run,
              child: _running
                  ? const SizedBox(
                      width: 16,
                      height: 16,
                      child: FCircularProgress(),
                    )
                  : const Text('Run'),
            ),
            const SizedBox(height: 16),
            Expanded(child: _buildResults()),
          ],
        ),
    );
  }

  Widget _buildResults() {
    if (_error != null) {
      return Center(child: Text('Query failed: $_error'));
    }
    if (!_ranOnce) {
      return const Center(child: Text('Run a query to see results.'));
    }
    // Column set is derived from whatever the query actually returned — arbitrary SQL has no
    // fixed schema this page can know ahead of time, unlike the Tasks/Workflows CDataTable uses.
    final columnNames = _rows.isEmpty ? const <String>[] : _rows.first.keys.toList();
    return CDataTable<Map<String, dynamic>>(
      isLoading: _running,
      data: _rows,
      getRowId: (row) => _rows.indexOf(row).toString(),
      columns: [
        for (final name in columnNames)
          CDataTableColumn(
            id: name,
            header: name,
            cell: (row) => Text('${row[name] ?? ''}'),
          ),
      ],
    );
  }
}

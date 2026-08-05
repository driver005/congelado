import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FButton takes `variant:` (FButtonVariant enum)
// directly instead of `style: FButtonStyle.outline`.

/// Task-queue dashboard — merges `GET /api/v1/tasks/queue_sizes` (SCHEDULED
/// counts per worker type) and `GET /api/v1/tasks/queue_polldata` (last poll
/// heartbeat per worker type) into one flat table, with a per-row "Requeue"
/// action (`POST /api/v1/tasks/queue_requeue/:type`) for stuck IN_PROGRESS
/// instances. Nested as a tab under the "Executions" root entry — see
/// `register.dart` — since queue depth and execution search are both
/// "what's running/stuck right now" operator views checked together.
class TaskQueuePage extends StatefulWidget {
  const TaskQueuePage({super.key});

  @override
  State<TaskQueuePage> createState() => _TaskQueuePageState();
}

class _TaskQueuePageState extends State<TaskQueuePage> {
  final _api = EngineApiClient();
  List<Map<String, dynamic>> _rows = [];
  bool _loading = true;
  String? _error;
  final Set<String> _requeuing = {};

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final results = await Future.wait([_api.queueSizes(), _api.queuePollData()]);
      final sizes = results[0];
      final pollData = results[1];
      final lastPollByType = {
        for (final row in pollData)
          row['worker_type'] as String? ?? '': row['last_poll_at'] as String?,
      };
      final merged = [
        for (final row in sizes)
          {
            'worker_type': row['worker_type'],
            'count': row['count'],
            'last_poll_at': lastPollByType[row['worker_type']],
          },
      ];
      if (mounted) setState(() => _rows = merged);
    } catch (e) {
      if (mounted) setState(() => _error = '$e');
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  static String _relativeTime(String? iso) {
    if (iso == null) return 'Never';
    final at = DateTime.tryParse(iso);
    if (at == null) return iso;
    final diff = DateTime.now().difference(at);
    if (diff.inSeconds < 60) return '${diff.inSeconds}s ago';
    if (diff.inMinutes < 60) return '${diff.inMinutes}m ago';
    if (diff.inHours < 24) return '${diff.inHours}h ago';
    return '${diff.inDays}d ago';
  }

  Future<void> _requeue(String workerType) async {
    setState(() => _requeuing.add(workerType));
    try {
      final requeued = await _api.queueRequeue(workerType);
      if (!mounted) return;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text('Requeued $requeued instance(s) for $workerType')));
      await _load();
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Requeue failed: $e')));
    } finally {
      if (mounted) setState(() => _requeuing.remove(workerType));
    }
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Task Queue'],
        actions: [
          IconButton(onPressed: _load, icon: const Icon(Icons.refresh)),
        ],
      ),
      child: _error != null
            ? Center(child: Text('Failed to load queue data: $_error'))
            : CDataTable<Map<String, dynamic>>(
                isLoading: _loading,
                data: _rows,
                getRowId: (row) => row['worker_type'] as String? ?? '',
                columns: [
                  CDataTableColumn(
                    id: 'worker_type',
                    header: 'Worker type',
                    sortable: true,
                    cell: (row) => Text(row['worker_type'] as String? ?? ''),
                  ),
                  CDataTableColumn(
                    id: 'count',
                    header: 'Scheduled count',
                    cell: (row) => FBadge(child: Text('${row['count'] ?? 0}')),
                  ),
                  CDataTableColumn(
                    id: 'last_poll_at',
                    header: 'Last poll',
                    cell: (row) => Text(_relativeTime(row['last_poll_at'] as String?)),
                  ),
                  CDataTableColumn(
                    id: 'actions',
                    header: 'Actions',
                    cell: (row) {
                      final workerType = row['worker_type'] as String? ?? '';
                      final busy = _requeuing.contains(workerType);
                      return FButton(
                        variant: FButtonVariant.outline,
                        onPress: busy ? null : () => _requeue(workerType),
                        child: busy
                            ? const SizedBox(width: 14, height: 14, child: FCircularProgress())
                            : const Text('Requeue'),
                      );
                    },
                  ),
                ],
              ),
    );
  }
}

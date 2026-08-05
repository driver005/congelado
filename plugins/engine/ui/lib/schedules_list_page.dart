import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'schedule_detail_page.dart';

// NOTE: see shell_scaffold.dart's top-of-file note — FBadge calls below are written from
// Forui's documented component list, not verified against the live package.

/// Root page for the engine plugin's "Schedules" sidebar entry — lists
/// `WorkflowSchedule`s from `GET /api/v1/schedules` via [CDataTable] (search
/// + row selection + a bulk "Delete" command), drills into
/// [ScheduleDetailPage] on row click.
class SchedulesListPage extends StatefulWidget {
  const SchedulesListPage({super.key});

  @override
  State<SchedulesListPage> createState() => _SchedulesListPageState();
}

class _SchedulesListPageState extends State<SchedulesListPage> {
  final _api = EngineApiClient();
  List<Map<String, dynamic>> _schedules = [];
  bool _loading = true;
  String _search = '';
  Set<String> _selected = {};

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    setState(() => _loading = true);
    try {
      final schedules = await _api.listSchedules();
      if (mounted) setState(() => _schedules = schedules);
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  List<Map<String, dynamic>> get _filtered {
    if (_search.isEmpty) return _schedules;
    final query = _search.toLowerCase();
    return _schedules.where((schedule) {
      final name = (schedule['name'] as String? ?? '').toLowerCase();
      final workflow = (schedule['workflow_name'] as String? ?? '').toLowerCase();
      return name.contains(query) || workflow.contains(query);
    }).toList();
  }

  static CStatusVariant _statusVariant(Map<String, dynamic> schedule) {
    if (schedule['paused'] as bool? ?? false) return CStatusVariant.warning;
    if (!(schedule['enabled'] as bool? ?? true)) return CStatusVariant.neutral;
    return CStatusVariant.success;
  }

  static String _statusLabel(Map<String, dynamic> schedule) {
    if (schedule['paused'] as bool? ?? false) return 'Paused';
    if (!(schedule['enabled'] as bool? ?? true)) return 'Disabled';
    return 'Enabled';
  }

  Future<void> _createNew() async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (context) => const ScheduleDetailPage(name: null)),
    );
    if (changed == true) _load();
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Schedules'],
        actions: [
          IconButton(onPressed: _createNew, icon: const Icon(Icons.add)),
          IconButton(onPressed: _load, icon: const Icon(Icons.refresh)),
        ],
      ),
      child: CDataTable<Map<String, dynamic>>(
          isLoading: _loading,
          data: _filtered,
          getRowId: (row) => row['name'] as String? ?? '',
          searchValue: _search,
          onSearchChanged: (value) => setState(() => _search = value),
          columns: [
            CDataTableColumn(
              id: 'name',
              header: 'Name',
              sortable: true,
              cell: (row) => Text(row['name'] as String? ?? '(unnamed)'),
            ),
            CDataTableColumn(
              id: 'workflow_name',
              header: 'Workflow',
              cell: (row) => Text(
                '${row['workflow_name'] ?? ''} v${row['workflow_version'] ?? '?'}',
              ),
            ),
            CDataTableColumn(
              id: 'cron_expression',
              header: 'Cron',
              cell: (row) => CCode(row['cron_expression'] as String? ?? ''),
            ),
            CDataTableColumn(
              id: 'status',
              header: 'Status',
              cell: (row) => CStatusBadge(label: _statusLabel(row), variant: _statusVariant(row)),
            ),
            CDataTableColumn(
              id: 'last_fired_at',
              header: 'Last fired',
              cell: (row) => Text(row['last_fired_at'] as String? ?? 'Never'),
            ),
          ],
          onRowClick: (row) async {
            final name = row['name'] as String? ?? '';
            final changed = await Navigator.of(context).push<bool>(
              MaterialPageRoute(builder: (context) => ScheduleDetailPage(name: name)),
            );
            if (changed == true) _load();
          },
          selectedRowIds: _selected,
          onRowSelectionChanged: (ids) => setState(() => _selected = ids),
          commands: [
            CDataTableCommand(
              label: 'Delete',
              destructive: true,
              onExecute: (ids) async {
                if (!await showCPrompt(
                  context,
                  title: 'Delete ${ids.length} schedule(s)?',
                  description: 'This removes every selected schedule.',
                  confirmLabel: 'Delete',
                )) {
                  return;
                }
                for (final id in ids) {
                  await _api.deleteSchedule(id);
                }
                setState(() => _selected = {});
                await _load();
              },
            ),
          ],
        ),
    );
  }
}

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'task_definition_editor_page.dart';
import 'task_detail_page.dart';

// NOTE: see shell_scaffold.dart's top-of-file note — FBadge calls below are written from
// Forui's documented component list, not verified against the live package.

/// Root page for the engine plugin's "Tasks" sidebar entry — lists task
/// definitions from `GET /api/v1/metadata/tasks` via [CDataTable] (search +
/// row selection + a bulk "Delete" command), drills into [TaskDetailPage]
/// on row click.
class TasksListPage extends StatefulWidget {
  const TasksListPage({super.key});

  @override
  State<TasksListPage> createState() => _TasksListPageState();
}

class _TasksListPageState extends State<TasksListPage> {
  final _api = EngineApiClient();
  List<Map<String, dynamic>> _tasks = [];
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
      final tasks = await _api.listTasks();
      if (mounted) setState(() => _tasks = tasks);
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  List<Map<String, dynamic>> get _filtered {
    if (_search.isEmpty) return _tasks;
    final query = _search.toLowerCase();
    return _tasks.where((task) {
      final name = (task['name'] as String? ?? '').toLowerCase();
      return name.contains(query);
    }).toList();
  }

  Future<void> _createNew() async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (context) => const TaskDefinitionEditorPage()),
    );
    if (changed == true) _load();
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Tasks'],
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
              id: 'worker_type',
              header: 'Worker type',
              cell: (row) => FBadge(child: Text(row['worker_type'] as String? ?? '')),
            ),
          ],
          onRowClick: (row) async {
            final name = row['name'] as String? ?? '';
            final changed = await Navigator.of(context).push<bool>(
              MaterialPageRoute(builder: (context) => TaskDetailPage(taskName: name)),
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
                  title: 'Delete ${ids.length} task(s)?',
                  description: 'This removes every selected task definition.',
                  confirmLabel: 'Delete',
                )) {
                  return;
                }
                for (final id in ids) {
                  await _api.deleteTask(id);
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


import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'workflow_definition_editor_page.dart';
import 'workflow_detail_page.dart';

// NOTE: see shell_scaffold.dart's top-of-file note — FBadge calls below are written from
// Forui's documented component list, not verified against the live package.

/// Root page for the engine plugin's "Workflows" sidebar entry — lists
/// workflow definitions from `GET /api/v1/metadata/workflows` via
/// [CDataTable] (search + row selection + a bulk "Delete" command), drills
/// into [WorkflowDetailPage] on row click.
class WorkflowsListPage extends StatefulWidget {
  const WorkflowsListPage({super.key});

  @override
  State<WorkflowsListPage> createState() => _WorkflowsListPageState();
}

class _WorkflowsListPageState extends State<WorkflowsListPage> {
  final _api = EngineApiClient();
  List<Map<String, dynamic>> _workflows = [];
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
      final workflows = await _api.listWorkflows();
      if (mounted) setState(() => _workflows = workflows);
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  List<Map<String, dynamic>> get _filtered {
    if (_search.isEmpty) return _workflows;
    final query = _search.toLowerCase();
    return _workflows.where((workflow) {
      final name = (workflow['name'] as String? ?? '').toLowerCase();
      return name.contains(query);
    }).toList();
  }

  Future<void> _createNew() async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (context) => const WorkflowDefinitionEditorPage()),
    );
    if (changed == true) _load();
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Workflows'],
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
              id: 'version',
              header: 'Version',
              cell: (row) => FBadge(child: Text('v${row['version'] ?? '?'}')),
            ),
          ],
          onRowClick: (row) async {
            final name = row['name'] as String? ?? '';
            final changed = await Navigator.of(context).push<bool>(
              MaterialPageRoute(builder: (context) => WorkflowDetailPage(workflowName: name)),
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
                  title: 'Delete ${ids.length} workflow(s)?',
                  description: 'This removes every selected workflow definition.',
                  confirmLabel: 'Delete',
                )) {
                  return;
                }
                for (final id in ids) {
                  await _api.deleteWorkflow(id);
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

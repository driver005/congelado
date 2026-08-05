import 'dart:convert';

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'task_definition_editor_page.dart';

// Verified against Forui 0.25.0's real source: FButton takes `variant:` (FButtonVariant enum)
// directly instead of `style: FButtonStyle.outline`. Scaffold/AppBar stay Material (known-safe
// host).

/// Read-only summary + edit/delete for a single task definition, bound by
/// hand to TaskDef's known JSON shape (see
/// plugins/engine/src/model/task/definition.cppm). Pushed from
/// [TasksListPage]; not its own [PluginUiContribution] — drill-down
/// navigation is ordinary Flutter routing, not part of the shell's nav tree.
///
/// Previously this page's edit form only PUT a `{name, worker_type}` object,
/// silently dropping every other TaskDef field (type, retry/timeout policy,
/// schemas, ...) on every save — a real bug, fixed by routing edits through
/// [TaskDefinitionEditorPage]'s full JSON editor instead.
class TaskDetailPage extends StatefulWidget {
  const TaskDetailPage({super.key, required this.taskName});

  final String taskName;

  @override
  State<TaskDetailPage> createState() => _TaskDetailPageState();
}

class _TaskDetailPageState extends State<TaskDetailPage> {
  final _api = EngineApiClient();
  late Future<Map<String, dynamic>> _task = _api.getTask(widget.taskName);

  void _refresh() => setState(() => _task = _api.getTask(widget.taskName));

  Future<void> _edit(Map<String, dynamic> task) async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (context) => TaskDefinitionEditorPage(existing: task)),
    );
    if (changed == true) _refresh();
  }

  Future<void> _delete() async {
    final confirmed = await showCPrompt(
      context,
      title: 'Delete task?',
      description: 'This removes the "${widget.taskName}" task definition.',
      confirmLabel: 'Delete',
    );
    if (!confirmed) return;
    try {
      await _api.deleteTask(widget.taskName);
      if (mounted) Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text('Delete failed: $e')));
    }
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: ['Tasks', widget.taskName],
        actions: [
          IconButton(
            onPressed: _delete,
            icon: const Icon(Icons.delete_outline),
          ),
        ],
      ),
      child: FutureBuilder<Map<String, dynamic>>(
        future: _task,
        builder: (context, snapshot) {
          if (snapshot.connectionState != ConnectionState.done) {
            return const Center(child: FCircularProgress());
          }
          if (snapshot.hasError) {
            return Center(child: Text('Failed to load task: ${snapshot.error}'));
          }
          final task = snapshot.data ?? const {};
          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              mainAxisSize: MainAxisSize.min,
              children: [
                CCodeBlock(const JsonEncoder.withIndent('  ').convert(task)),
                const SizedBox(height: 16),
                FButton(
                  variant: FButtonVariant.outline,
                  onPress: () => _edit(task),
                  child: const Text('Edit'),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}

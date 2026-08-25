import 'dart:convert';

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FTextField's `controller` moved into a `control:`
// FTextFieldControl object.

const _templateTaskDef = {
  'name': '',
  'type': 'SIMPLE',
  'worker_type': '',
};

/// JSON-textarea create/edit for a `TaskDef` — same rationale as
/// [WorkflowDefinitionEditorPage] (matches Conductor's own JSON-editor UX),
/// minus a graph preview (no DAG concept for a single task def).
/// `existing == null` means create mode. Pushed from [TasksListPage]/
/// [TaskDetailPage].
class TaskDefinitionEditorPage extends StatefulWidget {
  const TaskDefinitionEditorPage({super.key, this.existing});

  /// The task definition being edited, or `null` to create a new one.
  final Map<String, dynamic>? existing;

  @override
  State<TaskDefinitionEditorPage> createState() => _TaskDefinitionEditorPageState();
}

class _TaskDefinitionEditorPageState extends State<TaskDefinitionEditorPage> {
  final _api = EngineApiClient();
  late final _controller = TextEditingController(
    text: const JsonEncoder.withIndent('  ').convert(widget.existing ?? _templateTaskDef),
  );
  bool _saving = false;

  bool get _isCreate => widget.existing == null;

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  Future<void> _save() async {
    late final Map<String, dynamic> body;
    try {
      body = jsonDecode(_controller.text) as Map<String, dynamic>;
    } catch (e) {
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text('Invalid JSON: $e')));
      return;
    }
    setState(() => _saving = true);
    try {
      if (_isCreate) {
        await _api.createTask(body);
      } else {
        final name = widget.existing!['name'] as String? ?? body['name'] as String? ?? '';
        await _api.updateTask(name, body);
      }
      if (mounted) Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Save failed: $e')));
    } finally {
      if (mounted) setState(() => _saving = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(breadcrumbs: ['Tasks', _isCreate ? 'New' : 'Edit']),
      child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
              child: FTextField.multiline(
                control: FTextFieldControl.managed(controller: _controller),
                minLines: null,
                maxLines: null,
                expands: true,
              ),
            ),
            const SizedBox(height: 12),
            FButton(
              onPress: _saving ? null : _save,
              child: _saving
                  ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                  : const Text('Save'),
            ),
          ],
        ),
    );
  }
}

import 'dart:convert';

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FTextField's `controller` moved into a `control:`
// FTextFieldControl object.

const _templateWorkflowDef = {
  'name': '',
  'version': 1,
  'nodes': [],
};

/// JSON-textarea create/edit for a `WorkflowDef`, with a live graph preview
/// alongside — matches Conductor's own definition-editing UX (a JSON/YAML
/// editor, not a drag-and-drop builder). `existing == null` means create
/// mode. Pushed from [WorkflowsListPage]/[WorkflowDetailPage].
class WorkflowDefinitionEditorPage extends StatefulWidget {
  const WorkflowDefinitionEditorPage({super.key, this.existing});

  /// The workflow definition being edited, or `null` to create a new one.
  final Map<String, dynamic>? existing;

  @override
  State<WorkflowDefinitionEditorPage> createState() => _WorkflowDefinitionEditorPageState();
}

class _WorkflowDefinitionEditorPageState extends State<WorkflowDefinitionEditorPage> {
  final _api = EngineApiClient();
  late final _controller = TextEditingController(
    text: const JsonEncoder.withIndent('  ').convert(widget.existing ?? _templateWorkflowDef),
  );
  bool _saving = false;
  String? _parseError;
  List<CWorkflowGraphNode> _previewNodes = [];
  List<CWorkflowGraphEdge> _previewEdges = [];

  bool get _isCreate => widget.existing == null;

  @override
  void initState() {
    super.initState();
    _updatePreview(_controller.text);
    _controller.addListener(() => _updatePreview(_controller.text));
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _updatePreview(String text) {
    try {
      final decoded = jsonDecode(text) as Map<String, dynamic>;
      // Keyed by ref_name, not task_def_name — see workflow_graph.dart's own doc comment on why:
      // TaskEdge.from/to and TaskInstance.node_ref key off ref_name, since one TaskDef can sit at
      // more than one DAG position.
      final rawNodes = (decoded['nodes'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>();
      final nodes = [
        for (final node in rawNodes)
          CWorkflowGraphNode(
            id: node['ref_name'] as String? ?? '',
            label: node['ref_name'] as String? ?? '',
            taskDefName: node['task_def_name'] as String?,
          ),
      ];
      final edges = [
        for (final node in rawNodes)
          for (final edge in (node['edges'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>())
            CWorkflowGraphEdge(
              from: node['ref_name'] as String? ?? '',
              to: edge['to'] as String? ?? '',
              condition: edge['condition'] as String?,
            ),
      ];
      // Only replace the last-good preview once parsing actually succeeds — keeps the diagram
      // stable (rather than flashing empty) while the user is mid-edit with invalid JSON.
      setState(() {
        _parseError = null;
        _previewNodes = nodes;
        _previewEdges = edges;
      });
    } catch (e) {
      setState(() => _parseError = '$e');
    }
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
        await _api.createWorkflow(body);
      } else {
        final name = widget.existing!['name'] as String? ?? body['name'] as String? ?? '';
        await _api.updateWorkflow(name, body);
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
      topbar: CPageTopbar(breadcrumbs: ['Workflows', _isCreate ? 'New' : 'Edit']),
      child: Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
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
            ),
            const SizedBox(width: 16),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  CHeading('Graph preview', level: CHeadingLevel.h5),
                  const SizedBox(height: 8),
                  if (_parseError != null) CInlineTip('Invalid JSON — fix to preview the graph.'),
                  const SizedBox(height: 8),
                  Expanded(
                    child: CContainer(
                      child: CWorkflowGraph(nodes: _previewNodes, edges: _previewEdges),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
    );
  }
}

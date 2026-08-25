import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'workflow_definition_editor_page.dart';
import 'workflow_execution_page.dart';

// Verified against Forui 0.25.0's real source: FButton takes `variant:` (FButtonVariant enum)
// directly instead of `style: FButtonStyle.outline`; FDialog dropped `title`/`body`/`actions` in
// favor of a single `builder: (context, style) => Widget` (no built-in layout ships in the
// package — hand-composed here using FDialogStyle's own titleTextStyle).

/// Detail view for a single workflow definition, bound by hand to
/// WorkflowDef's known JSON shape (`name`, `version`, `nodes` — see
/// plugins/engine/src/model/workflow/definition.cppm). Pushed from
/// [WorkflowsListPage]; not its own [PluginUiContribution] — drill-down
/// navigation is ordinary Flutter routing, not part of the shell's nav tree.
class WorkflowDetailPage extends StatefulWidget {
  const WorkflowDetailPage({super.key, required this.workflowName});

  final String workflowName;

  @override
  State<WorkflowDetailPage> createState() => _WorkflowDetailPageState();
}

class _WorkflowDetailPageState extends State<WorkflowDetailPage> {
  final _api = EngineApiClient();
  late Future<Map<String, dynamic>> _workflow = _api.getWorkflow(
    widget.workflowName,
  );
  bool _starting = false;

  Future<void> _start() async {
    var seedVariables = <String, String>{};
    final proceed = await showDialog<bool>(
      context: context,
      builder: (dialogContext) => FDialog(
        builder: (context, style) => Padding(
          padding: const EdgeInsets.all(16),
          child: StatefulBuilder(
            builder: (context, setDialogState) => Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text('Start execution', style: style.titleTextStyle),
                const SizedBox(height: 8),
                CKeyValueEditor(
                  value: seedVariables,
                  onChanged: (value) => setDialogState(() => seedVariables = value),
                ),
                const SizedBox(height: 16),
                Row(
                  mainAxisAlignment: MainAxisAlignment.end,
                  children: [
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: () => Navigator.pop(dialogContext, false),
                      child: const Text('Cancel'),
                    ),
                    const SizedBox(width: 8),
                    FButton(
                      onPress: () => Navigator.pop(dialogContext, true),
                      child: const Text('Start'),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
    if (proceed != true) return;

    setState(() => _starting = true);
    try {
      final execution = await _api.startWorkflow(widget.workflowName, variables: seedVariables);
      if (!mounted) return;
      final execId = execution['exec_id'] as String? ?? '';
      await Navigator.of(context).push(
        MaterialPageRoute(
          builder: (context) => WorkflowExecutionPage(execId: execId),
        ),
      );
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text('Start failed: $e')));
    } finally {
      if (mounted) setState(() => _starting = false);
    }
  }

  Future<void> _edit(Map<String, dynamic> workflow) async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(
        builder: (context) => WorkflowDefinitionEditorPage(existing: workflow),
      ),
    );
    if (changed == true && mounted) {
      setState(() => _workflow = _api.getWorkflow(widget.workflowName));
    }
  }

  Future<void> _delete() async {
    final confirmed = await showCPrompt(
      context,
      title: 'Delete workflow?',
      description: 'This removes the "${widget.workflowName}" workflow definition.',
      confirmLabel: 'Delete',
    );
    if (!confirmed) return;
    try {
      await _api.deleteWorkflow(widget.workflowName);
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
        breadcrumbs: ['Workflows', widget.workflowName],
        actions: [
          IconButton(
            onPressed: _delete,
            icon: const Icon(Icons.delete_outline),
          ),
        ],
      ),
      child: FutureBuilder<Map<String, dynamic>>(
        future: _workflow,
        builder: (context, snapshot) {
          if (snapshot.connectionState != ConnectionState.done) {
            return const Center(child: FCircularProgress());
          }
          if (snapshot.hasError) {
            return Center(
              child: Text('Failed to load workflow: ${snapshot.error}'),
            );
          }
          final workflow = snapshot.data ?? const {};
          final rawNodes = (workflow['nodes'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>();
          // Keyed by ref_name, not task_def_name — TaskEdge.from/to and TaskInstance.node_ref key
          // off ref_name (the DAG-position identity), since one TaskDef can sit at more than one
          // DAG position (join branches, loop bodies, dynamic-fork branches). See
          // `plugins/engine/src/model/workflow/dag.cppm`.
          final graphNodes = [
            for (final node in rawNodes)
              CWorkflowGraphNode(
                id: node['ref_name'] as String? ?? '',
                label: node['ref_name'] as String? ?? '',
                taskDefName: node['task_def_name'] as String?,
              ),
          ];
          final graphEdges = [
            for (final node in rawNodes)
              for (final edge in (node['edges'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>())
                CWorkflowGraphEdge(
                  from: node['ref_name'] as String? ?? '',
                  to: edge['to'] as String? ?? '',
                  condition: edge['condition'] as String?,
                ),
          ];
          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    FBadge(child: Text('v${workflow['version'] ?? '?'}')),
                    const SizedBox(width: 8),
                    FBadge(child: Text('${rawNodes.length} node(s)')),
                  ],
                ),
                const SizedBox(height: 16),
                Row(
                  children: [
                    FButton(
                      onPress: _starting ? null : _start,
                      child: _starting
                          ? const SizedBox(
                              width: 16,
                              height: 16,
                              child: FCircularProgress(),
                            )
                          : const Text('Start execution'),
                    ),
                    const SizedBox(width: 8),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: () => _edit(workflow),
                      child: const Text('Edit'),
                    ),
                  ],
                ),
                const SizedBox(height: 16),
                CHeading('Graph', level: CHeadingLevel.h4),
                const SizedBox(height: 8),
                Expanded(
                  child: CContainer(
                    child: CWorkflowGraph(nodes: graphNodes, edges: graphEdges),
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}

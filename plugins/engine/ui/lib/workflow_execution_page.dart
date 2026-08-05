import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FButton takes `variant:` (FButtonVariant enum)
// directly instead of `style: FButtonStyle.outline`; FDialog dropped `title`/`body`/`actions`
// in favor of a single `builder: (context, style) => Widget` (no built-in layout ships in the
// package — hand-composed here using FDialogStyle's own titleTextStyle); FSelect now takes
// `items: Map<String, T>` + `control: FSelectControl<T>` instead of `format`/`initialValue`/
// `children: [FSelectItem(...)]`; FTextField's `controller` moved into `control:`.

const _terminalWorkflowStatuses = {'COMPLETED', 'FAILED', 'TIMED_OUT', 'TERMINATED'};

/// Everything one execution's page needs loaded together — the execution
/// itself, its definition (for the static DAG + per-node task type), and
/// every task def (to resolve each node's `TaskDef.type` by
/// `task_def_name`).
class _ExecutionPageData {
  const _ExecutionPageData({required this.execution, required this.def, required this.taskDefTypes});

  final Map<String, dynamic> execution;
  final Map<String, dynamic> def;
  final Map<String, String> taskDefTypes;
}

/// Single workflow execution view, bound by hand to WorkflowExecution's
/// known JSON shape (`exec_id`, `def_name`, `status`, `variables`,
/// `task_instances` — see plugins/engine/src/model/workflow/exec.cppm).
/// Reachable from [WorkflowDetailPage]'s "Start execution" action, or by row
/// from the `engine.executions` [PluginUiContribution]
/// (`executions_page.dart`) — not its own top-level nav entry.
class WorkflowExecutionPage extends StatefulWidget {
  const WorkflowExecutionPage({super.key, required this.execId});

  final String execId;

  @override
  State<WorkflowExecutionPage> createState() => _WorkflowExecutionPageState();
}

class _WorkflowExecutionPageState extends State<WorkflowExecutionPage> {
  final _api = EngineApiClient();
  late Future<_ExecutionPageData> _data = _load();

  // One bool per action instead of a single "busy" flag, so an in-flight action doesn't also
  // gray out every *other* button on the page — same per-action pattern _terminating already
  // used, extended to the rest of the action set.
  bool _pausing = false;
  bool _resuming = false;
  bool _retrying = false;
  bool _restarting = false;
  bool _terminating = false;

  void _refresh() => setState(() => _data = _load());

  Future<_ExecutionPageData> _load() async {
    final execution = await _api.getExecution(widget.execId);
    final defName = execution['def_name'] as String? ?? '';
    final results = await Future.wait([
      _api.getWorkflow(defName),
      _api.listTasks(),
    ]);
    final def = results[0] as Map<String, dynamic>;
    final taskDefs = results[1] as List<Map<String, dynamic>>;
    final taskDefTypes = {
      for (final task in taskDefs)
        (task['name'] as String? ?? ''): (task['type'] as String? ?? ''),
    };
    return _ExecutionPageData(execution: execution, def: def, taskDefTypes: taskDefTypes);
  }

  static CWorkflowGraphNode _graphNodeFrom(
    Map<String, dynamic> node,
    Map<String, Map<String, dynamic>> instanceByNodeRef,
    Map<String, String> taskDefTypes,
  ) {
    final refName = node['ref_name'] as String? ?? '';
    final taskDefName = node['task_def_name'] as String?;
    final instance = instanceByNodeRef[refName];
    return CWorkflowGraphNode(
      id: refName,
      label: refName,
      taskDefName: taskDefName,
      taskType: taskDefTypes[taskDefName],
      status: instance == null ? null : statusVariantForTaskStatus(instance['status'] as String? ?? ''),
    );
  }

  static CTimelineTask _timelineTaskFrom(Map<String, dynamic> instance) {
    final timings = instance['timings'] as Map<String, dynamic>? ?? const {};
    return CTimelineTask(
      label: instance['def_name'] as String? ?? '(unnamed)',
      scheduledAt: DateTime.tryParse(timings['scheduled_at'] as String? ?? ''),
      startedAt: DateTime.tryParse(timings['started_at'] as String? ?? ''),
      completedAt: DateTime.tryParse(timings['completed_at'] as String? ?? ''),
      variant: statusVariantForTaskStatus(instance['status'] as String? ?? ''),
    );
  }

  Future<void> _runAction(
    Future<void> Function() action, {
    required void Function(bool busy) setBusy,
  }) async {
    setBusy(true);
    try {
      await action();
      _refresh();
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Action failed: $e')));
    } finally {
      if (mounted) setBusy(false);
    }
  }

  Future<void> _rerunDialog(List<Map<String, dynamic>> taskInstances) async {
    String? nodeRef = taskInstances.isEmpty ? null : taskInstances.first['node_ref'] as String?;
    var input = <String, String>{};
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
                Text('Rerun', style: style.titleTextStyle),
                const SizedBox(height: 8),
                const CInlineTip(
                  'Resets this one node and re-runs from there — not a full clone-and-replay of '
                  'the whole execution.',
                ),
                const SizedBox(height: 8),
                FSelect<String>(
                  hint: 'Task reference name',
                  items: {
                    for (final instance in taskInstances)
                      (instance['node_ref'] as String? ?? ''): (instance['node_ref'] as String? ?? ''),
                  },
                  control: FSelectControl<String>.managed(
                    initial: nodeRef,
                    onChange: (value) => setDialogState(() => nodeRef = value),
                  ),
                ),
                const SizedBox(height: 8),
                CKeyValueEditor(
                  value: input,
                  onChanged: (value) => setDialogState(() => input = value),
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
                      child: const Text('Rerun'),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
    if (proceed != true || nodeRef == null || nodeRef!.isEmpty) return;
    await _runAction(
      () => _api.rerunExecution(widget.execId, nodeRef: nodeRef!, input: input),
      setBusy: (_) {},
    );
  }

  Future<void> _signalDialog(List<Map<String, dynamic>> taskInstances) async {
    final inProgress =
        taskInstances.where((instance) => instance['status'] == 'IN_PROGRESS').toList();
    String? nodeRef = inProgress.isEmpty ? null : inProgress.first['node_ref'] as String?;
    final payloadController = TextEditingController();
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
                Text('Signal', style: style.titleTextStyle),
                const SizedBox(height: 8),
                const CInlineTip('Wakes a WAIT/HUMAN node currently waiting on this execution.'),
                const SizedBox(height: 8),
                FSelect<String>(
                  hint: 'Task reference name',
                  items: {
                    for (final instance in inProgress)
                      (instance['node_ref'] as String? ?? ''): (instance['node_ref'] as String? ?? ''),
                  },
                  control: FSelectControl<String>.managed(
                    initial: nodeRef,
                    onChange: (value) => setDialogState(() => nodeRef = value),
                  ),
                ),
                const SizedBox(height: 8),
                FTextField(
                  control: FTextFieldControl.managed(controller: payloadController),
                  hint: 'Payload (optional)',
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
                      child: const Text('Signal'),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
    if (proceed != true || nodeRef == null || nodeRef!.isEmpty) return;
    final payload = payloadController.text.isEmpty ? null : payloadController.text;
    await _runAction(
      () => _api.signalExecution(widget.execId, nodeRef: nodeRef!, payload: payload),
      setBusy: (_) {},
    );
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: ['Executions', widget.execId],
        actions: [
          IconButton(onPressed: _refresh, icon: const Icon(Icons.refresh)),
        ],
      ),
      child: FutureBuilder<_ExecutionPageData>(
        future: _data,
        builder: (context, snapshot) {
          if (snapshot.connectionState != ConnectionState.done) {
            return const Center(child: FCircularProgress());
          }
          if (snapshot.hasError) {
            return Center(
              child: Text('Failed to load execution: ${snapshot.error}'),
            );
          }
          final data = snapshot.data!;
          final execution = data.execution;
          final status = execution['status'] as String? ?? 'UNKNOWN';
          final defName = execution['def_name'] as String? ?? '';
          final variables =
              (execution['variables'] as Map<String, dynamic>? ?? const {});
          final terminal = _terminalWorkflowStatuses.contains(status);

          final taskInstances =
              (execution['task_instances'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>();
          final timelineTasks = [for (final instance in taskInstances) _timelineTaskFrom(instance)];

          // Static DAG (keyed by ref_name — see workflow_detail_page.dart's own note on why,
          // not task_def_name) joined against this execution's task_instances (keyed by
          // node_ref) for a per-node status overlay.
          final instanceByNodeRef = {
            for (final instance in taskInstances) instance['node_ref'] as String? ?? '': instance,
          };
          final rawNodes =
              (data.def['nodes'] as List<dynamic>? ?? const []).cast<Map<String, dynamic>>();
          final graphNodes = [
            for (final node in rawNodes)
              _graphNodeFrom(node, instanceByNodeRef, data.taskDefTypes),
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
                    CStatusBadge(label: status, variant: statusVariantForWorkflowStatus(status)),
                    const SizedBox(width: 8),
                    Text(defName),
                  ],
                ),
                if (variables.isNotEmpty) ...[
                  const SizedBox(height: 16),
                  const Text('Variables'),
                  for (final entry in variables.entries)
                    Text('${entry.key}: ${entry.value}'),
                ],
                const SizedBox(height: 16),
                Wrap(
                  spacing: 8,
                  runSpacing: 8,
                  children: [
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: status != 'RUNNING' || _pausing
                          ? null
                          : () => _runAction(
                                () => _api.pauseExecution(widget.execId),
                                setBusy: (v) => setState(() => _pausing = v),
                              ),
                      child: _pausing
                          ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                          : const Text('Pause'),
                    ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: status != 'PAUSED' || _resuming
                          ? null
                          : () => _runAction(
                                () => _api.resumeExecution(widget.execId),
                                setBusy: (v) => setState(() => _resuming = v),
                              ),
                      child: _resuming
                          ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                          : const Text('Resume'),
                    ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: status != 'FAILED' || _retrying
                          ? null
                          : () => _runAction(
                                () => _api.retryExecution(widget.execId),
                                setBusy: (v) => setState(() => _retrying = v),
                              ),
                      child: _retrying
                          ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                          : const Text('Retry'),
                    ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: !terminal || _restarting
                          ? null
                          : () => _runAction(
                                () => _api.restartExecution(widget.execId),
                                setBusy: (v) => setState(() => _restarting = v),
                              ),
                      child: _restarting
                          ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                          : const Text('Restart'),
                    ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: taskInstances.isEmpty ? null : () => _rerunDialog(taskInstances),
                      child: const Text('Rerun'),
                    ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: () => _signalDialog(taskInstances),
                      child: const Text('Signal'),
                    ),
                    FButton(
                      variant: FButtonVariant.destructive,
                      onPress: terminal || _terminating
                          ? null
                          : () => _runAction(
                                () => _api.terminateExecution(widget.execId),
                                setBusy: (v) => setState(() => _terminating = v),
                              ),
                      child: _terminating
                          ? const SizedBox(
                              width: 16,
                              height: 16,
                              child: FCircularProgress(),
                            )
                          : const Text('Terminate'),
                    ),
                  ],
                ),
                const SizedBox(height: 16),
                CHeading('Graph', level: CHeadingLevel.h4),
                const CHint(
                  'Rendered against the current stored definition, not a version pinned to this '
                  'run — if the definition changed since this execution started, the diagram may '
                  'not exactly match what actually ran.',
                ),
                const SizedBox(height: 8),
                SizedBox(
                  height: 240,
                  child: CContainer(
                    child: CWorkflowGraph(nodes: graphNodes, edges: graphEdges),
                  ),
                ),
                const SizedBox(height: 16),
                CHeading('Timeline', level: CHeadingLevel.h4),
                const SizedBox(height: 8),
                Expanded(
                  child: CContainer(
                    child: CExecutionTimeline(tasks: timelineTasks),
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

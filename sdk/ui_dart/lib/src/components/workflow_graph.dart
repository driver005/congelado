import 'package:flutter/material.dart' show Icons, Tooltip;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';
import 'package:graphview/GraphView.dart';

import 'status_badge.dart' show CStatusVariant, cStatusColor;

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note for the standing
// Forui-API-not-verified caveat. `graphview`'s API (Graph/Node.Id/graph.addEdge/GraphView.builder/
// SugiyamaAlgorithm) is confirmed against the package's own pub.dev docs (not a blind guess like
// several Forui calls elsewhere), but still unverified against the literal installed package.

/// `TaskType` values (`plugins/engine/src/model/task/status.cppm`) that are
/// modeled in the DAG but not yet wired up in the orchestrator — see
/// `orchestrator.cppm`'s own doc comment. Flagged visually in the graph
/// (dashed border + warning icon) rather than rendered as if they run like
/// every other node type.
const Set<String> _unsupportedPreviewTaskTypes = {'DO_WHILE', 'FORK_JOIN_DYNAMIC'};

/// One node in a [CWorkflowGraph] — id/label key off `TaskNode.ref_name`
/// (the DAG-position identity), NOT `task_def_name` — one `TaskDef` can sit
/// at more than one DAG position (join branches, loop bodies, dynamic-fork
/// branches), so `ref_name` is what `TaskEdge.from`/`to` and
/// `TaskInstance.node_ref` actually key off
/// (`plugins/engine/src/model/workflow/dag.cppm`).
class CWorkflowGraphNode {
  const CWorkflowGraphNode({
    required this.id,
    required this.label,
    this.taskDefName,
    this.taskType,
    this.status,
  });

  /// The node's `ref_name`.
  final String id;

  /// Display label — normally also the `ref_name`; [taskDefName] is shown as
  /// a secondary line when it differs.
  final String label;

  /// The `TaskDef` this node runs, if known (`TaskNode.task_def_name`).
  final String? taskDefName;

  /// The resolved `TaskDef.type` for this node, if known (e.g. `"FORK_JOIN"`,
  /// `"SWITCH"`) — used only to render the type subtitle/chip and the
  /// unsupported-preview flag, not part of the graph's structure.
  final String? taskType;

  /// This node's current execution status, or `null` when rendering a bare
  /// definition with no execution context (the node card then renders with
  /// its old plain outline, unchanged).
  final CStatusVariant? status;
}

/// One edge in a [CWorkflowGraph] — mirrors `TaskNode.edges[]`'s `to`/`condition`
/// fields (`dag.cppm`'s `TaskEdge`; `from` is a branch/condition label on the
/// edge itself, not a second node reference, so it isn't used for the graph's
/// structure here — only `to` determines which node this edge points at).
class CWorkflowGraphEdge {
  const CWorkflowGraphEdge({required this.from, required this.to, this.condition});

  final String from;
  final String to;
  final String? condition;
}

/// A workflow definition's node/edge DAG, laid out and rendered as a diagram
/// — mirrors Conductor OSS's `reaflow`-backed `GraphPanel.tsx`
/// (`ui-next/src/pages/definition/GraphPanel.tsx`). `SugiyamaAlgorithm`
/// (layered-graph layout) is used rather than a tree algorithm since a
/// workflow can have join nodes with multiple parents.
class CWorkflowGraph extends StatelessWidget {
  const CWorkflowGraph({super.key, required this.nodes, required this.edges});

  final List<CWorkflowGraphNode> nodes;
  final List<CWorkflowGraphEdge> edges;

  @override
  Widget build(BuildContext context) {
    if (nodes.isEmpty) {
      final colors = FTheme.of(context).colors;
      return Center(
        child: Text('No nodes defined.', style: TextStyle(color: colors.mutedForeground)),
      );
    }

    final graph = Graph()..isTree = false;
    final graphNodes = {for (final node in nodes) node.id: Node.Id(node.id)};
    for (final node in graphNodes.values) {
      graph.addNode(node);
    }
    for (final edge in edges) {
      final from = graphNodes[edge.from];
      final to = graphNodes[edge.to];
      if (from != null && to != null) {
        graph.addEdge(from, to);
      }
    }

    final algorithm = SugiyamaAlgorithm(SugiyamaConfiguration()
      ..nodeSeparation = 24
      ..levelSeparation = 48
      ..orientation = SugiyamaConfiguration.ORIENTATION_TOP_BOTTOM);

    return InteractiveViewer(
      constrained: false,
      boundaryMargin: const EdgeInsets.all(48),
      minScale: 0.2,
      maxScale: 2,
      child: GraphView(
        graph: graph,
        algorithm: algorithm,
        builder: (node) {
          final id = node.key!.value as String;
          final graphNode = nodes.firstWhere(
            (n) => n.id == id,
            orElse: () => CWorkflowGraphNode(id: id, label: id),
          );
          return _CWorkflowGraphNodeCard(node: graphNode);
        },
      ),
    );
  }
}

class _CWorkflowGraphNodeCard extends StatelessWidget {
  const _CWorkflowGraphNodeCard({required this.node});

  final CWorkflowGraphNode node;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    final isPreview = _unsupportedPreviewTaskTypes.contains(node.taskType);
    final subtitleParts = [
      if (node.taskDefName != null && node.taskDefName != node.label) node.taskDefName!,
      if (node.taskType != null) node.taskType!,
    ];
    // Flutter's Border has no native dashed style without a custom painter/extra dependency —
    // an amber outline (this codebase's existing "warning" color) is the preview flag instead of
    // a literal dash pattern, kept as a solid Border so every side still renders.
    final outlineColor = isPreview ? cStatusColor(colors, CStatusVariant.warning) : colors.border;
    final statusColor = node.status == null ? null : cStatusColor(colors, node.status!);

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: colors.background,
        border: Border(
          left: BorderSide(color: statusColor ?? outlineColor, width: statusColor == null ? 1 : 4),
          top: BorderSide(color: outlineColor),
          right: BorderSide(color: outlineColor),
          bottom: BorderSide(color: outlineColor),
        ),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(node.label, style: TextStyle(color: colors.foreground, fontSize: 13)),
              if (isPreview) ...[
                const SizedBox(width: 4),
                Tooltip(
                  message:
                      'Modeled but not yet executed by the orchestrator — preview only.',
                  child: Icon(Icons.science_outlined, size: 14, color: colors.mutedForeground),
                ),
              ],
            ],
          ),
          if (subtitleParts.isNotEmpty)
            Text(
              subtitleParts.join(' · '),
              style: TextStyle(color: colors.mutedForeground, fontSize: 10),
            ),
        ],
      ),
    );
  }
}

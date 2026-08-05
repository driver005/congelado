import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

import 'status_badge.dart' show CStatusVariant, cStatusColor;

// Verified against Forui 0.25.0's real source: FTheme.of(context).colors is typed FColors
// (renamed from the FColorScheme this was originally guessed as).

/// One row in a [CExecutionTimeline] — mirrors `TaskInstance.timings`'
/// `scheduled_at`/`started_at`/`completed_at` fields
/// (`plugins/engine/src/model/common/timestamps.cppm`'s `ExecutionTimings`,
/// as attached to a `TaskInstance` — `plugins/engine/src/model/task/instance.cppm`).
/// A task still running has `completedAt == null` — its bar extends to "now".
class CTimelineTask {
  const CTimelineTask({
    required this.label,
    this.scheduledAt,
    this.startedAt,
    this.completedAt,
    this.variant = CStatusVariant.neutral,
  });

  final String label;
  final DateTime? scheduledAt;
  final DateTime? startedAt;
  final DateTime? completedAt;
  final CStatusVariant variant;

  DateTime? get _barStart => startedAt ?? scheduledAt;
}

/// A Gantt-style bar chart of task execution within one workflow run —
/// mirrors Conductor OSS's `vis-timeline`-backed `Timeline.tsx`
/// (`ui-next/src/pages/execution/Timeline.tsx`). Hand-rolled (`Stack`/
/// `Positioned` bars against a shared axis computed via `LayoutBuilder`) —
/// no charting/gantt dependency, per this feature's own scoping decision
/// (a Gantt bar is just start/width positioning, not worth a new
/// dependency the way the DAG layout in `workflow_graph.dart` was).
class CExecutionTimeline extends StatelessWidget {
  const CExecutionTimeline({super.key, required this.tasks, this.rowHeight = 32});

  final List<CTimelineTask> tasks;
  final double rowHeight;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    final withStart = tasks.where((task) => task._barStart != null).toList();
    if (withStart.isEmpty) {
      return Center(
        child: Text('No timing data yet.', style: TextStyle(color: colors.mutedForeground)),
      );
    }

    final now = DateTime.now();
    final rangeStart = withStart.map((task) => task._barStart!).reduce((a, b) => a.isBefore(b) ? a : b);
    final rangeEnd = withStart
        .map((task) => task.completedAt ?? now)
        .reduce((a, b) => a.isAfter(b) ? a : b);
    final totalMicros = rangeEnd.difference(rangeStart).inMicroseconds;
    // A single instantaneous task (or all tasks sharing one timestamp) would divide by zero below
    // — floor it to 1 so that task still renders as a thin bar instead of crashing.
    final safeTotalMicros = totalMicros <= 0 ? 1 : totalMicros;

    return LayoutBuilder(
      builder: (context, constraints) {
        const labelWidth = 140.0;
        final axisWidth = (constraints.maxWidth - labelWidth).clamp(0.0, double.infinity);
        return Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            for (final task in tasks)
              SizedBox(
                height: rowHeight,
                child: Row(
                  children: [
                    SizedBox(
                      width: labelWidth,
                      child: Text(
                        task.label,
                        overflow: TextOverflow.ellipsis,
                        style: TextStyle(fontSize: 12, color: colors.foreground),
                      ),
                    ),
                    SizedBox(
                      width: axisWidth,
                      child: task._barStart == null
                          ? null
                          : _buildBar(colors, task, rangeStart, safeTotalMicros, axisWidth),
                    ),
                  ],
                ),
              ),
          ],
        );
      },
    );
  }

  Widget _buildBar(
    FColors colors,
    CTimelineTask task,
    DateTime rangeStart,
    int safeTotalMicros,
    double axisWidth,
  ) {
    final barStart = task._barStart!;
    final barEnd = task.completedAt ?? DateTime.now();
    final left = barStart.difference(rangeStart).inMicroseconds / safeTotalMicros * axisWidth;
    final width = (barEnd.difference(barStart).inMicroseconds / safeTotalMicros * axisWidth)
        .clamp(2.0, axisWidth);

    final barColor = cStatusColor(colors, task.variant);

    return Stack(
      children: [
        Positioned(
          left: left.clamp(0.0, axisWidth),
          width: width,
          top: 6,
          bottom: 6,
          child: DecoratedBox(
            decoration: BoxDecoration(color: barColor, borderRadius: BorderRadius.circular(4)),
          ),
        ),
      ],
    );
  }
}

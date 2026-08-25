import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';

import 'event_handlers_list_page.dart';
import 'executions_page.dart';
import 'schedules_list_page.dart';
import 'sql_query_page.dart';
import 'task_queue_page.dart';
import 'tasks_list_page.dart';
import 'workflows_list_page.dart';

/// This plugin's nav contribution(s), imported and merged by
/// app/lib/registry.dart. Adding a page: append a [PluginUiContribution]
/// here — no other file in this package or the shell needs to change.
///
/// `engine.executions` is the first real use of [PluginUiContribution.parentId]
/// tab-grouping in this codebase: it and `engine.executions.queue` are
/// operationally paired ("what's running/stuck right now") but distinct
/// enough concerns to be tabs, not a merged page — see
/// `app/lib/shell/shell_scaffold.dart` for how `parentId` groups render.
final List<PluginUiContribution> contributions = [
  const PluginUiContribution(
    id: 'engine.tasks',
    label: 'Tasks',
    icon: Icons.list_alt,
    order: 10,
    builder: _buildTasksListPage,
  ),
  const PluginUiContribution(
    id: 'engine.workflows',
    label: 'Workflows',
    icon: Icons.account_tree_outlined,
    order: 20,
    builder: _buildWorkflowsListPage,
  ),
  // Root entry — its own builder is only reached if shell_scaffold.dart ever renders a
  // parentId-having root with no children (it doesn't today, children are always shown as tabs
  // when present); kept pointed at the Search tab as a harmless fallback either way.
  const PluginUiContribution(
    id: 'engine.executions',
    label: 'Executions',
    icon: Icons.timeline_outlined,
    order: 25,
    builder: _buildExecutionsPage,
  ),
  const PluginUiContribution(
    id: 'engine.executions.search',
    parentId: 'engine.executions',
    label: 'Search',
    order: 10,
    builder: _buildExecutionsPage,
  ),
  const PluginUiContribution(
    id: 'engine.executions.queue',
    parentId: 'engine.executions',
    label: 'Task Queue',
    order: 20,
    builder: _buildTaskQueuePage,
  ),
  const PluginUiContribution(
    id: 'engine.event_handlers',
    label: 'Event Handlers',
    icon: Icons.bolt_outlined,
    order: 27,
    builder: _buildEventHandlersListPage,
  ),
  const PluginUiContribution(
    id: 'engine.schedules',
    label: 'Schedules',
    icon: Icons.schedule_outlined,
    order: 28,
    builder: _buildSchedulesListPage,
  ),
  const PluginUiContribution(
    id: 'engine.sql',
    label: 'SQL',
    icon: Icons.storage_outlined,
    order: 30,
    builder: _buildSqlQueryPage,
  ),
];

Widget _buildTasksListPage(BuildContext context) => const TasksListPage();
Widget _buildWorkflowsListPage(BuildContext context) =>
    const WorkflowsListPage();
Widget _buildExecutionsPage(BuildContext context) => const ExecutionsPage();
Widget _buildTaskQueuePage(BuildContext context) => const TaskQueuePage();
Widget _buildEventHandlersListPage(BuildContext context) => const EventHandlersListPage();
Widget _buildSchedulesListPage(BuildContext context) => const SchedulesListPage();
Widget _buildSqlQueryPage(BuildContext context) => const SqlQueryPage();

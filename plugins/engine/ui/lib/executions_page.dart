import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';

import 'api_client.dart';
import 'workflow_execution_page.dart';

/// Cross-workflow execution search — mirrors Conductor OSS's `executions`
/// page (`ui-next/src/pages/executions`), which searches/filters across
/// every workflow run rather than drilling in one at a time. Backed by the
/// real `POST /api/v1/workflow/search` projection endpoint
/// (`plugins/engine/src/core/handler/search.cppm`) — this page used to run a
/// raw `SELECT` through the SQL query viewer's own plumbing because that
/// endpoint predated this page; now rewired onto the real search API.
///
/// `SearchRequestBody` has no structured status/date-range field — only
/// `query` (backend-grammar-dependent, e.g. Lucene-style for most
/// `ISearchProvider` backends) and `free_text` (genuine full-text). The
/// status filter below is client-concatenated into `query` as
/// `status:VALUE`; a SQL-WHERE-style provider would need a different string
/// here, there's no way to sidestep that from this response shape alone.
/// Search is optional infra — a `[]` result can mean "no matches" or "no
/// search backend configured," indistinguishable from the response, same
/// ES-dependency story Conductor's own UI has.
class ExecutionsPage extends StatefulWidget {
  const ExecutionsPage({super.key});

  @override
  State<ExecutionsPage> createState() => _ExecutionsPageState();
}

class _ExecutionsPageState extends State<ExecutionsPage> {
  static const _statusFilterId = 'status';
  static const _statuses = [
    'RUNNING',
    'COMPLETED',
    'FAILED',
    'TIMED_OUT',
    'PAUSED',
    'TERMINATED',
  ];
  static const _pageSize = 20;

  final _api = EngineApiClient();
  List<Map<String, dynamic>> _executions = [];
  bool _loading = true;
  String? _error;
  String _search = '';
  Map<String, String> _filterValues = {};
  var _pagination = const CDataTablePaginationState(pageSize: _pageSize);
  bool _isLastPage = false;
  Set<String> _selected = {};

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final statusFilter = _filterValues[_statusFilterId];
      final rows = await _api.searchWorkflows(
        query: statusFilter == null ? '' : 'status:$statusFilter',
        freeText: _search,
        start: _pagination.pageIndex * _pagination.pageSize,
        size: _pagination.pageSize,
      );
      if (mounted) {
        setState(() {
          _executions = rows;
          _isLastPage = rows.length < _pagination.pageSize;
        });
      }
    } catch (e) {
      if (mounted) setState(() => _error = '$e');
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  Future<void> _bulkAction(
    Future<List<Map<String, dynamic>>> Function(Set<String> ids) action,
    Set<String> ids,
  ) async {
    final results = await action(ids);
    final succeeded = results.where((r) => r['success'] == true).length;
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text('$succeeded/${results.length} succeeded')),
    );
    setState(() => _selected = {});
    await _load();
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Executions'],
        actions: [
          IconButton(onPressed: _load, icon: const Icon(Icons.refresh)),
        ],
      ),
      child: _error != null
            ? Center(child: Text('Failed to load executions: $_error'))
            : Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  if (!_loading && _executions.isEmpty)
                    const Padding(
                      padding: EdgeInsets.only(bottom: 8),
                      child: CInlineTip(
                        'No results. If this engine has no search backend configured, '
                        'execution search always returns empty (Conductor has this same '
                        'dependency without Elasticsearch) — try the SQL page for a direct '
                        'query instead.',
                      ),
                    ),
                  Expanded(
                    child: CDataTable<Map<String, dynamic>>(
                      isLoading: _loading,
                      data: _executions,
                      getRowId: (row) => row['exec_id'] as String? ?? '',
                      searchValue: _search,
                      onSearchChanged: (value) {
                        setState(() {
                          _search = value;
                          _pagination = const CDataTablePaginationState(pageSize: _pageSize);
                        });
                        _load();
                      },
                      // Not `const` — a `for` collection-element (building `options` from
                      // `_statuses`) can't legally appear inside a const list literal in Dart;
                      // this list is built fresh each build instead (cheap, six entries).
                      filters: [
                        CDataTableFilter(
                          id: _statusFilterId,
                          label: 'Status',
                          options: [
                            for (final status in _statuses)
                              CDataTableFilterOption(value: status, label: status),
                          ],
                        ),
                      ],
                      filterValues: _filterValues,
                      onFilterChanged: (values) {
                        setState(() {
                          _filterValues = values;
                          _pagination = const CDataTablePaginationState(pageSize: _pageSize);
                        });
                        _load();
                      },
                      pagination: _pagination,
                      // No total-count field exists in the search response (a bare array) — a
                      // conservative estimate that always allows one more "next" page until a
                      // short page (fewer rows than pageSize) proves it's actually the end.
                      rowCount: _isLastPage
                          ? _pagination.pageIndex * _pagination.pageSize + _executions.length
                          : (_pagination.pageIndex + 2) * _pagination.pageSize,
                      onPaginationChanged: (state) {
                        setState(() => _pagination = state);
                        _load();
                      },
                      columns: [
                        CDataTableColumn(
                          id: 'exec_id',
                          header: 'Execution ID',
                          cell: (row) => CCode(row['exec_id'] as String? ?? ''),
                        ),
                        CDataTableColumn(
                          id: 'workflow_type',
                          header: 'Workflow',
                          cell: (row) => Text(row['workflow_type'] as String? ?? ''),
                        ),
                        CDataTableColumn(
                          id: 'version',
                          header: 'Version',
                          cell: (row) => Text('v${row['version'] ?? '?'}'),
                        ),
                        CDataTableColumn(
                          id: 'status',
                          header: 'Status',
                          cell: (row) => CStatusBadge(
                            label: row['status'] as String? ?? 'UNKNOWN',
                            variant: statusVariantForWorkflowStatus(row['status'] as String? ?? ''),
                          ),
                        ),
                        CDataTableColumn(
                          id: 'start_time',
                          header: 'Started',
                          cell: (row) => Text(row['start_time'] as String? ?? '—'),
                        ),
                        CDataTableColumn(
                          id: 'failed_task_names',
                          header: 'Failed tasks',
                          cell: (row) {
                            final failed =
                                (row['failed_task_names'] as List<dynamic>? ?? const []);
                            if (failed.isEmpty) return const SizedBox.shrink();
                            return CStatusBadge(
                              label: '${failed.length} failed',
                              variant: CStatusVariant.danger,
                            );
                          },
                        ),
                      ],
                      onRowClick: (row) => Navigator.of(context).push(
                        MaterialPageRoute(
                          builder: (context) =>
                              WorkflowExecutionPage(execId: row['exec_id'] as String? ?? ''),
                        ),
                      ),
                      selectedRowIds: _selected,
                      onRowSelectionChanged: (ids) => setState(() => _selected = ids),
                      commands: [
                        CDataTableCommand(
                          label: 'Pause',
                          onExecute: (ids) => _bulkAction(_api.bulkPause, ids),
                        ),
                        CDataTableCommand(
                          label: 'Resume',
                          onExecute: (ids) => _bulkAction(_api.bulkResume, ids),
                        ),
                        CDataTableCommand(
                          label: 'Retry',
                          onExecute: (ids) => _bulkAction(_api.bulkRetry, ids),
                        ),
                        CDataTableCommand(
                          label: 'Restart',
                          onExecute: (ids) => _bulkAction(_api.bulkRestart, ids),
                        ),
                        CDataTableCommand(
                          label: 'Terminate',
                          destructive: true,
                          onExecute: (ids) async {
                            if (!await showCPrompt(
                              context,
                              title: 'Terminate ${ids.length} execution(s)?',
                              description: 'This stops every selected execution.',
                              confirmLabel: 'Terminate',
                            )) {
                              return;
                            }
                            await _bulkAction(_api.bulkTerminate, ids);
                          },
                        ),
                        CDataTableCommand(
                          label: 'Remove',
                          destructive: true,
                          onExecute: (ids) async {
                            if (!await showCPrompt(
                              context,
                              title: 'Remove ${ids.length} execution(s)?',
                              description: 'This permanently deletes every selected execution.',
                              confirmLabel: 'Remove',
                            )) {
                              return;
                            }
                            await _bulkAction(_api.bulkRemove, ids);
                          },
                        ),
                      ],
                    ),
                  ),
                ],
              ),
    );
  }
}

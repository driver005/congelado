import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';
import 'event_handler_detail_page.dart';

// NOTE: see shell_scaffold.dart's top-of-file note — FBadge calls below are written from
// Forui's documented component list, not verified against the live package.

/// Root page for the engine plugin's "Event Handlers" sidebar entry — lists
/// `EventHandler`s from `GET /api/v1/event_handlers` via [CDataTable] (search
/// + row selection + a bulk "Delete" command), drills into
/// [EventHandlerDetailPage] on row click.
class EventHandlersListPage extends StatefulWidget {
  const EventHandlersListPage({super.key});

  @override
  State<EventHandlersListPage> createState() => _EventHandlersListPageState();
}

class _EventHandlersListPageState extends State<EventHandlersListPage> {
  final _api = EngineApiClient();
  List<Map<String, dynamic>> _handlers = [];
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
      final handlers = await _api.listEventHandlers();
      if (mounted) setState(() => _handlers = handlers);
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  List<Map<String, dynamic>> get _filtered {
    if (_search.isEmpty) return _handlers;
    final query = _search.toLowerCase();
    return _handlers.where((handler) {
      final name = (handler['name'] as String? ?? '').toLowerCase();
      final event = (handler['event'] as String? ?? '').toLowerCase();
      return name.contains(query) || event.contains(query);
    }).toList();
  }

  Future<void> _createNew() async {
    final changed = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (context) => const EventHandlerDetailPage(name: null)),
    );
    if (changed == true) _load();
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['Event Handlers'],
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
              id: 'event',
              header: 'Event',
              cell: (row) => CCode(row['event'] as String? ?? ''),
            ),
            CDataTableColumn(
              id: 'active',
              header: 'Active',
              cell: (row) => CStatusBadge(
                label: (row['active'] as bool? ?? false) ? 'Active' : 'Inactive',
                variant: (row['active'] as bool? ?? false)
                    ? CStatusVariant.success
                    : CStatusVariant.neutral,
              ),
            ),
            CDataTableColumn(
              id: 'actions',
              header: 'Actions',
              cell: (row) => Text(
                '${(row['actions'] as List<dynamic>? ?? const []).length}',
              ),
            ),
          ],
          onRowClick: (row) async {
            final name = row['name'] as String? ?? '';
            final changed = await Navigator.of(context).push<bool>(
              MaterialPageRoute(
                builder: (context) => EventHandlerDetailPage(name: name),
              ),
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
                  title: 'Delete ${ids.length} event handler(s)?',
                  description: 'This removes every selected event handler.',
                  confirmLabel: 'Delete',
                )) {
                  return;
                }
                for (final id in ids) {
                  await _api.deleteEventHandler(id);
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

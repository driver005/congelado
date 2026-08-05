import 'package:flutter/material.dart' show DataTable, DataColumn, DataRow, DataCell;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

import 'data_table_types.dart';

export 'data_table_types.dart';

// Verified against Forui 0.25.0's real source: FTextField's `initialValue`/`onChange` moved
// into a `control:` FTextFieldControl object; FButton takes `variant:` (FButtonVariant enum)
// directly instead of `style: FButtonStyle.outline`; FPopoverMenu's `menu:` list requires
// FItemGroupMixin entries (only FItemGroup implements that — bare FItem doesn't), so the filter
// options get wrapped in one FItemGroup; icons come from FLucideIcons, not FIcons. FTheme.of
// (context).colors is typed FColors (renamed from the FColorScheme this was guessed as). The
// grid itself is built on Material's `DataTable` (confirmed no Forui table component exists at
// all, per this component's own research) rather than a from-scratch grid implementation.
//
// Fully controlled, like Medusa's own `useDataTable` (a hook over caller-owned state, not an
// internal-state component) — every piece of state (search text, active filters, sort, page,
// row selection) lives in the CALLER, passed in + changed via callbacks. This mirrors Medusa's
// real design instead of duplicating state internally, and matches how every other page in this
// app already manages its own `setState`.

/// The flagship filterable/sortable/paginated table — Medusa UI's `DataTable`
/// block (`.../ui/src/blocks/data-table`), composed here from a toolbar
/// (search + filter menu), an active-filter chip bar, the grid itself,
/// a pagination footer, and a floating command bar shown when rows are
/// selected — the same breakdown as Medusa's own
/// `data-table-toolbar`/`data-table-filter-bar`/`data-table-table`/
/// `data-table-pagination`/`data-table-command-bar` sub-components.
///
/// Column-visibility (Medusa's `data-table-column-visibility-menu`) isn't
/// included in this first pass — flagged as a known gap, not silently
/// dropped.
class CDataTable<T> extends StatelessWidget {
  const CDataTable({
    super.key,
    required this.columns,
    required this.data,
    required this.getRowId,
    this.rowCount,
    this.isLoading = false,
    this.onRowClick,
    this.searchValue,
    this.onSearchChanged,
    this.filters = const [],
    this.filterValues = const {},
    this.onFilterChanged,
    this.sorting,
    this.onSortingChanged,
    this.pagination,
    this.onPaginationChanged,
    this.selectedRowIds,
    this.onRowSelectionChanged,
    this.commands = const [],
  });

  final List<CDataTableColumn<T>> columns;
  final List<T> data;
  final String Function(T row) getRowId;

  /// Total row count across every page, for a caller doing server-side
  /// pagination — falls back to `data.length` when null (client-side data).
  final int? rowCount;
  final bool isLoading;
  final void Function(T row)? onRowClick;

  final String? searchValue;
  final ValueChanged<String>? onSearchChanged;

  final List<CDataTableFilter> filters;

  /// filter id -> selected option value.
  final Map<String, String> filterValues;
  final ValueChanged<Map<String, String>>? onFilterChanged;

  final CDataTableSortingState? sorting;
  final ValueChanged<CDataTableSortingState>? onSortingChanged;

  final CDataTablePaginationState? pagination;
  final ValueChanged<CDataTablePaginationState>? onPaginationChanged;

  /// Selected row ids — omit (leave both this and [onRowSelectionChanged]
  /// null) to disable selection/the command bar entirely.
  final Set<String>? selectedRowIds;
  final ValueChanged<Set<String>>? onRowSelectionChanged;
  final List<CDataTableCommand> commands;

  bool get _selectable => selectedRowIds != null && onRowSelectionChanged != null;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    final selected = selectedRowIds ?? const <String>{};

    return Stack(
      children: [
        Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            if (onSearchChanged != null || filters.isNotEmpty) _buildToolbar(context),
            if (filterValues.isNotEmpty) _buildFilterBar(context),
            if (isLoading)
              const Padding(
                padding: EdgeInsets.all(24),
                child: Center(child: FCircularProgress()),
              )
            else if (data.isEmpty)
              Padding(
                padding: const EdgeInsets.all(24),
                child: Center(
                  child: Text('No results.', style: TextStyle(color: colors.mutedForeground)),
                ),
              )
            else
              _buildTable(context, selected),
            if (pagination != null) _buildPagination(context),
          ],
        ),
        // Floating command bar — Medusa's data-table-command-bar, shown only once at least one
        // row is selected.
        if (_selectable && selected.isNotEmpty && commands.isNotEmpty)
          Positioned(
            left: 0,
            right: 0,
            bottom: 8,
            child: Center(child: _buildCommandBar(context, selected)),
          ),
      ],
    );
  }

  Widget _buildToolbar(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Row(
        children: [
          if (onSearchChanged != null)
            SizedBox(
              width: 240,
              child: FTextField(
                hint: 'Search',
                control: FTextFieldControl.managed(
                  initial: searchValue == null ? null : TextEditingValue(text: searchValue!),
                  onChange: onSearchChanged == null ? null : (change) => onSearchChanged!(change.text),
                ),
              ),
            ),
          const SizedBox(width: 8),
          for (final filter in filters) _buildFilterMenu(context, filter),
        ],
      ),
    );
  }

  Widget _buildFilterMenu(BuildContext context, CDataTableFilter filter) {
    final active = filterValues[filter.id];
    return Padding(
      padding: const EdgeInsets.only(right: 8),
      child: FPopoverMenu(
        menu: [
          FItemGroup(
            children: [
              for (final option in filter.options)
                FItem(
                  title: Text(option.label),
                  onPress: () => onFilterChanged?.call({...filterValues, filter.id: option.value}),
                ),
            ],
          ),
        ],
        child: FButton(
          variant: active == null ? FButtonVariant.outline : FButtonVariant.secondary,
          onPress: () {}, // FPopoverMenu's child opens the menu on press; see its own API.
          child: Text(active == null
              ? filter.label
              : '${filter.label}: ${filter.options.firstWhere((option) => option.value == active).label}'),
        ),
      ),
    );
  }

  Widget _buildFilterBar(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Wrap(
        spacing: 6,
        children: [
          for (final entry in filterValues.entries)
            FBadge(
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text(_filterChipLabel(entry.key, entry.value)),
                  const SizedBox(width: 4),
                  GestureDetector(
                    onTap: () {
                      final next = {...filterValues}..remove(entry.key);
                      onFilterChanged?.call(next);
                    },
                    child: const Icon(FLucideIcons.x, size: 12),
                  ),
                ],
              ),
            ),
        ],
      ),
    );
  }

  String _filterChipLabel(String filterId, String value) {
    final filter = filters.firstWhere((filter) => filter.id == filterId);
    final option = filter.options.firstWhere((option) => option.value == value);
    return '${filter.label}: ${option.label}';
  }

  Widget _buildTable(BuildContext context, Set<String> selected) {
    return SingleChildScrollView(
      scrollDirection: Axis.horizontal,
      child: DataTable(
        showCheckboxColumn: _selectable,
        columns: [
          for (final column in columns)
            DataColumn(
              label: column.sortable
                  ? GestureDetector(
                      onTap: () => onSortingChanged?.call(
                        (sorting ?? const CDataTableSortingState()).toggledFor(column.id),
                      ),
                      child: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          Text(column.header),
                          if (sorting?.columnId == column.id)
                            Icon(
                              sorting!.direction == CSortDirection.ascending
                                  ? FLucideIcons.chevronUp
                                  : FLucideIcons.chevronDown,
                              size: 14,
                            ),
                        ],
                      ),
                    )
                  : Text(column.header),
            ),
        ],
        rows: [
          for (final row in data)
            DataRow(
              selected: selected.contains(getRowId(row)),
              onSelectChanged: _selectable
                  ? (value) {
                      final id = getRowId(row);
                      final next = {...selected};
                      if (value ?? false) {
                        next.add(id);
                      } else {
                        next.remove(id);
                      }
                      onRowSelectionChanged!(next);
                    }
                  : null,
              onLongPress: onRowClick == null ? null : () => onRowClick!(row),
              cells: [for (final column in columns) DataCell(column.cell(row), onTap: onRowClick == null ? null : () => onRowClick!(row))],
            ),
        ],
      ),
    );
  }

  Widget _buildPagination(BuildContext context) {
    final state = pagination!;
    final total = rowCount ?? data.length;
    final pageCount = (total / state.pageSize).ceil().clamp(1, 1 << 30);
    return Padding(
      padding: const EdgeInsets.only(top: 8),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.end,
        children: [
          Text('Page ${state.pageIndex + 1} of $pageCount'),
          const SizedBox(width: 8),
          FButton.icon(
            variant: FButtonVariant.outline,
            onPress: state.pageIndex <= 0
                ? null
                : () => onPaginationChanged?.call(
                    CDataTablePaginationState(pageIndex: state.pageIndex - 1, pageSize: state.pageSize)),
            child: const Icon(FLucideIcons.chevronLeft),
          ),
          FButton.icon(
            variant: FButtonVariant.outline,
            onPress: state.pageIndex >= pageCount - 1
                ? null
                : () => onPaginationChanged?.call(
                    CDataTablePaginationState(pageIndex: state.pageIndex + 1, pageSize: state.pageSize)),
            child: const Icon(FLucideIcons.chevronRight),
          ),
        ],
      ),
    );
  }

  Widget _buildCommandBar(BuildContext context, Set<String> selected) {
    return FCard(
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text('${selected.length} selected'),
          const SizedBox(width: 12),
          for (final command in commands)
            Padding(
              padding: const EdgeInsets.only(left: 4),
              child: FButton(
                variant: command.destructive ? FButtonVariant.destructive : FButtonVariant.secondary,
                onPress: () => command.onExecute(selected),
                child: Text(command.label),
              ),
            ),
        ],
      ),
    );
  }
}

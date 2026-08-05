import 'package:flutter/widgets.dart';

/// One column in a [CDataTable] — Medusa UI's
/// `createDataTableColumnHelper().accessor(...)`. `cell` renders the value
/// for a given row; `header` is the column's display label.
class CDataTableColumn<T> {
  const CDataTableColumn({
    required this.id,
    required this.header,
    required this.cell,
    this.sortable = false,
  });

  final String id;
  final String header;
  final Widget Function(T row) cell;
  final bool sortable;
}

/// Sort direction for a [CDataTable] column.
enum CSortDirection { ascending, descending }

/// Current sort state — Medusa UI's `DataTableSortingState`. `columnId` is
/// null when no column is actively sorted.
class CDataTableSortingState {
  const CDataTableSortingState({this.columnId, this.direction = CSortDirection.ascending});

  final String? columnId;
  final CSortDirection direction;

  CDataTableSortingState toggledFor(String columnId) {
    if (this.columnId != columnId) {
      return CDataTableSortingState(columnId: columnId, direction: CSortDirection.ascending);
    }
    return CDataTableSortingState(
      columnId: columnId,
      direction: direction == CSortDirection.ascending
          ? CSortDirection.descending
          : CSortDirection.ascending,
    );
  }
}

/// One selectable value for a [CDataTableFilter] (e.g. a status option).
class CDataTableFilterOption {
  const CDataTableFilterOption({required this.value, required this.label});

  final String value;
  final String label;
}

/// One filterable column — Medusa UI's `createDataTableFilterHelper()`.
/// Single-select for a first pass (Medusa's real filters also support
/// multi-select/date-range; not needed by anything in this codebase yet).
class CDataTableFilter {
  const CDataTableFilter({required this.id, required this.label, required this.options});

  final String id;
  final String label;
  final List<CDataTableFilterOption> options;
}

/// Current page/page-size — Medusa UI's `DataTablePaginationState`.
class CDataTablePaginationState {
  const CDataTablePaginationState({this.pageIndex = 0, this.pageSize = 20});

  final int pageIndex;
  final int pageSize;
}

/// A bulk action offered in the command bar shown when one or more rows are
/// selected — Medusa UI's `createDataTableCommandHelper()`.
class CDataTableCommand {
  const CDataTableCommand({
    required this.label,
    required this.onExecute,
    this.destructive = false,
  });

  final String label;
  final bool destructive;

  /// Receives the currently-selected row ids.
  final Future<void> Function(Set<String> selectedIds) onExecute;
}

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 table variants (table.css `.table-root--primary/secondary`).
enum HeroTableVariant {
  /// Gray wrapper (`bg-surface-secondary px-1 pb-1`) with the white table
  /// card inside (default).
  primary,

  /// No wrapper — the header is a standalone rounded strip and the body
  /// cells are transparent (`.table-root--secondary`).
  secondary,
}

/// One column of a [HeroTable].
class HeroTableColumn {
  const HeroTableColumn(this.label, {this.numeric = false, this.width});

  /// Header label — rendered `text-xs font-medium text-muted`, start-aligned.
  final String label;

  /// Right-aligns the column's header and cell content. The table.css spec
  /// itself does not define numeric alignment; this is the standard numeric
  /// convention exposed for convenience.
  final bool numeric;

  /// Fixed column width in logical pixels. Columns without a width share the
  /// remaining horizontal space equally (`w-full` table content).
  final double? width;
}

/// A HeroUI v3 table (table.css).
///
/// ```dart
/// HeroTable(
///   columns: const [
///     HeroTableColumn('Name'),
///     HeroTableColumn('Role'),
///     HeroTableColumn('Price', numeric: true),
///   ],
///   rows: [
///     [const Text('Tony Reichert'), const Text('CEO'), const Text('\$129.00')],
///     [const Text('Zoey Lang'), const Text('Technical Lead'), const Text('\$89.00')],
///   ],
///   onRowTap: (index) => print('row $index'),
/// )
/// ```
///
/// Mirror of table.css:
/// * primary variant — `bg-surface-secondary` wrapper, `px-1 pb-1` (4), radius
///   `min(32px, calc(var(--radius) * 2.5))` = 20, `overflow-clip`; the table
///   is the white card (`bg-surface`) inside, with `overflow-x-auto` scrolling;
/// * header band — `bg-surface-secondary` with a `separator/50` bottom border;
///   header cells `px-4 py-2.5`, `text-xs font-medium text-muted`, plus the
///   short `separator` vertical line (`h-4 w-px`, vertically centered) on the
///   right edge of every column except the last-of-many;
/// * body cells — `bg-surface px-4 py-3 align-middle text-sm text-foreground`
///   with a `border-b border-separator-tertiary/50`; the body's corner cells
///   are rounded `min(32px, var(--radius-2xl))` = 16;
/// * states — row hover paints the cells `bg-surface/40` (primary) /
///   `bg-default/50` (secondary); `[data-selected]` paints `bg-surface/10`
///   (wins over hover, matching source order); keyboard focus draws the 2px
///   `--focus` inset ring split across the row's cells (`rounded-lg` corners);
/// * secondary variant — transparent root and cells, header strip
///   `bg-surface-secondary` with the first/last header cell rounded 16, body
///   corners unrounded;
/// * footer — `flex items-center px-4 py-2.5`, outside the scroll container.
///
/// Rows are keyboard-focusable when [onRowTap] is provided. The focus ring is
/// the CSS inset ring (no layout shift), so it is drawn by the row's own
/// [FocusNode] (attached exactly once) instead of the external
/// `HeroFocusRing` wrapper, whose 2px outside ring cannot span a table row
/// without shifting the row layout.
///
/// table.css declares no transitions for the hover/selected states, so the
/// state changes are instant, mirroring the source exactly (no
/// `HeroMotion.durationOf` needed).
class HeroTable extends StatelessWidget {
  const HeroTable({
    super.key,
    required this.columns,
    required this.rows,
    this.variant = HeroTableVariant.primary,
    this.onRowTap,
    this.selectedRows = const <int>{},
    this.emptyMessage = 'No results',
    this.footer,
  });

  /// Column definitions — see [HeroTableColumn].
  final List<HeroTableColumn> columns;

  /// One entry per body row; each entry holds exactly [columns] cells.
  final List<List<Widget>> rows;

  /// Variant — see [HeroTableVariant].
  final HeroTableVariant variant;

  /// Invoked with the row index when a row is tapped. When null rows are not
  /// interactive (no hover cursor, no keyboard focus).
  final ValueChanged<int>? onRowTap;

  /// Row indexes rendered with the selected state (`[data-selected="true"]`:
  /// cells `bg-surface/10`).
  final Set<int> selectedRows;

  /// Shown centered in a full-width cell when [rows] is empty.
  final String emptyMessage;

  /// Optional footer strip (`flex items-center px-4 py-2.5`) below the table.
  final Widget? footer;

  bool get _primary => variant == HeroTableVariant.primary;

  @override
  Widget build(BuildContext context) {
    assert(
      rows.every((row) => row.length == columns.length),
      'Every HeroTable row must have exactly columns.length cells.',
    );
    if (columns.isEmpty) return const SizedBox.shrink();

    // .table-root--primary: bg-surface-secondary px-1 pb-1, radius
    // min(32px, calc(var(--radius) * 2.5)); --radius = 0.5rem = 8px -> 20.
    const rootRadius = 20.0;
    final padding = _primary
        ? EdgeInsets.only(
            left: HeroTokens.space1.resolve(context),
            right: HeroTokens.space1.resolve(context),
            bottom: HeroTokens.space1.resolve(context),
          )
        : EdgeInsets.zero;

    return Container(
      // .table-root: relative grid w-full overflow-clip. Secondary keeps a
      // transparent decoration (never null): clipBehavior: hardEdge requires
      // a non-null decoration.
      clipBehavior: Clip.hardEdge,
      decoration: _primary
          ? BoxDecoration(
              color: HeroTokens.colorSurfaceSecondary.resolve(context),
              borderRadius: BorderRadius.circular(rootRadius),
            )
          : BoxDecoration(
              color: HeroTokens.colorTransparent.resolve(context),
            ),
      padding: padding,
      child: LayoutBuilder(
        builder: (context, constraints) {
          final widths = _columnWidths(constraints.maxWidth);
          return Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // .table__scroll-container — overflow-x-auto.
              SingleChildScrollView(
                scrollDirection: Axis.horizontal,
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  // NOT stretch: inside a horizontal scroll view the cross
                  // axis is unbounded (w=Infinity) — stretch would force an
                  // infinite width. Children keep their fixed widths and the
                  // column sizes to the widest (the total table width).
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _HeroTableHeader(
                      columns: columns,
                      widths: widths,
                      variant: variant,
                    ),
                    if (rows.isEmpty)
                      SizedBox(
                        width: _totalWidth(widths),
                        child: _HeroTableEmpty(
                          message: emptyMessage,
                          primary: _primary,
                        ),
                      )
                    else
                      for (var i = 0; i < rows.length; i++)
                        _HeroTableRow(
                          columns: columns,
                          cells: rows[i],
                          widths: widths,
                          variant: variant,
                          isFirst: i == 0,
                          isLast: i == rows.length - 1,
                          selected: selectedRows.contains(i),
                          onTap: onRowTap == null
                              ? null
                              : () => onRowTap!(i),
                        ),
                    ],
                  ),
                ),
              if (footer != null)
                // .table__footer — flex items-center px-4 py-2.5.
                Padding(
                  padding: EdgeInsets.symmetric(
                    horizontal: HeroTokens.space4.resolve(context),
                    vertical: 10.0,
                  ),
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [footer!],
                  ),
                ),
            ],
          );
        },
      ),
    );
  }

  static double _totalWidth(List<double> widths) =>
      widths.fold(0.0, (sum, w) => sum + w);

  /// Column widths: fixed [HeroTableColumn.width]s are kept; flexible columns
  /// share the remaining viewport width. With no flexible columns the leftover
  /// space is distributed equally (auto table layout grows columns to fill
  /// `w-full`).
  List<double> _columnWidths(double viewportWidth) {
    final n = columns.length;
    var fixedSum = 0.0;
    var flexCount = 0;
    for (final column in columns) {
      if (column.width != null) {
        fixedSum += column.width!;
      } else {
        flexCount++;
      }
    }
    final remaining = viewportWidth > fixedSum ? viewportWidth - fixedSum : 0.0;
    if (flexCount > 0) {
      final share = remaining / flexCount;
      return [for (final column in columns) column.width ?? share];
    }
    final share = remaining / n;
    return [for (final column in columns) column.width! + share];
  }
}

/// Header band (`.table__header`) with the `<th>` cells (`.table__column`).
class _HeroTableHeader extends StatelessWidget {
  const _HeroTableHeader({
    required this.columns,
    required this.widths,
    required this.variant,
  });

  final List<HeroTableColumn> columns;
  final List<double> widths;
  final HeroTableVariant variant;

  @override
  Widget build(BuildContext context) {
    final primary = variant == HeroTableVariant.primary;
    return Container(
      // .table__header — border-b border-separator/50 bg-surface-secondary
      // (secondary: border-b-0 bg-transparent).
      decoration: BoxDecoration(
        color: primary
            ? HeroTokens.colorSurfaceSecondary.resolve(context)
            : HeroTokens.colorTransparent.resolve(context),
        border: primary
            ? Border(
                bottom: BorderSide(
                  color: HeroTokens.colorSeparator
                      .resolve(context)
                      .withValues(alpha: 0.5),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                ),
              )
            : null,
      ),
      child: Row(
        // start, not stretch: the header can sit in an unbounded height
        // context (widgetbook frame); stretch would demand a bounded height.
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          for (var i = 0; i < columns.length; i++)
            SizedBox(
              width: widths[i],
              child: _HeroTableColumnHeader(
                column: columns[i],
                variant: variant,
                isFirst: i == 0,
                isLast: i == columns.length - 1,
              ),
            ),
        ],
      ),
    );
  }
}

/// One `<th>` (`.table__column`) — label plus the `::after` separator line.
class _HeroTableColumnHeader extends StatelessWidget {
  const _HeroTableColumnHeader({
    required this.column,
    required this.variant,
    required this.isFirst,
    required this.isLast,
  });

  final HeroTableColumn column;
  final HeroTableVariant variant;
  final bool isFirst;
  final bool isLast;

  @override
  Widget build(BuildContext context) {
    final primary = variant == HeroTableVariant.primary;
    // .table__column:last-child:not(:only-child)::after -> content: none.
    final showSeparator = !(isLast && !(isFirst && isLast));
    final radius = BorderRadius.circular(
      HeroTokens.radius2xl.resolve(context).x,
    );
    return Stack(
      children: [
        Container(
          // Secondary: the header cells carry the strip background and the
          // first/last cell is rounded (start/end corners, 16).
          decoration: BoxDecoration(
            color: primary
                ? null
                : HeroTokens.colorSurfaceSecondary.resolve(context),
            borderRadius: !primary
                ? isFirst && isLast
                    ? radius
                    : isFirst
                        ? BorderRadius.horizontal(left: radius.topLeft)
                        : isLast
                            ? BorderRadius.horizontal(right: radius.topRight)
                            : null
                : null,
          ),
          padding: EdgeInsets.symmetric(
            horizontal: HeroTokens.space4.resolve(context),
            vertical: 10.0, // py-2.5
          ),
          alignment:
              column.numeric ? Alignment.centerRight : Alignment.centerLeft,
          child: Text(
            column.label,
            style: TextStyle(
              fontSize: HeroTokens.typeXs.resolve(context).fontSize,
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: HeroTokens.colorMuted.resolve(context),
            ),
          ),
        ),
        if (showSeparator)
          // .table__column::after — absolute end-0 top-1/2 h-4 w-px
          // -translate-y-1/2 rounded-sm bg-separator.
          Align(
            alignment: Alignment.centerRight,
            child: Container(
              width: 1.0,
              height: HeroTokens.space4.resolve(context),
              decoration: BoxDecoration(
                color: HeroTokens.colorSeparator.resolve(context),
                borderRadius: BorderRadius.circular(
                  HeroTokens.radiusSm.resolve(context).x,
                ),
              ),
            ),
          ),
      ],
    );
  }
}

/// One body row (`.table__row`) — owns its hover/focus state and renders the
/// `<td>` cells (`.table__cell`).
class _HeroTableRow extends StatefulWidget {
  const _HeroTableRow({
    required this.columns,
    required this.cells,
    required this.widths,
    required this.variant,
    required this.isFirst,
    required this.isLast,
    required this.selected,
    required this.onTap,
  });

  final List<HeroTableColumn> columns;
  final List<Widget> cells;
  final List<double> widths;
  final HeroTableVariant variant;
  final bool isFirst;
  final bool isLast;
  final bool selected;
  final VoidCallback? onTap;

  @override
  State<_HeroTableRow> createState() => _HeroTableRowState();
}

class _HeroTableRowState extends State<_HeroTableRow> {
  // The row's own focus node, attached exactly once below. Disposed with the
  // row state; the node survives widget rebuilds (hover/selection changes).
  final FocusNode _focusNode = FocusNode();
  bool _focused = false;
  bool _hovered = false;

  bool get _interactive => widget.onTap != null;

  @override
  void initState() {
    super.initState();
    _focusNode.addListener(_onFocusChange);
  }

  void _onFocusChange() {
    final focused = _focusNode.hasFocus;
    if (focused != _focused) {
      setState(() => _focused = focused);
    }
  }

  @override
  void dispose() {
    _focusNode.removeListener(_onFocusChange);
    _focusNode.dispose();
    super.dispose();
  }

  KeyEventResult _onKeyEvent(FocusNode node, KeyEvent event) {
    if (event is KeyDownEvent &&
        (event.logicalKey == LogicalKeyboardKey.enter ||
            event.logicalKey == LogicalKeyboardKey.space)) {
      widget.onTap?.call();
      return KeyEventResult.handled;
    }
    return KeyEventResult.ignored;
  }

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      // Only interactive rows advertise the pointer affordance; the hover
      // paint itself applies to every row (table.css @media (hover:hover)).
      cursor: _interactive ? SystemMouseCursors.click : SystemMouseCursors.basic,
      onEnter: (_) => setState(() => _hovered = true),
      onExit: (_) => setState(() => _hovered = false),
      child: Focus(
        focusNode: _focusNode,
        canRequestFocus: _interactive,
        onKeyEvent: _interactive ? _onKeyEvent : null,
        child: GestureDetector(
          behavior: HitTestBehavior.opaque,
          onTap: _interactive
              ? () {
                  _focusNode.requestFocus();
                  widget.onTap!();
                }
              : null,
          child: Row(
            // start, not stretch: rows can sit in an unbounded height
            // context; stretch would demand a bounded height. Cells size to
            // their content — the row height is the tallest cell's height.
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              for (var i = 0; i < widget.cells.length; i++)
                SizedBox(
                  width: widget.widths[i],
                  child: _HeroTableCell(
                    column: widget.columns[i],
                    variant: widget.variant,
                    isFirstRow: widget.isFirst,
                    isLastRow: widget.isLast,
                    isFirstColumn: i == 0,
                    isLastColumn: i == widget.cells.length - 1,
                    hovered: _hovered,
                    selected: widget.selected,
                    focused: _focused,
                    child: widget.cells[i],
                  ),
                ),
            ],
          ),
        ),
      ),
    );
  }
}

/// One `<td>` (`.table__cell`) — background/separator/radius per state plus
/// the row-focus inset ring segment.
class _HeroTableCell extends StatelessWidget {
  const _HeroTableCell({
    required this.column,
    required this.child,
    required this.variant,
    required this.isFirstRow,
    required this.isLastRow,
    required this.isFirstColumn,
    required this.isLastColumn,
    required this.hovered,
    required this.selected,
    required this.focused,
  });

  final HeroTableColumn column;
  final Widget child;
  final HeroTableVariant variant;
  final bool isFirstRow;
  final bool isLastRow;
  final bool isFirstColumn;
  final bool isLastColumn;
  final bool hovered;
  final bool selected;
  final bool focused;

  @override
  Widget build(BuildContext context) {
    final primary = variant == HeroTableVariant.primary;
    final cellRadius = _cellRadius(context, primary);
    final ringRadius = _ringRadius(context);
    // No Stack/Positioned.fill here: a row is laid out inside an unbounded
    // height context (widgetbook use-case frame, a bare Column), and a
    // stretch-positioned child would crash with "BoxConstraints forces an
    // infinite height". The focus ring is painted as the cell's border when
    // focused instead.
    return Container(
      decoration: BoxDecoration(
        color: _background(context, primary),
        borderRadius: focused ? ringRadius : cellRadius,
        border: focused
            ? _ringBorder(context)
            : Border(
                bottom: BorderSide(
                  // .table__cell border-b border-separator-tertiary/50.
                  color: HeroTokens.colorSeparatorTertiary
                      .resolve(context)
                      .withValues(alpha: 0.5),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                ),
              ),
      ),
      // .table__cell: h-full bg-surface px-4 py-3 align-middle
      // text-sm text-foreground.
      padding: EdgeInsets.symmetric(
        horizontal: HeroTokens.space4.resolve(context),
        vertical: HeroTokens.space3.resolve(context),
      ),
      child: DefaultTextStyle(
        style: TextStyle(
          fontSize: HeroTokens.typeSm.resolve(context).fontSize,
          height: HeroTokens.typeSm.resolve(context).height,
          color: HeroTokens.colorForeground.resolve(context),
        ),
        child: Align(
          alignment:
              column.numeric ? Alignment.centerRight : Alignment.centerLeft,
          child: child,
        ),
      ),
    );
  }

  Color _background(BuildContext context, bool primary) {
    final surface = HeroTokens.colorSurface.resolve(context);
    // `&[data-selected]` follows `&:hover` in the source, so the selected
    // paint wins when both apply.
    if (selected) return surface.withValues(alpha: 0.1);
    if (hovered) {
      return primary
          ? surface.withValues(alpha: 0.4)
          : HeroTokens.colorDefault.resolve(context).withValues(alpha: 0.5);
    }
    return primary
        ? surface
        : HeroTokens.colorTransparent.resolve(context);
  }

  /// Body corner rounding — first/last row, first/last column cells get
  /// `min(32px, var(--radius-2xl))` = 16; secondary resets to none.
  BorderRadius? _cellRadius(BuildContext context, bool primary) {
    if (!primary) return null;
    final radius = Radius.circular(HeroTokens.radius2xl.resolve(context).x);
    final topLeft = isFirstRow && isFirstColumn;
    final topRight = isFirstRow && isLastColumn;
    final bottomLeft = isLastRow && isFirstColumn;
    final bottomRight = isLastRow && isLastColumn;
    if (!(topLeft || topRight || bottomLeft || bottomRight)) return null;
    return BorderRadius.only(
      topLeft: topLeft ? radius : Radius.zero,
      topRight: topRight ? radius : Radius.zero,
      bottomLeft: bottomLeft ? radius : Radius.zero,
      bottomRight: bottomRight ? radius : Radius.zero,
    );
  }

  /// Focus ring corner radius — `rounded-lg` (8) on the ring's outer corners.
  BorderRadius? _ringRadius(BuildContext context) {
    final radius = HeroTokens.radiusLg.resolve(context).x;
    if (isFirstColumn && isLastColumn) {
      return BorderRadius.circular(radius);
    }
    if (isFirstColumn) return BorderRadius.horizontal(left: Radius.circular(radius));
    if (isLastColumn) return BorderRadius.horizontal(right: Radius.circular(radius));
    return null;
  }

  Border _ringBorder(BuildContext context) {
    final focus = HeroTokens.colorFocus.resolve(context);
    const w = 2.0;
    final side = BorderSide(color: focus, width: w);
    if (isFirstColumn && isLastColumn) return Border.all(color: focus, width: w);
    if (isFirstColumn) {
      return Border(left: side, top: side, bottom: side);
    }
    if (isLastColumn) {
      return Border(right: side, top: side, bottom: side);
    }
    return Border(top: side, bottom: side);
  }
}

/// Empty body — a full-width cell (`py-3 text-center`, muted) mirroring the
/// body card: white surface with a `separator-tertiary/50` bottom border and
/// rounded 16 corners (the empty body row is both the first and the last
/// body row) in the primary variant.
class _HeroTableEmpty extends StatelessWidget {
  const _HeroTableEmpty({required this.message, required this.primary});

  final String message;
  final bool primary;

  @override
  Widget build(BuildContext context) {
    final radius = BorderRadius.circular(
      HeroTokens.radius2xl.resolve(context).x,
    );
    return Container(
      decoration: BoxDecoration(
        color: primary
            ? HeroTokens.colorSurface.resolve(context)
            : HeroTokens.colorTransparent.resolve(context),
        // The empty body is a single row — it is both the first and the last
        // body row, so all four corner cells round to 16 (primary only).
        borderRadius: primary ? radius : null,
        border: Border(
          bottom: BorderSide(
            color: HeroTokens.colorSeparatorTertiary
                .resolve(context)
                .withValues(alpha: 0.5),
            width: HeroTokens.doubleBorderWidth.resolve(context),
          ),
        ),
      ),
      padding: EdgeInsets.symmetric(vertical: HeroTokens.space3.resolve(context)),
      child: Center(
        child: Text(
          message,
          style: TextStyle(
            fontSize: HeroTokens.typeSm.resolve(context).fontSize,
            color: HeroTokens.colorMuted.resolve(context),
          ),
        ),
      ),
    );
  }
}

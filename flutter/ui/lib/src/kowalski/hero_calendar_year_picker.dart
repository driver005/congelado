import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_calendar.dart';

/// HeroUI v3 calendar year picker (calendar-year-picker.css).
///
/// In the source this is the toggleable year overlay of the calendar: a
/// 3-column, scrollable grid (`.calendar-year-picker__year-grid` —
/// `grid-template-columns: repeat(3, 1fr)`, `gap-1 p-1`, 20 visible years
/// around the focused year) whose cells (`.calendar-year-picker__year-cell`)
/// are `h-8 px-2.5 rounded-3xl text-sm font-medium`; hover (not selected)
/// `bg-default text-default-foreground`, selected `bg-accent
/// text-accent-foreground` (hover `bg-accent-hover`), focus `status-focused`.
///
/// This mirror exposes the year grid standalone (embedded, non-overlay):
///
/// ```dart
/// HeroCalendarYearPicker(
///   selectedYear: 2025,
///   onYearSelected: (y) => setState(() => year = y),
/// )
/// ```
class HeroCalendarYearPicker extends StatefulWidget {
  const HeroCalendarYearPicker({
    super.key,
    this.selectedYear,
    this.initialYear,
    this.onYearSelected,
    this.enabled = true,
    this.height = heroCalendarYearPickerHeight,
  });

  /// The highlighted year (accent fill).
  final int? selectedYear;

  /// The year the grid is centered on when first built (defaults to
  /// [selectedYear] or the current year).
  final int? initialYear;

  /// Invoked when a year cell is tapped. When null the picker is disabled.
  final ValueChanged<int>? onYearSelected;

  /// Disabled state (`status-disabled`, 50% opacity).
  final bool enabled;

  /// Grid height — defaults to roughly one month-grid's worth (6 rows of
  /// 32px cells plus `gap-1` and `p-1`), the area the source overlays.
  final double height;

  @override
  State<HeroCalendarYearPicker> createState() => _HeroCalendarYearPickerState();
}

/// Default grid height: 6 rows × 32 + 5 × 4 gap + 2 × 4 padding.
const double heroCalendarYearPickerHeight = 220.0;

/// Years rendered in the grid (react-aria's default `visibleYears: 20`).
const int heroCalendarYearPickerCount = 20;

class _HeroCalendarYearPickerState extends State<HeroCalendarYearPicker> {
  late int _focusedYear;
  int? _focusedIndex;

  @override
  void initState() {
    super.initState();
    _focusedYear =
        widget.initialYear ?? widget.selectedYear ?? DateTime.now().year;
  }

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled && widget.onYearSelected != null;
    // 20 years with the focused year centered (approximation of react-aria's
    // year page).
    final firstYear = _focusedYear - (heroCalendarYearPickerCount ~/ 2 - 1);
    final years = [
      for (var y = firstYear; y < firstYear + heroCalendarYearPickerCount; y++) y,
    ];

    return Opacity(
      opacity: widget.enabled
          ? 1.0
          : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: Container(
        width: heroCalendarWidth, // matches the calendar grid area
        height: widget.height,
        padding: const EdgeInsets.all(4), // p-1
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(heroCalendarCellRadius),
        ),
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              for (var row = 0; row < (years.length + 2) ~/ 3; row++) ...[
                if (row > 0) const SizedBox(height: 4), // gap-1
                Row(
                  children: [
                    for (var col = 0; col < 3; col++) ...[
                      if (col > 0) const SizedBox(width: 4), // gap-1
                      if (row * 3 + col < years.length)
                        Expanded(
                          child: _HeroYearCell(
                            year: years[row * 3 + col],
                            selected: years[row * 3 + col] == widget.selectedYear,
                            enabled: enabled,
                            focused: _focusedIndex == row * 3 + col,
                            onTap: enabled
                                ? () {
                                    setState(() => _focusedIndex = row * 3 + col);
                                    widget.onYearSelected!(years[row * 3 + col]);
                                  }
                                : null,
                          ),
                        )
                      else
                        const Expanded(child: SizedBox()),
                    ],
                  ],
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}

/// One year cell (`.calendar-year-picker__year-cell`): `h-8 px-2.5
/// rounded-3xl text-sm font-medium`; hover (not selected) `bg-default
/// text-default-foreground`; selected `bg-accent text-accent-foreground`
/// (hover `bg-accent-hover`); focus `status-focused`.
class _HeroYearCell extends StatefulWidget {
  const _HeroYearCell({
    required this.year,
    required this.selected,
    required this.enabled,
    required this.focused,
    required this.onTap,
  });

  final int year;
  final bool selected;
  final bool enabled;
  final bool focused;
  final VoidCallback? onTap;

  @override
  State<_HeroYearCell> createState() => _HeroYearCellState();
}

class _HeroYearCellState extends State<_HeroYearCell> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled && widget.onTap != null;
    final accent = HeroTokens.colorAccent.resolve(context);
    final accentHover = HeroTokens.colorAccentHover.resolve(context);
    final accentForeground = HeroTokens.colorAccentForeground.resolve(context);
    final defaultColor = HeroTokens.colorDefault.resolve(context);
    final defaultForeground = HeroTokens.colorDefaultForeground.resolve(context);
    final foreground = HeroTokens.colorForeground.resolve(context);

    Color? background;
    Color textColor = foreground;
    if (widget.selected) {
      background = _hovered ? accentHover : accent;
      textColor = accentForeground;
    } else if (_hovered) {
      background = defaultColor;
      textColor = defaultForeground;
    }

    return MouseRegion(
      cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
      onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
      onExit: enabled ? (_) => setState(() => _hovered = false) : null,
      child: GestureDetector(
        onTap: widget.onTap,
        child: AnimatedContainer(
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: 100),
          ),
          curve: heroEaseSmooth,
          height: 32, // h-8
          padding: const EdgeInsets.symmetric(horizontal: 10), // px-2.5
          alignment: Alignment.center,
          decoration: BoxDecoration(
            color: background,
            borderRadius: BorderRadius.circular(heroCalendarCellRadius),
            border: widget.focused
                ? Border.all(color: accent, width: 2) // status-focused
                : null,
          ),
          child: Text(
            '${widget.year}',
            style: TextStyle(
              fontSize: HeroTokens.typeSm.resolve(context).fontSize,
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: textColor,
            ),
          ),
        ),
      ),
    );
  }
}

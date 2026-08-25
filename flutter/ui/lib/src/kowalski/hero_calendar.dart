library;

import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 calendar (calendar.css).
///
/// `.calendar` — `w-63 max-w-63` (252): a header with prev/next ghost nav
/// buttons and a `text-sm font-medium` month-year heading, a muted weekday
/// row (`text-xs font-medium text-muted`), then 6 weeks of 36px day cells.
/// Cells are `rounded-3xl` (24) `text-sm font-medium`; hover paints
/// `bg-default`, today `bg-accent-soft`, selected `bg-accent text-accent-
/// foreground`. The calendar itself has no padding/radius/background — that
/// chrome belongs to the picker popover.
///
/// This file also hosts the internal building blocks shared by the other
/// date components (month grid math, weekday header, nav button) — they are
/// public so `HeroRangeCalendar`, `HeroDatePicker` and friends can reuse the
/// exact same geometry/behavior.

/// `.calendar` width — `w-63 max-w-63` (252).
const double heroCalendarWidth = 252.0;

/// Day-cell size — 252 / 7 grid columns.
const double heroCalendarCellSize = 36.0;

/// Radius of a day cell / year cell — `rounded-3xl` (24).
const double heroCalendarCellRadius = 24.0;

/// Strips the time component of a [DateTime].
DateTime heroDateOnly(DateTime date) => DateTime(date.year, date.month, date.day);

/// Whether [a] and [b] fall on the same calendar day.
bool heroSameDay(DateTime? a, DateTime? b) =>
    a != null &&
    b != null &&
    a.year == b.year &&
    a.month == b.month &&
    a.day == b.day;

/// First weekday index (0 = the locale's first day of the week, Sunday for
/// en-US) — mirrors react-aria's locale-derived `firstDayOfWeek`.
int heroFirstWeekdayIndex(BuildContext context) =>
    MaterialLocalizations.of(context).firstDayOfWeekIndex;

/// Weekday labels ordered from the locale's first day of the week —
/// single letters (HeroUI storybook shows `S M T W T F S`).
List<String> heroWeekdayLabels(BuildContext context) {
  final localizations = MaterialLocalizations.of(context);
  final all = localizations.narrowWeekdays;
  final offset = heroFirstWeekdayIndex(context);
  return [for (var i = 0; i < 7; i++) all[(i + offset) % 7]];
}

/// "July 2025" style heading — `useCalendarHeading` equivalent.
String heroMonthYearLabel(BuildContext context, DateTime month) =>
    MaterialLocalizations.of(context).formatMonthYear(month);

/// The 42 days (6 weeks) of the grid containing [month], starting from the
/// week of the month's first day. Leading/trailing days belong to the
/// adjacent months (callers mark them via `day.month != month.month`).
List<DateTime> heroMonthGridDays(DateTime month, int firstWeekdayIndex) {
  final first = DateTime(month.year, month.month, 1);
  // Column of the 1st under the locale's week start (0 = the first weekday).
  // `weekday % 7` maps Sunday=0 … Saturday=6; shift so firstWeekdayIndex is 0.
  final offset = ((first.weekday % 7) - firstWeekdayIndex) % 7;
  final start = first.subtract(Duration(days: offset));
  return [for (var i = 0; i < 42; i++) start.add(Duration(days: i))];
}

/// A HeroUI v3 calendar nav button (`.calendar__nav-button` / the identical
/// `.range-calendar__nav-button`): a 24px ghost icon button, `rounded-2xl`
/// (16) in the calendar and `rounded-xl` (12) in the range calendar,
/// `text-accent-soft-foreground`, hover `bg-default`, pressed `scale(0.95)`
/// at 250ms `--ease-out`, focus `status-focused`.
class HeroCalendarNavButton extends StatefulWidget {
  const HeroCalendarNavButton({
    super.key,
    required this.icon,
    this.onPressed,
    this.enabled = true,
    this.radius = heroCalendarNavButtonRadius,
  });

  final IconData icon;
  final VoidCallback? onPressed;
  final bool enabled;

  /// Corner radius — 16 (calendar) or 12 (range calendar).
  final double radius;

  @override
  State<HeroCalendarNavButton> createState() => _HeroCalendarNavButtonState();
}

/// `.calendar__nav-button` radius — `rounded-2xl` (16).
const double heroCalendarNavButtonRadius = 16.0;

/// `.range-calendar__nav-button` radius — `rounded-xl` (12).
const double heroRangeCalendarNavButtonRadius = 12.0;

class _HeroCalendarNavButtonState extends State<HeroCalendarNavButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled && widget.onPressed != null;
    final size = HeroTokens.space6.resolve(context); // size-6 (24)
    final iconSize = HeroTokens.space4.resolve(context); // size-4 (16)
    return HeroFocusRing(
      radius: widget.radius,
      builder: (context, node, focused) => Opacity(
        opacity: enabled
            ? 1.0
            : HeroTokens.doubleDisabledOpacity.resolve(context),
        child: MouseRegion(
          cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
          onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
          onExit: enabled ? (_) => setState(() => _hovered = false) : null,
          child: Focus(
            focusNode: node,
            child: GestureDetector(
              onTap: enabled ? widget.onPressed : null,
              onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
              onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
              onTapCancel: enabled ? () => setState(() => _pressed = false) : null,
              child: AnimatedScale(
                scale: _pressed ? 0.95 : 1.0,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
                ),
                curve: heroEaseOut,
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 100),
                  ),
                  curve: heroEaseOut,
                  width: size,
                  height: size,
                  decoration: BoxDecoration(
                    color: _hovered
                        ? HeroTokens.colorDefault.resolve(context)
                        : null,
                    borderRadius: BorderRadius.circular(widget.radius),
                  ),
                  child: Icon(
                    widget.icon,
                    size: iconSize,
                    color: HeroTokens.colorAccentSoftForeground.resolve(context),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// HeroUI v3 calendar header (`.calendar__header` / `.range-calendar__header`):
/// `flex items-center justify-between px-0.5 pb-4` — prev/next ghost buttons
/// flanking a `flex-1 text-sm font-medium` month-year heading.
class HeroCalendarHeader extends StatelessWidget {
  const HeroCalendarHeader({
    super.key,
    required this.heading,
    this.onPrevious,
    this.onNext,
    this.enabled = true,
    this.navButtonRadius = heroCalendarNavButtonRadius,
  });

  final String heading;
  final VoidCallback? onPrevious;
  final VoidCallback? onNext;
  final bool enabled;
  final double navButtonRadius;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(left: 2, right: 2, bottom: 16), // px-0.5 pb-4
      child: Row(
        children: [
          HeroCalendarNavButton(
            icon: Icons.chevron_left_rounded,
            onPressed: onPrevious,
            enabled: enabled,
            radius: navButtonRadius,
          ),
          Expanded(
            child: Text(
              heading,
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
          HeroCalendarNavButton(
            icon: Icons.chevron_right_rounded,
            onPressed: onNext,
            enabled: enabled,
            radius: navButtonRadius,
          ),
        ],
      ),
    );
  }
}

/// The weekday header row (`.calendar__header-cell`): 7 equal columns of
/// `text-xs font-medium text-muted` with `pb-2` (8).
Widget heroCalendarWeekdayHeader(BuildContext context) {
  final labels = heroWeekdayLabels(context);
  return Row(
    children: [
      for (final label in labels)
        Expanded(
          child: Padding(
            padding: const EdgeInsets.only(bottom: 8), // pb-2
            child: Text(
              label,
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
        ),
    ],
  );
}

/// The 6-week day grid (`.calendar__grid`): a 7-column table of 36px cells,
/// 252px wide. [cellBuilder] receives each day, whether it belongs to the
/// adjacent month, and its 0-based index (row = index ~/ 7, col = index % 7).
class HeroCalendarMonthGrid extends StatelessWidget {
  const HeroCalendarMonthGrid({
    super.key,
    required this.month,
    required this.firstWeekdayIndex,
    required this.cellBuilder,
  });

  final DateTime month;
  final int firstWeekdayIndex;
  final Widget Function(BuildContext context, DateTime day, bool outsideMonth, int index)
      cellBuilder;

  @override
  Widget build(BuildContext context) {
    final days = heroMonthGridDays(month, firstWeekdayIndex);
    return SizedBox(
      width: heroCalendarWidth,
      child: Table(
        defaultColumnWidth: const FixedColumnWidth(heroCalendarCellSize),
        children: [
          for (var row = 0; row < 6; row++)
            TableRow(
              children: [
                for (var col = 0; col < 7; col++)
                  cellBuilder(
                    context,
                    days[row * 7 + col],
                    days[row * 7 + col].month != month.month,
                    row * 7 + col,
                  ),
              ],
            ),
        ],
      ),
    );
  }
}

/// HeroUI v3 calendar (calendar.css) — an embedded (non-overlay) single-date
/// month grid.
///
/// ```dart
/// HeroCalendar(
///   value: selectedDate,
///   onChanged: (date) => setState(() => selectedDate = date),
/// )
/// ```
class HeroCalendar extends StatefulWidget {
  const HeroCalendar({
    super.key,
    this.value,
    this.initialDate,
    this.onChanged,
    this.enabled = true,
    this.isDateDisabled,
    this.width = heroCalendarWidth,
  });

  /// The selected date (highlighted with the accent fill).
  final DateTime? value;

  /// The month shown on first build (defaults to [value] or today).
  final DateTime? initialDate;

  /// Invoked when a day cell is tapped. When null the calendar is disabled.
  final ValueChanged<DateTime>? onChanged;

  /// Disables the whole calendar (`status-disabled` at 50% opacity).
  final bool enabled;

  /// Marks individual days unavailable (struck through, non-interactive).
  final bool Function(DateTime day)? isDateDisabled;

  /// Grid width — `.calendar` `w-63 max-w-63` (252).
  final double width;

  @override
  State<HeroCalendar> createState() => _HeroCalendarState();
}

class _HeroCalendarState extends State<HeroCalendar> {
  late DateTime _visibleMonth;
  int? _focusedIndex;

  @override
  void initState() {
    super.initState();
    _visibleMonth =
        heroDateOnly(widget.initialDate ?? widget.value ?? DateTime.now());
  }

  void _previousMonth() => setState(() {
        _visibleMonth = DateTime(_visibleMonth.year, _visibleMonth.month - 1);
      });

  void _nextMonth() => setState(() {
        _visibleMonth = DateTime(_visibleMonth.year, _visibleMonth.month + 1);
      });

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled && widget.onChanged != null;
    final firstWeekdayIndex = heroFirstWeekdayIndex(context);
    return SizedBox(
      width: widget.width,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          HeroCalendarHeader(
            heading: heroMonthYearLabel(context, _visibleMonth),
            onPrevious: enabled ? _previousMonth : null,
            onNext: enabled ? _nextMonth : null,
            enabled: enabled,
          ),
          heroCalendarWeekdayHeader(context),
          const SizedBox(height: 4), // .calendar__grid-body tr:first-child td mt-1
          HeroCalendarMonthGrid(
            month: _visibleMonth,
            firstWeekdayIndex: firstWeekdayIndex,
            cellBuilder: (context, day, outsideMonth, index) {
              final selected = heroSameDay(day, widget.value);
              final today = heroSameDay(day, DateTime.now());
              final disabled =
                  !enabled || (widget.isDateDisabled?.call(day) ?? false);
              return _HeroCalendarDayCell(
                day: day,
                selected: selected,
                today: today,
                outsideMonth: outsideMonth,
                disabled: disabled,
                focused: _focusedIndex == index,
                onTap: disabled
                    ? null
                    : () {
                        setState(() => _focusedIndex = index);
                        if (outsideMonth) {
                          _visibleMonth = DateTime(day.year, day.month);
                        }
                        widget.onChanged?.call(day);
                      },
              );
            },
          ),
        ],
      ),
    );
  }
}

/// A single day cell (`.calendar__cell`): 36×36 `rounded-3xl`, `text-sm
/// font-medium`. Selected → `bg-accent text-accent-foreground`; today →
/// `bg-accent-soft text-accent-soft-foreground` (hover
/// `bg-accent-soft-hover`); hover (not selected) → `bg-default`; pressed →
/// `bg-default scale(0.95)` (selected → `bg-accent-hover`); outside month →
/// `text-muted opacity-50` (selected → `bg-default`); disabled →
/// `status-disabled` + line-through.
class _HeroCalendarDayCell extends StatefulWidget {
  const _HeroCalendarDayCell({
    required this.day,
    required this.selected,
    required this.today,
    required this.outsideMonth,
    required this.disabled,
    required this.focused,
    required this.onTap,
  });

  final DateTime day;
  final bool selected;
  final bool today;
  final bool outsideMonth;
  final bool disabled;
  final bool focused;
  final VoidCallback? onTap;

  @override
  State<_HeroCalendarDayCell> createState() => _HeroCalendarDayCellState();
}

class _HeroCalendarDayCellState extends State<_HeroCalendarDayCell> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onTap != null;
    final accent = HeroTokens.colorAccent.resolve(context);
    final accentForeground = HeroTokens.colorAccentForeground.resolve(context);
    final accentSoft = HeroTokens.colorAccentSoft.resolve(context);
    final accentSoftHover = HeroTokens.colorAccentSoftHover.resolve(context);
    final accentSoftForeground =
        HeroTokens.colorAccentSoftForeground.resolve(context);
    final defaultColor = HeroTokens.colorDefault.resolve(context);
    final accentHover = HeroTokens.colorAccentHover.resolve(context);
    final muted = HeroTokens.colorMuted.resolve(context);
    final foreground = HeroTokens.colorForeground.resolve(context);

    // Cell background per state priority: selected > today > hover/pressed.
    // Selected outside-month cells fall back to `bg-default` (calendar.css
    // `&[data-selected="true"][data-outside-month="true"]`).
    Color? background;
    Color textColor = foreground;
    if (widget.selected) {
      background =
          _pressed ? accentHover : (widget.outsideMonth ? defaultColor : accent);
      textColor = accentForeground;
    } else if (widget.today) {
      background = _pressed ? defaultColor : (_hovered ? accentSoftHover : accentSoft);
      textColor = accentSoftForeground;
    } else if (_hovered || _pressed) {
      background = defaultColor;
    }
    if (widget.outsideMonth && !widget.selected) {
      textColor = muted;
    }

    return Opacity(
      // Outside-month days render at 50% (`opacity-50`); disabled whole-
      // calendar days use `status-disabled` (--disabled-opacity 0.5).
      opacity: widget.outsideMonth || widget.disabled
          ? HeroTokens.doubleDisabledOpacity.resolve(context)
          : 1.0,
      child: MouseRegion(
        cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
        onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
        onExit: enabled ? (_) => setState(() => _hovered = false) : null,
        child: GestureDetector(
          onTap: widget.onTap,
          onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
          onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
          onTapCancel: enabled ? () => setState(() => _pressed = false) : null,
          child: AnimatedScale(
            // transform 250ms --ease-out; pressed scale(0.95).
            scale: _pressed ? 0.95 : 1.0,
            duration: HeroMotion.durationOf(
              context,
              const Duration(milliseconds: 250),
            ),
            curve: heroEaseOut,
            child: AnimatedContainer(
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 100),
              ),
              curve: heroEaseOut,
              width: heroCalendarCellSize,
              height: heroCalendarCellSize,
              alignment: Alignment.center,
              decoration: BoxDecoration(
                color: background,
                borderRadius: BorderRadius.circular(heroCalendarCellRadius),
                border: widget.focused
                    ? Border.all(color: accent, width: 2) // status-focused
                    : null,
              ),
              child: Text(
                '${widget.day.day}',
                style: TextStyle(
                  fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                  fontWeight: HeroTokens.weightMedium.resolve(context),
                  color: textColor,
                  decoration: widget.disabled
                      ? TextDecoration.lineThrough
                      : TextDecoration.none,
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

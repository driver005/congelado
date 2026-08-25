import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_calendar.dart';

/// A contiguous date range — `(start, end)`; an in-progress selection has a
/// non-null [DateTime] start and a null end.
typedef HeroDateRange = (DateTime?, DateTime?);

/// HeroUI v3 range calendar (range-calendar.css) — an embedded (non-overlay)
/// range-selection month grid.
///
/// Shares the calendar chrome (header/nav/weekday row — see hero_calendar.dart)
/// but renders a continuous range track: cells inside the range get
/// `bg-accent-soft rounded-none`, the selection caps (`.range-calendar__cell-
/// button`) get `bg-accent text-accent-foreground`, and track corners round
/// at week boundaries (`rounded-ss-lg/es-lg`, `rounded-se-lg/ee-lg`). Tapping
/// starts the range, tapping again (after the start) completes it; tapping
/// before the current start restarts the range.
///
/// ```dart
/// HeroRangeCalendar(
///   value: (start, end),
///   onChanged: (range) => setState(() => selected = range),
/// )
/// ```
class HeroRangeCalendar extends StatefulWidget {
  const HeroRangeCalendar({
    super.key,
    this.value,
    this.initialDate,
    this.onChanged,
    this.enabled = true,
    this.isDateDisabled,
    this.width = heroCalendarWidth,
  });

  /// The selected range — `(start, end)`.
  final HeroDateRange? value;

  /// The month shown on first build (defaults to the range start or today).
  final DateTime? initialDate;

  /// Invoked with the new range on every tap. When null the calendar is
  /// disabled.
  final ValueChanged<HeroDateRange>? onChanged;

  /// Disables the whole calendar (`status-disabled` at 50% opacity).
  final bool enabled;

  /// Marks individual days unavailable (struck through, non-interactive).
  final bool Function(DateTime day)? isDateDisabled;

  /// Grid width — `.range-calendar` `w-63 max-w-63` (252).
  final double width;

  @override
  State<HeroRangeCalendar> createState() => _HeroRangeCalendarState();
}

class _HeroRangeCalendarState extends State<HeroRangeCalendar> {
  late DateTime _visibleMonth;
  int? _focusedIndex;
  DateTime? _hoveredDay;

  @override
  void initState() {
    super.initState();
    final (start, _) = widget.value ?? (null, null);
    _visibleMonth = heroDateOnly(
      widget.initialDate ?? start ?? DateTime.now(),
    );
  }

  void _previousMonth() => setState(() {
        _visibleMonth = DateTime(_visibleMonth.year, _visibleMonth.month - 1);
      });

  void _nextMonth() => setState(() {
        _visibleMonth = DateTime(_visibleMonth.year, _visibleMonth.month + 1);
      });

  void _handleTap(DateTime day) {
    final disabled = widget.isDateDisabled?.call(day) ?? false;
    if (disabled) return;
    var (start, end) = widget.value ?? (null, null);
    if (start == null || end != null) {
      start = day;
      end = null;
    } else if (day.isBefore(start)) {
      start = day;
    } else {
      end = day;
    }
    widget.onChanged?.call((start, end));
  }

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled && widget.onChanged != null;
    final firstWeekdayIndex = heroFirstWeekdayIndex(context);
    final (start, end) = widget.value ?? (null, null);
    // Preview track: with only a start chosen, hovering extends the track to
    // the hovered day (react-aria's selection preview).
    final previewEnd = end == null &&
            start != null &&
            _hoveredDay != null &&
            !_hoveredDay!.isBefore(start)
        ? _hoveredDay!
        : null;

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
            navButtonRadius: heroRangeCalendarNavButtonRadius,
          ),
          heroCalendarWeekdayHeader(context),
          const SizedBox(height: 4), // .range-calendar__grid-body tr:first-child td mt-1
          HeroCalendarMonthGrid(
            month: _visibleMonth,
            firstWeekdayIndex: firstWeekdayIndex,
            cellBuilder: (context, day, outsideMonth, index) {
              final disabled =
                  !enabled || (widget.isDateDisabled?.call(day) ?? false);
              return _HeroRangeCalendarDayCell(
                day: day,
                start: start,
                end: end,
                previewEnd: previewEnd,
                outsideMonth: outsideMonth,
                disabled: disabled,
                focused: _focusedIndex == index,
                onTap: disabled ? null : () => _handleTap(day),
                onHoverChanged: (hovered) {
                  if (enabled && hovered != _hoveredDay) {
                    setState(() => _hoveredDay = hovered);
                  }
                },
              );
            },
          ),
        ],
      ),
    );
  }
}

/// A range-calendar day cell: the td-style track container (`.range-calendar
/// __cell`, `my-[2px]`) plus the inner circle button (`.range-calendar__cell-
/// button`). The track fills the cell with `bg-accent-soft` (or `bg-default
/// /20` for outside-month segments); the button circle gets the accent fill on
/// the selection caps, `bg-accent-soft` when today, `bg-default` on hover,
/// and scales to 0.9 when pressed.
class _HeroRangeCalendarDayCell extends StatefulWidget {
  const _HeroRangeCalendarDayCell({
    required this.day,
    required this.start,
    required this.end,
    required this.previewEnd,
    required this.outsideMonth,
    required this.disabled,
    required this.focused,
    required this.onTap,
    required this.onHoverChanged,
  });

  final DateTime day;
  final DateTime? start;
  final DateTime? end;
  final DateTime? previewEnd;
  final bool outsideMonth;
  final bool disabled;
  final bool focused;
  final VoidCallback? onTap;
  final ValueChanged<DateTime?> onHoverChanged;

  @override
  State<_HeroRangeCalendarDayCell> createState() =>
      _HeroRangeCalendarDayCellState();
}

class _HeroRangeCalendarDayCellState extends State<_HeroRangeCalendarDayCell> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final day = widget.day;
    final start = widget.start;
    final end = widget.end;
    final previewEnd = widget.previewEnd;
    final enabled = !widget.disabled && widget.onTap != null;

    final isStart = heroSameDay(day, start);
    final isEnd = heroSameDay(day, end);
    final inRange = start != null &&
        end != null &&
        !day.isBefore(start) &&
        !day.isAfter(end);
    final inPreview = start != null &&
        previewEnd != null &&
        !day.isBefore(start) &&
        !day.isAfter(previewEnd) &&
        !isStart;
    final onTrack = (inRange || inPreview) && !isStart && !isEnd;

    final accent = HeroTokens.colorAccent.resolve(context);
    final accentForeground = HeroTokens.colorAccentForeground.resolve(context);
    final accentHover = HeroTokens.colorAccentHover.resolve(context);
    final accentSoft = HeroTokens.colorAccentSoft.resolve(context);
    final accentSoftHover = HeroTokens.colorAccentSoftHover.resolve(context);
    final accentSoftForeground =
        HeroTokens.colorAccentSoftForeground.resolve(context);
    final defaultColor = HeroTokens.colorDefault.resolve(context);
    final muted = HeroTokens.colorMuted.resolve(context);
    final foreground = HeroTokens.colorForeground.resolve(context);

    // Track (td) fill + corner rounding at week boundaries.
    Color? trackColor;
    BorderRadius trackRadius = BorderRadius.zero;
    if (onTrack) {
      trackColor = widget.outsideMonth
          ? defaultColor.withValues(alpha: 0.2) // bg-default/20
          : accentSoft;
      if (isStart || (onTrack && _colOf(day) == 0)) {
        trackRadius = BorderRadius.only(
          topLeft: const Radius.circular(8), // rounded-ss-lg (8)
          bottomLeft: const Radius.circular(8), // rounded-es-lg
        );
      }
      if (isEnd || (onTrack && _colOf(day) == 6)) {
        trackRadius = BorderRadius.only(
          topRight: const Radius.circular(8), // rounded-se-lg
          bottomRight: const Radius.circular(8), // rounded-ee-lg
        );
      }
    }
    if (isStart && !widget.outsideMonth) {
      trackRadius = BorderRadius.only(
        topLeft: const Radius.circular(heroCalendarCellRadius),
        bottomLeft: const Radius.circular(heroCalendarCellRadius),
        topRight: trackRadius.topRight,
        bottomRight: trackRadius.bottomRight,
      );
    }
    if (isEnd && !widget.outsideMonth) {
      trackRadius = BorderRadius.only(
        topRight: const Radius.circular(heroCalendarCellRadius),
        bottomRight: const Radius.circular(heroCalendarCellRadius),
        topLeft: trackRadius.topLeft,
        bottomLeft: trackRadius.bottomLeft,
      );
    }

    // Inner button (circle) colors.
    Color? buttonBackground;
    Color textColor = foreground;
    if ((isStart || isEnd) && !widget.outsideMonth) {
      // Selection caps — `.range-calendar__cell-button` bg-accent. The CSS
      // guards caps with `:not([data-outside-month])`: outside-month caps stay
      // transparent (muted at 50% via the wrapper opacity).
      buttonBackground = _pressed ? accentHover : accent;
      textColor = accentForeground;
    } else if (widget.outsideMonth) {
      textColor = muted;
    } else if (heroSameDay(day, DateTime.now())) {
      buttonBackground =
          _pressed ? defaultColor : (_hovered ? accentSoftHover : accentSoft);
      textColor = accentSoftForeground;
    } else if (_hovered || _pressed) {
      buttonBackground = defaultColor;
    }

    final button = AnimatedScale(
      // .range-calendar__cell-button — scale 200ms --ease-out; pressed 0.9.
      scale: _pressed ? 0.9 : 1.0,
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 200),
      ),
      curve: heroEaseOut,
      child: Container(
        width: heroCalendarCellSize,
        height: heroCalendarCellSize,
        alignment: Alignment.center,
        decoration: BoxDecoration(
          color: buttonBackground,
          borderRadius: BorderRadius.circular(heroCalendarCellRadius),
        ),
        child: Text(
          '${day.day}',
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
    );

    // Cell = 2px top margin + 36px track/button + 2px bottom margin
    // (`.range-calendar__cell { my-[2px] }`). The track container carries the
    // range fill + week-boundary corner rounding; the button scales on press.
    final cell = SizedBox(
      width: heroCalendarCellSize,
      height: heroCalendarCellSize + 4,
      child: Column(
        children: [
          const SizedBox(height: 2),
          Expanded(
            child: AnimatedContainer(
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 100),
              ),
              curve: heroEaseOut,
              decoration: BoxDecoration(
                color: trackColor,
                borderRadius: trackRadius,
                border: widget.focused
                    ? Border.all(color: accent, width: 2) // status-focused
                    : null,
              ),
              child: button,
            ),
          ),
          const SizedBox(height: 2),
        ],
      ),
    );

    return Opacity(
      opacity: widget.outsideMonth || widget.disabled
          ? HeroTokens.doubleDisabledOpacity.resolve(context)
          : 1.0,
      child: MouseRegion(
        cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
        onEnter: enabled
            ? (_) {
                setState(() => _hovered = true);
                widget.onHoverChanged(day);
              }
            : null,
        onExit: enabled
            ? (_) {
                setState(() => _hovered = false);
                widget.onHoverChanged(null);
              }
            : null,
        child: GestureDetector(
          onTap: widget.onTap,
          onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
          onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
          onTapCancel: enabled ? () => setState(() => _pressed = false) : null,
          child: cell,
        ),
      ),
    );
  }

  /// Grid column (0-based) of [day] for the locale's first weekday.
  int _colOf(DateTime day) {
    final firstWeekdayIndex = heroFirstWeekdayIndex(context);
    // `weekday % 7` maps Sunday=0 … Saturday=6; shift so the first weekday
    // of the locale lands in column 0.
    return ((day.weekday % 7) - firstWeekdayIndex) % 7;
  }
}

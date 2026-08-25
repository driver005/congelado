import 'dart:async';

import 'package:flutter/material.dart';

import 'hero_anchored_overlay.dart';
import 'hero_calendar.dart';
import 'hero_date_input_group.dart';

/// HeroUI v3 date picker (date-picker.css) — a date input group whose
/// trailing trigger opens the calendar popover.
///
/// `.date-picker` — `inline-flex flex-col gap-1`; the trigger (the whole
/// field, `.date-picker__trigger` `rounded-field p-1`) toggles the popover
/// (`.date-picker__popover`: `bg-overlay p-3`, radius
/// `min(32px, calc(var(--radius) * 2.5))` = 20, `shadow-overlay`, entering
/// `fade-in zoom-in-95` at 150ms `--ease-smooth`). The popover is ANCHORED
/// below the trigger (a popover, not a modal); the calendar grid is from
/// calendar.css.
///
/// ```dart
/// HeroDatePicker(
///   value: date,
///   onChanged: (d) => setState(() => date = d),
/// )
/// ```
class HeroDatePicker extends StatefulWidget {
  const HeroDatePicker({
    super.key,
    this.value,
    this.onChanged,
    this.initialDate,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.fullWidth = false,
  });

  /// The selected date (shown as segments in the trigger).
  final DateTime? value;

  /// Invoked when a day is picked from the overlay.
  final ValueChanged<DateTime>? onChanged;

  /// The month shown when the overlay opens (defaults to [value] or today).
  final DateTime? initialDate;

  /// Optional field label.
  final String? label;

  /// Optional helper text.
  final String? helperText;

  /// Invalid state (`status-invalid-field`).
  final bool error;

  /// Disabled state (`status-disabled`, 50% opacity).
  final bool enabled;

  /// `date-picker--full-width` (`w-full`).
  final bool fullWidth;

  @override
  State<HeroDatePicker> createState() => _HeroDatePickerState();
}

class _HeroDatePickerState extends State<HeroDatePicker> {
  final GlobalKey _fieldKey = GlobalKey();

  @override
  Widget build(BuildContext context) {
    return HeroDateInputGroup(
      value: widget.value,
      label: widget.label,
      helperText: widget.helperText,
      error: widget.error,
      enabled: widget.enabled,
      fullWidth: widget.fullWidth,
      fieldKey: _fieldKey,
      onTap: widget.enabled
          ? () async {
              final picked = await showHeroDatePicker(
                context,
                fieldKey: _fieldKey,
                initialDate: widget.initialDate,
                value: widget.value,
              );
              if (picked != null) widget.onChanged?.call(picked);
            }
          : null,
    );
  }
}

/// Shows a HeroUI v3 date-picker popover (date-picker.css
/// `.date-picker__popover`) anchored below the trigger field, and resolves
/// with the picked day (or null when dismissed).
///
/// Popover chrome — `bg-overlay`, `p-3` (12), radius 20
/// (`min(32px, calc(var(--radius) * 2.5))` with `--radius: 8`), per-theme
/// `shadow-overlay`. Anchored via [showHeroAnchoredOverlay] (below the field,
/// clamped on-screen, dismissed by outside tap or picking a day) — a popover,
/// not a modal dialog.
Future<DateTime?> showHeroDatePicker(
  BuildContext context, {
  required GlobalKey fieldKey,
  DateTime? initialDate,
  DateTime? value,
  ValueChanged<DateTime>? onSelected,
}) {
  final overlay = Overlay.maybeOf(context);
  final targetBox = fieldKey.currentContext?.findRenderObject() as RenderBox?;
  final overlayBox = overlay?.context.findRenderObject() as RenderBox?;
  if (overlay == null || targetBox == null || overlayBox == null) {
    return Future.value(null);
  }
  final origin = overlayBox.globalToLocal(
    targetBox.localToGlobal(Offset.zero),
  );
  final anchorRect = origin & targetBox.size;

  final completer = Completer<DateTime?>();
  late HeroAnchoredOverlayClose close;

  final scopeColors = HeroPopoverColors.resolve(context);

  close = showHeroAnchoredOverlay(
    context,
    anchorRect: anchorRect,
    gap: 4,
    onClosed: () {
      if (!completer.isCompleted) completer.complete(null);
    },
    builder: (entryContext) => HeroPopoverPanel(
      colors: scopeColors,
      child: HeroCalendar(
        value: value,
        initialDate: initialDate,
        onChanged: (day) {
          onSelected?.call(day);
          // Complete with the picked day BEFORE close() — close() fires
          // onClosed synchronously, which completes the completer with
          // null first (guarded by isCompleted), silently discarding this
          // completion if the order were reversed.
          if (!completer.isCompleted) completer.complete(day);
          close();
        },
      ),
    ),
  );

  // If the overlay entry was not inserted (no overlay), still complete.
  return completer.future;
}

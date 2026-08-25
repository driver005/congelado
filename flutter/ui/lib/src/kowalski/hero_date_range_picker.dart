import 'dart:async';

import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_anchored_overlay.dart';
import 'hero_date_input_group.dart';
import 'hero_focus_ring.dart';
import 'hero_range_calendar.dart';

/// HeroUI v3 date range picker (date-range-picker.css) — a date input group
/// with start/end segments separated by the `-` literal
/// (`.date-range-picker__range-separator` `px-1 text-field-placeholder`),
/// whose trailing trigger opens the range-calendar popover
/// (`.date-range-picker__popover`, identical chrome to the date picker's).
///
/// ```dart
/// HeroDateRangePicker(
///   value: (start, end),
///   onChanged: (r) => setState(() => range = r),
/// )
/// ```
class HeroDateRangePicker extends StatefulWidget {
  const HeroDateRangePicker({
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

  /// The selected range — `(start, end)`; an in-progress selection has a
  /// null end.
  final HeroDateRange? value;

  /// Invoked when the range changes (start picked, or range completed).
  final ValueChanged<HeroDateRange>? onChanged;

  /// The month shown when the overlay opens (defaults to the range start or
  /// today).
  final DateTime? initialDate;

  /// Optional field label.
  final String? label;

  /// Optional helper text.
  final String? helperText;

  /// Invalid state (`status-invalid-field`).
  final bool error;

  /// Disabled state (`status-disabled`, 50% opacity).
  final bool enabled;

  /// Full-width modifier (`w-full`).
  final bool fullWidth;

  @override
  State<HeroDateRangePicker> createState() => _HeroDateRangePickerState();
}

class _HeroDateRangePickerState extends State<HeroDateRangePicker> {
  final GlobalKey _fieldKey = GlobalKey();

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (widget.label != null)
          Padding(
            padding: const EdgeInsets.only(bottom: 4),
            child: Text(
              widget.label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        HeroFocusRing(
          radius: HeroTokens.radiusField.resolve(context).x,
          builder: (context, node, focused) =>
              _HeroDateRangePickerTrigger(
                value: widget.value,
                error: widget.error,
                enabled: widget.enabled,
                fullWidth: widget.fullWidth,
                fieldKey: _fieldKey,
                onTap: widget.enabled
                    ? () async {
                        final picked = await showHeroDateRangePicker(
                          context,
                          fieldKey: _fieldKey,
                          initialRange: widget.value,
                          initialDate: widget.initialDate,
                        );
                        if (picked != null) widget.onChanged?.call(picked);
                      }
                    : null,
              ),
        ),
        if (widget.helperText != null)
          Padding(
            padding: const EdgeInsets.only(top: 4),
            child: Text(
              widget.helperText!,
              style: TextStyle(
                fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                color: widget.error
                    ? HeroTokens.colorDanger.resolve(context)
                    : HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
      ],
    );
  }
}

/// The `.date-range-picker__trigger` field box — the date-input-group chrome
/// (`h-9 rounded-field bg-field shadow-field`) with the calendar prefix,
/// start segments, the `-` range separator and the end segments, then the
/// chevron trigger indicator.
class _HeroDateRangePickerTrigger extends StatefulWidget {
  const _HeroDateRangePickerTrigger({
    required this.value,
    required this.error,
    required this.enabled,
    required this.fullWidth,
    required this.onTap,
    this.fieldKey,
  });

  final HeroDateRange? value;
  final bool error;
  final bool enabled;
  final bool fullWidth;
  final VoidCallback? onTap;
  final Key? fieldKey;

  @override
  State<_HeroDateRangePickerTrigger> createState() =>
      _HeroDateRangePickerTriggerState();
}

class _HeroDateRangePickerTriggerState extends State<_HeroDateRangePickerTrigger> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled;
    final (start, end) = widget.value ?? (null, null);
    final field = HeroTokens.colorField.resolve(context);
    final fieldHover = HeroTokens.colorFieldHover.resolve(context);
    final fieldFocus = HeroTokens.colorFieldFocus.resolve(context);
    final placeholder = HeroTokens.colorFieldPlaceholder.resolve(context);

    Color background = widget.error ? fieldFocus : (enabled && _hovered ? fieldHover : field);
    var style = BoxDecoration(
      color: background,
      borderRadius: BorderRadius.circular(HeroTokens.radiusField.resolve(context).x),
      border: widget.error
          ? Border.all(
              color: HeroTokens.colorDanger.resolve(context),
              width: HeroTokens.doubleBorderWidth.resolve(context),
            )
          : enabled && _hovered
              ? Border.all(
                  color: HeroTokens.colorFieldBorderHover.resolve(context),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                )
              : null,
    );
    final shadows = HeroTokens.shadowField.resolve(context);
    if (shadows.isNotEmpty) style = style.copyWith(boxShadow: shadows);

    return Opacity(
      opacity: enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: MouseRegion(
        cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
        onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
        onExit: enabled ? (_) => setState(() => _hovered = false) : null,
        child: GestureDetector(
          key: widget.fieldKey,
          onTap: widget.onTap,
          child: AnimatedContainer(
            duration: HeroMotion.durationOf(
              context,
              const Duration(milliseconds: 150),
            ),
            curve: heroEaseSmooth,
            width: widget.fullWidth ? double.infinity : null,
            height: HeroTokens.doubleInputMinHeight.resolve(context), // h-9 (36)
            decoration: style,
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                const SizedBox(width: 12), // ms-3
                Icon(Icons.calendar_month_rounded, size: 16, color: placeholder),
                Padding(
                  padding: EdgeInsets.symmetric(
                    horizontal: HeroTokens.doubleInputPaddingX.resolve(context),
                    vertical: HeroTokens.doubleInputPaddingY.resolve(context),
                  ),
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      HeroDateSegments(value: start, enabled: enabled, error: widget.error),
                      Padding(
                        padding: const EdgeInsets.symmetric(horizontal: 4), // px-1
                        child: Text(
                          ' - ',
                          style: TextStyle(
                            fontSize: HeroTokens.doubleInputFontSize.resolve(context),
                            color: placeholder,
                          ),
                        ),
                      ),
                      HeroDateSegments(value: end, enabled: enabled, error: widget.error),
                    ],
                  ),
                ),
                Icon(Icons.keyboard_arrow_down_rounded, size: 18, color: placeholder),
                const SizedBox(width: 12), // me-3
              ],
            ),
          ),
        ),
      ),
    );
  }
}

/// Shows a HeroUI v3 date-range-picker popover (`date-range-picker.css`
/// `.date-range-picker__popover`) anchored below the trigger field, hosting
/// a [HeroRangeCalendar], and resolves with the completed range (or null
/// when dismissed / still in progress).
///
/// The popover chrome mirrors the date picker's: `bg-overlay p-3`, radius 20
/// (`min(32px, calc(var(--radius) * 2.5))`), per-theme `shadow-overlay` —
/// anchored via [showHeroAnchoredOverlay] (below the field, clamped
/// on-screen, dismissed by outside tap or completing the range). A popover,
/// not a modal dialog.
Future<HeroDateRange?> showHeroDateRangePicker(
  BuildContext context, {
  required GlobalKey fieldKey,
  HeroDateRange? initialRange,
  DateTime? initialDate,
  ValueChanged<HeroDateRange>? onSelected,
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

  final completer = Completer<HeroDateRange?>();
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
      child: _HeroDateRangePickerDialog(
        initialRange: initialRange,
        initialDate: initialDate,
        onChanged: (range) {
          onSelected?.call(range);
          final (start, end) = range;
          // Close once the range completes (both ends picked). Complete
          // BEFORE close() — close() fires onClosed synchronously, which
          // completes the completer with null first (guarded by
          // isCompleted), silently discarding this completion otherwise.
          if (start != null && end != null) {
            if (!completer.isCompleted) completer.complete(range);
            close();
          }
        },
      ),
    ),
  );

  return completer.future;
}

/// The dialog's calendar host — [HeroRangeCalendar] is fully controlled
/// (it reads `value` on every tap and keeps no internal selection state), so
/// the in-progress range MUST round-trip through state. Without this wrapper
/// the dialog builder would keep handing the calendar the original
/// [initialRange] and a second tap could never complete the range.
class _HeroDateRangePickerDialog extends StatefulWidget {
  const _HeroDateRangePickerDialog({
    required this.initialRange,
    required this.initialDate,
    required this.onChanged,
  });

  final HeroDateRange? initialRange;
  final DateTime? initialDate;
  final ValueChanged<HeroDateRange> onChanged;

  @override
  State<_HeroDateRangePickerDialog> createState() =>
      _HeroDateRangePickerDialogState();
}

class _HeroDateRangePickerDialogState extends State<_HeroDateRangePickerDialog> {
  late HeroDateRange _range = widget.initialRange ?? (null, null);

  @override
  Widget build(BuildContext context) {
    return HeroRangeCalendar(
      value: _range,
      initialDate: widget.initialDate,
      onChanged: (range) {
        setState(() => _range = range);
        widget.onChanged(range);
      },
    );
  }
}

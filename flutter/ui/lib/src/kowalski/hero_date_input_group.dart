import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 date input group (date-input-group.css).
///
/// `.date-input-group` — `inline-flex h-9 items-center overflow-hidden
/// rounded-field border bg-field text-sm text-field-foreground shadow-field`;
/// hover `bg-field-hover`, focus-within `status-focused-field` (2px accent
/// ring), invalid `status-invalid-field`, disabled `status-disabled`. The
/// `__input` slot (the segment row) is `flex-1 px-3 py-2`, the `__prefix` /
/// `__suffix` icons sit at the edges in `text-field-placeholder`.
///
/// ```dart
/// HeroDateInputGroup(
///   value: date,
///   label: 'Date',
///   onClear: () => setState(() => date = null),
/// )
/// ```
class HeroDateInputGroup extends StatefulWidget {
  const HeroDateInputGroup({
    super.key,
    this.value,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.variant = HeroDateInputGroupVariant.primary,
    this.prefix,
    this.suffix,
    this.onClear,
    this.onTap,
    this.fieldKey,
    this.fullWidth = false,
  });

  /// The displayed date; null renders placeholder segments.
  final DateTime? value;

  /// Optional field label (`text-sm font-medium`).
  final String? label;

  /// Helper text (`text-xs` muted, danger while [error]).
  final String? helperText;

  /// Invalid state (`status-invalid-field`).
  final bool error;

  /// Disabled state (`status-disabled`, 50% opacity, not interactive).
  final bool enabled;

  /// Fill class — primary (`bg-field` + `shadow-field`) or secondary
  /// (`bg-default`, no shadow).
  final HeroDateInputGroupVariant variant;

  /// Leading slot (`date-input-group__prefix`) — defaults to a calendar icon.
  final Widget? prefix;

  /// Trailing slot (`date-input-group__suffix`) — defaults to a clear icon
  /// when [onClear] is set and [value] is non-null, otherwise a chevron.
  final Widget? suffix;

  /// Shows a clear icon in the trailing slot; tapping it invokes this
  /// callback (the trailing "clear" action).
  final VoidCallback? onClear;

  /// Tap target — e.g. a picker opens the calendar overlay here.
  final VoidCallback? onTap;

  /// Attaches to the tap target (the field box) so callers can measure its
  /// rect for anchored overlays (e.g. the date-picker popover).
  final Key? fieldKey;

  /// `date-input-group--full-width` (`w-full`).
  final bool fullWidth;

  @override
  State<HeroDateInputGroup> createState() => _HeroDateInputGroupState();
}

/// HeroUI v3 date input group fill classes (`.date-input-group--primary` /
/// `--secondary`).
enum HeroDateInputGroupVariant {
  /// `bg-field` + `shadow-field` (default).
  primary,

  /// `--date-input-group-bg: var(--default)`, no shadow.
  secondary,
}

class _HeroDateInputGroupState extends State<HeroDateInputGroup> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled;
    final radius = HeroTokens.radiusField.resolve(context).x;
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
          radius: radius,
          builder: (context, node, focused) => Opacity(
            opacity: enabled
                ? 1.0
                : HeroTokens.doubleDisabledOpacity.resolve(context),
            child: MouseRegion(
              cursor: enabled
                  ? SystemMouseCursors.click
                  : SystemMouseCursors.basic,
              onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
              onExit: enabled ? (_) => setState(() => _hovered = false) : null,
              child: GestureDetector(
                key: widget.fieldKey,
                onTap: enabled ? widget.onTap : null,
                child: _buildFieldBox(context, focused),
              ),
            ),
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

  Widget _buildFieldBox(BuildContext context, bool focused) {
    final enabled = widget.enabled;
    final field = HeroTokens.colorField.resolve(context);
    final fieldHover = HeroTokens.colorFieldHover.resolve(context);
    final fieldFocus = HeroTokens.colorFieldFocus.resolve(context);
    final defaultColor = HeroTokens.colorDefault.resolve(context);

    // Background per state: invalid > focused > hovered > rest.
    Color background = widget.variant == HeroDateInputGroupVariant.secondary
        ? defaultColor
        : field;
    if (widget.error) {
      background = fieldFocus;
    } else if (focused) {
      background = fieldFocus;
    } else if (_hovered && enabled) {
      background = widget.variant == HeroDateInputGroupVariant.secondary
          ? HeroTokens.colorDefaultHover.resolve(context)
          : fieldHover;
    }

    var style = BoxDecoration(
      color: background,
      borderRadius: BorderRadius.circular(HeroTokens.radiusField.resolve(context).x),
      border: widget.error
          ? Border.all(
              color: HeroTokens.colorDanger.resolve(context),
              width: HeroTokens.doubleBorderWidth.resolve(context),
            )
          : _hovered && enabled
              ? Border.all(
                  color: HeroTokens.colorFieldBorderHover.resolve(context),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                )
              : null,
    );
    if (widget.variant == HeroDateInputGroupVariant.primary) {
      final shadows = HeroTokens.shadowField.resolve(context);
      if (shadows.isNotEmpty) style = style.copyWith(boxShadow: shadows);
    }

    final placeholder = HeroTokens.colorFieldPlaceholder.resolve(context);
    final trailing = widget.suffix ??
        (widget.onClear != null && widget.value != null
            ? GestureDetector(
                onTap: enabled ? widget.onClear : null,
                child: Icon(Icons.close_rounded, size: 16, color: placeholder),
              )
            : Icon(Icons.keyboard_arrow_down_rounded, size: 18, color: placeholder));

    return AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 150),
      ),
      curve: heroEaseSmooth,
      width: widget.fullWidth ? double.infinity : null,
      height: HeroTokens.doubleInputMinHeight.resolve(context), // h-9 (36)
      decoration: style,
      child: Row(
        mainAxisSize: MainAxisSize.min, // .date-input-group is inline-flex
        children: [
          const SizedBox(width: 12), // ms-3
          widget.prefix ??
              Icon(Icons.calendar_month_rounded, size: 16, color: placeholder),
          Padding(
            padding: EdgeInsets.symmetric(
              horizontal: HeroTokens.doubleInputPaddingX.resolve(context), // px-3
              vertical: HeroTokens.doubleInputPaddingY.resolve(context), // py-2
            ),
            child: HeroDateSegments(
              value: widget.value,
              enabled: enabled,
              error: widget.error,
            ),
          ),
          trailing,
          const SizedBox(width: 12), // me-3
        ],
      ),
    );
  }
}

/// The segment row (`date-input-group__input`) — day / month / year with `/`
/// literals. Segments are `rounded-md px-0.5 text-end`, literals `text-muted`,
/// placeholders `text-field-placeholder`, the focused segment `bg-accent-soft
/// text-accent-soft-foreground` (danger-soft while invalid), disabled
/// segments at 50% opacity.
///
/// Internal building block: `HeroDatePicker` / `HeroDateRangePicker` reuse it
/// inside their triggers. Segments are read-only (tap to focus); keyboard
/// digit-entry is not implemented in this mirror.
class HeroDateSegments extends StatefulWidget {
  const HeroDateSegments({
    super.key,
    this.value,
    this.enabled = true,
    this.error = false,
  });

  final DateTime? value;
  final bool enabled;
  final bool error;

  @override
  State<HeroDateSegments> createState() => _HeroDateSegmentsState();
}

class _HeroDateSegmentsState extends State<HeroDateSegments> {
  int? _focusedSegment; // 0 = day, 1 = month, 2 = year

  @override
  Widget build(BuildContext context) {
    final value = widget.value;
    final segments = [
      (text: value?.day.toString().padLeft(2, '0') ?? '', placeholder: '--'),
      (text: value?.month.toString().padLeft(2, '0') ?? '', placeholder: '--'),
      (text: '${value?.year ?? ''}', placeholder: '----'),
    ];
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        for (var i = 0; i < segments.length; i++) ...[
          if (i > 0)
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 1), // gap-px
              child: Text(
                '/',
                style: TextStyle(
                  fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                  color: HeroTokens.colorMuted.resolve(context),
                ),
              ),
            ),
          _HeroDateSegment(
            text: segments[i].text,
            placeholder: segments[i].placeholder,
            focused: _focusedSegment == i,
            error: widget.error,
            enabled: widget.enabled,
            onTap: widget.enabled
                ? () => setState(() => _focusedSegment = i)
                : null,
          ),
        ],
      ],
    );
  }
}

class _HeroDateSegment extends StatelessWidget {
  const _HeroDateSegment({
    required this.text,
    required this.placeholder,
    required this.focused,
    required this.error,
    required this.enabled,
    required this.onTap,
  });

  final String text;
  final String placeholder;
  final bool focused;
  final bool error;
  final bool enabled;
  final VoidCallback? onTap;

  @override
  Widget build(BuildContext context) {
    final hasValue = text.isNotEmpty;
    final accentSoft = HeroTokens.colorAccentSoft.resolve(context);
    final accentSoftForeground =
        HeroTokens.colorAccentSoftForeground.resolve(context);
    final danger = HeroTokens.colorDanger.resolve(context);
    final dangerSoft = HeroTokens.colorDangerSoft.resolve(context);
    final dangerSoftForeground =
        HeroTokens.colorDangerSoftForeground.resolve(context);
    final foreground = HeroTokens.colorFieldForeground.resolve(context);
    final placeholderColor = HeroTokens.colorFieldPlaceholder.resolve(context);

    Color? background;
    Color textColor;
    if (focused) {
      background = error ? dangerSoft : accentSoft;
      textColor = error ? dangerSoftForeground : accentSoftForeground;
    } else {
      textColor = error
          ? danger
          : hasValue
              ? foreground
              : placeholderColor;
    }

    return GestureDetector(
      onTap: onTap,
      child: AnimatedContainer(
        duration: HeroMotion.durationOf(
          context,
          const Duration(milliseconds: 100),
        ),
        curve: heroEaseOut,
        padding: const EdgeInsets.symmetric(horizontal: 2), // px-0.5
        decoration: BoxDecoration(
          color: background,
          borderRadius: BorderRadius.circular(
            HeroTokens.radiusMd.resolve(context).x, // rounded-md (6)
          ),
        ),
        child: Opacity(
          opacity: enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context),
          child: Text(
            hasValue ? text : placeholder,
            style: TextStyle(
              fontSize: HeroTokens.doubleInputFontSize.resolve(context), // text-sm
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: textColor,
            ),
          ),
        ),
      ),
    );
  }
}

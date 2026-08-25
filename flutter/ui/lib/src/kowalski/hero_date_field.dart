import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 date field (date-field.css).
///
/// `.date-field` is the layout wrapper — `flex flex-col gap-1` with a
/// `w-fit` label and a description slot that is hidden while invalid. Its
/// body is the segment row (day / month / year) shared with the date input
/// group (`.date-input-group__segment`): each segment renders as a rounded
/// box (`bg-segment`, field border, `rounded-md`), the `/` literals are
/// muted, the focused segment paints `bg-accent-soft
/// text-accent-soft-foreground`, invalid segments turn danger, and disabled
/// segments drop to 50% opacity.
///
/// ```dart
/// HeroDateField(
///   value: birthday,
///   label: 'Birthday',
///   onChanged: (date) => setState(() => birthday = date),
/// )
/// ```
class HeroDateField extends StatefulWidget {
  const HeroDateField({
    super.key,
    this.value,
    this.onChanged,
    this.label,
    this.description,
    this.error = false,
    this.enabled = true,
    this.fullWidth = false,
  });

  /// The displayed date; null renders placeholder segments (`-- / -- / ----`).
  final DateTime? value;

  /// Reserved for segment edits. This mirror renders read-only segments
  /// (tap to focus, like the source's focusable segments); keyboard
  /// digit-entry is not implemented, so callers drive [value] themselves.
  final ValueChanged<DateTime?>? onChanged;

  /// Optional field label (`w-fit text-sm font-medium`).
  final String? label;

  /// Helper text (`text-xs` muted); hidden while [error] is true
  /// (date-field.css hides `[data-slot="description"]` when invalid).
  final String? description;

  /// Invalid state: description hidden, segments turn danger.
  final bool error;

  /// Disabled state: segments drop to 50% opacity (`status-disabled`).
  final bool enabled;

  /// `date-field--full-width` (`w-full`).
  final bool fullWidth;

  @override
  State<HeroDateField> createState() => _HeroDateFieldState();
}

class _HeroDateFieldState extends State<HeroDateField> {
  int? _focusedSegment; // 0 = day, 1 = month, 2 = year

  @override
  Widget build(BuildContext context) {
    final enabled = widget.enabled;
    final value = widget.value;
    final segments = [
      (label: value?.day.toString().padLeft(2, '0') ?? '', placeholder: '--'),
      (label: value?.month.toString().padLeft(2, '0') ?? '', placeholder: '--'),
      (label: '${value?.year ?? ''}', placeholder: '----'),
    ];

    return SizedBox(
      width: widget.fullWidth ? double.infinity : null, // date-field--full-width
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
        if (widget.label != null)
          Padding(
            padding: const EdgeInsets.only(bottom: 4), // gap-1 (4)
            child: Text(
              widget.label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            for (var i = 0; i < segments.length; i++) ...[
              if (i > 0) _HeroDateLiteral(text: '/'),
              _HeroDateSegmentBox(
                text: segments[i].label,
                placeholder: segments[i].placeholder,
                focused: _focusedSegment == i,
                error: widget.error,
                enabled: enabled,
                onTap: enabled
                    ? () => setState(() => _focusedSegment = i)
                    : null,
              ),
            ],
          ],
        ),
        if (widget.description != null && !widget.error)
          Padding(
            padding: const EdgeInsets.only(top: 4), // gap-1 (4)
            child: Text(
              widget.description!,
              style: TextStyle(
                fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                color: HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
        ],
        ),
    );
  }
}

/// The `/` separator between segments — `.date-input-group__segment`
/// `[data-type="literal"]`: `p-0 text-muted`.
class _HeroDateLiteral extends StatelessWidget {
  const _HeroDateLiteral({required this.text});

  final String text;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 2),
      child: Text(
        text,
        style: TextStyle(
          fontSize: HeroTokens.typeSm.resolve(context).fontSize,
          color: HeroTokens.colorMuted.resolve(context),
        ),
      ),
    );
  }
}

/// One segment box — `bg-segment` + field border + `rounded-md` (6);
/// focused → `bg-accent-soft text-accent-soft-foreground`; invalid →
/// `text-danger` (focused → `bg-danger-soft text-danger-soft-foreground`);
/// disabled → 50% opacity.
class _HeroDateSegmentBox extends StatefulWidget {
  const _HeroDateSegmentBox({
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
  State<_HeroDateSegmentBox> createState() => _HeroDateSegmentBoxState();
}

class _HeroDateSegmentBoxState extends State<_HeroDateSegmentBox> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final hasValue = widget.text.isNotEmpty;
    final enabled = widget.enabled && widget.onTap != null;
    final accentSoft = HeroTokens.colorAccentSoft.resolve(context);
    final accentSoftForeground =
        HeroTokens.colorAccentSoftForeground.resolve(context);
    final danger = HeroTokens.colorDanger.resolve(context);
    final dangerSoft = HeroTokens.colorDangerSoft.resolve(context);
    final dangerSoftForeground =
        HeroTokens.colorDangerSoftForeground.resolve(context);
    final segment = HeroTokens.colorSegment.resolve(context);
    final segmentForeground = HeroTokens.colorSegmentForeground.resolve(context);
    final placeholderColor = HeroTokens.colorFieldPlaceholder.resolve(context);

    Color? background;
    Color textColor;
    if (widget.focused) {
      background = widget.error ? dangerSoft : accentSoft;
      textColor = widget.error ? dangerSoftForeground : accentSoftForeground;
    } else {
      background = _hovered && enabled ? HeroTokens.colorFieldHover.resolve(context) : segment;
      textColor = widget.error
          ? danger
          : hasValue
              ? segmentForeground
              : placeholderColor;
    }

    return Opacity(
      opacity: widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: MouseRegion(
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
            curve: heroEaseOut,
            height: 32,
            padding: const EdgeInsets.symmetric(horizontal: 8), // px-2
            alignment: Alignment.center,
            decoration: BoxDecoration(
              color: background,
              borderRadius: BorderRadius.circular(
                HeroTokens.radiusMd.resolve(context).x, // rounded-md (6)
              ),
              border: Border.all(
                color: HeroTokens.colorFieldBorder.resolve(context),
                width: HeroTokens.doubleBorderWidth.resolve(context),
              ),
            ),
            child: Text(
              hasValue ? widget.text : widget.placeholder,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: textColor,
              ),
            ),
          ),
        ),
      ),
    );
  }
}

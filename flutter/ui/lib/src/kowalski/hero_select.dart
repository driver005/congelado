import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';
import 'hero_list_box.dart';
import 'hero_popup.dart';

/// One option of a [HeroSelect].
class HeroSelectItem<T> {
  const HeroSelectItem({
    required this.value,
    required this.label,
    this.description,
    this.leading,
    this.danger = false,
    this.disabled = false,
  });

  final T value;
  final String label;
  final String? description;
  final Widget? leading;
  final bool danger;
  final bool disabled;
}

/// A HeroUI v3 select (select.css `.select`) — a field-shaped trigger that
/// opens a popup list of options.
///
/// `.select__trigger` — `min-h-9 rounded-field border bg-field px-3 py-2
/// text-sm shadow-field` (hover `bg-field-hover`), `pe-7` with the chevron
/// indicator (`end-2 size-4 text-field-placeholder`, rotated 180° when open);
/// the popup is `bg-overlay` radius 24 `shadow-overlay` with the list-box
/// padded `p-1.5` and items `px-2.5`.
class HeroSelect<T> extends StatefulWidget {
  const HeroSelect({
    super.key,
    required this.items,
    this.value,
    this.onChanged,
    this.placeholder = 'Select an option',
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.fullWidth = false,
  });

  final List<HeroSelectItem<T>> items;
  final T? value;
  final ValueChanged<T>? onChanged;
  final String placeholder;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final HeroInputVariant variant;
  final bool fullWidth;

  @override
  State<HeroSelect<T>> createState() => _HeroSelectState<T>();
}

class _HeroSelectState<T> extends State<HeroSelect<T>> {
  final HeroPopupController _controller = HeroPopupController();
  bool _hovered = false;
  HeroSelectItem<T>? _selected;

  @override
  void initState() {
    super.initState();
    _controller.addListener(_onOpenChanged);
    _selected = _itemFor(widget.value);
  }

  @override
  void didUpdateWidget(HeroSelect<T> oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value) {
      _selected = _itemFor(widget.value);
    }
  }

  @override
  void dispose() {
    _controller.removeListener(_onOpenChanged);
    _controller.dispose();
    super.dispose();
  }

  void _onOpenChanged() {
    if (mounted) setState(() {});
  }

  HeroSelectItem<T>? _itemFor(T? value) {
    if (value == null) return null;
    for (final item in widget.items) {
      if (item.value == value) return item;
    }
    return null;
  }

  @override
  Widget build(BuildContext context) {
    final radius = HeroTokens.radiusField.resolve(context).x;
    final open = _controller.isOpen;
    final opacity =
        widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (widget.label != null)
          Padding(
            padding: EdgeInsets.only(bottom: HeroTokens.space1.resolve(context)),
            child: Text(
              widget.label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        HeroPopup(
          controller: _controller,
          child: HeroFocusRing(
            radius: radius,
            builder: (context, node, focused) => MouseRegion(
              cursor: widget.enabled
                  ? SystemMouseCursors.click
                  : SystemMouseCursors.basic,
              onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
              onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
              child: Opacity(
                opacity: opacity,
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: heroInputTransitionMs),
                  ),
                  curve: HeroMotion.smooth,
                  height: HeroTokens.doubleInputMinHeight.resolve(context),
                  width: widget.fullWidth ? double.infinity : null,
                  padding: EdgeInsets.only(
                    left: HeroTokens.doubleInputPaddingX.resolve(context),
                    right: 28,
                  ),
                  decoration: BoxDecoration(
                    color: _triggerColor(focused, open),
                    borderRadius: BorderRadius.circular(radius),
                    border: Border.all(
                      color: widget.error
                          ? HeroTokens.colorDanger.resolve(context)
                          : _hovered && !focused
                              ? HeroTokens.colorFieldBorderHover.resolve(context)
                              : HeroTokens.colorFieldBorder.resolve(context),
                      width: HeroTokens.doubleBorderWidth.resolve(context),
                    ),
                    boxShadow: widget.variant == HeroInputVariant.primary
                        ? HeroTokens.shadowField.resolve(context)
                        : null,
                  ),
                  child: Row(
                    children: [
                      Expanded(
                        child: Text(
                          _selected?.label ?? widget.placeholder,
                          overflow: TextOverflow.ellipsis,
                          style: TextStyle(
                            fontSize:
                                HeroTokens.doubleInputFontSize.resolve(context),
                            color: _selected == null
                                ? HeroTokens.colorFieldPlaceholder.resolve(context)
                                : HeroTokens.colorFieldForeground.resolve(context),
                          ),
                        ),
                      ),
                      AnimatedRotation(
                        turns: open ? 0.5 : 0,
                        duration: HeroMotion.durationOf(
                          context,
                          const Duration(milliseconds: 150),
                        ),
                        curve: HeroMotion.smooth,
                        child: Icon(
                          Icons.keyboard_arrow_down_rounded,
                          size: 16,
                          color: HeroTokens.colorFieldPlaceholder.resolve(context),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
          builder: (context) {
            final overlayShadow = HeroTokens.shadowOverlay.resolve(context);
            return Container(
              decoration: BoxDecoration(
                color: HeroTokens.colorOverlay.resolve(context),
                borderRadius: BorderRadius.circular(
                  HeroTokens.radius3xl.resolve(context).x,
                ),
                boxShadow: overlayShadow,
              ),
              child: HeroListBox(
                padding: 6,
                itemPaddingX: 10,
                children: [
                  for (final item in widget.items)
                    HeroListBoxItem(
                      label: item.label,
                      description: item.description,
                      leading: item.leading,
                      selected: item.value == widget.value,
                      danger: item.danger,
                      disabled: item.disabled || !widget.enabled,
                      onPressed: () {
                        widget.onChanged?.call(item.value);
                        _controller.close();
                      },
                    ),
                ],
              ),
            );
          },
        ),
        if (widget.helperText != null)
          Padding(
            padding: EdgeInsets.only(top: HeroTokens.space1.resolve(context)),
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

  Color _triggerColor(bool focused, bool open) {
    if (widget.variant == HeroInputVariant.secondary) {
      if (focused || open) return HeroTokens.colorDefault.resolve(context);
      if (_hovered) return HeroTokens.colorDefaultHover.resolve(context);
      return HeroTokens.colorDefault.resolve(context);
    }
    if (focused || open) return HeroTokens.colorFieldFocus.resolve(context);
    if (_hovered) return HeroTokens.colorFieldHover.resolve(context);
    return HeroTokens.colorField.resolve(context);
  }
}

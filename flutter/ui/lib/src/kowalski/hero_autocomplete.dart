import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';
import 'hero_list_box.dart';
import 'hero_popup.dart';

/// One suggestion of a [HeroAutocomplete].
class HeroAutocompleteItem<T> {
  const HeroAutocompleteItem({
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

/// A HeroUI v3 autocomplete (autocomplete.css `.autocomplete`) — a field that
/// filters a popup list of suggestions as the user types.
///
/// The trigger uses the input anatomy (`rounded-field`, `bg-field`,
/// `shadow-field`, hover `bg-field-hover`, focus `status-focused-field`) with
/// a chevron indicator (`end-2 size-4 text-field-placeholder`, rotated when
/// open) and a clear button that fades in when the field is non-empty. The
/// popup is `bg-overlay` radius 24 `shadow-overlay` `pt-2`, containing the
/// filtered list-box (`max-h-[320px] p-1.5`, items `px-2.5`).
class HeroAutocomplete<T> extends StatefulWidget {
  const HeroAutocomplete({
    super.key,
    required this.items,
    this.controller,
    this.onChanged,
    this.onSelected,
    this.placeholder = 'Search…',
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.fullWidth = false,
  });

  final List<HeroAutocompleteItem<T>> items;
  final TextEditingController? controller;
  final ValueChanged<String>? onChanged;
  final ValueChanged<T>? onSelected;
  final String placeholder;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final HeroInputVariant variant;
  final bool fullWidth;

  @override
  State<HeroAutocomplete<T>> createState() => _HeroAutocompleteState<T>();
}

class _HeroAutocompleteState<T> extends State<HeroAutocomplete<T>> {
  final HeroPopupController _controller = HeroPopupController();
  late final TextEditingController _textController;
  bool _hovered = false;
  String _query = '';

  @override
  void initState() {
    super.initState();
    _textController = widget.controller ?? TextEditingController();
    if (widget.controller == null) {
      _textController.addListener(_onTextChanged);
    }
  }

  @override
  void didUpdateWidget(HeroAutocomplete<T> oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.controller != widget.controller) {
      if (oldWidget.controller == null) {
        _textController.removeListener(_onTextChanged);
      }
      _textController = widget.controller ?? TextEditingController();
      if (widget.controller == null) {
        _textController.addListener(_onTextChanged);
      }
      _query = _textController.text;
    }
  }

  @override
  void dispose() {
    if (widget.controller == null) {
      _textController.removeListener(_onTextChanged);
      _textController.dispose();
    }
    _controller.dispose();
    super.dispose();
  }

  void _onTextChanged() {
    setState(() => _query = _textController.text);
    _controller.open();
  }

  List<HeroAutocompleteItem<T>> get _filtered {
    final q = _query.trim().toLowerCase();
    if (q.isEmpty) return widget.items;
    return widget.items
        .where((i) => i.label.toLowerCase().contains(q))
        .toList();
  }

  void _select(HeroAutocompleteItem<T> item) {
    _textController.text = item.label;
    _textController.selection = TextSelection.collapsed(
      offset: _textController.text.length,
    );
    setState(() => _query = item.label);
    widget.onSelected?.call(item.value);
    _controller.close();
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
          enterDuration: const Duration(milliseconds: 250),
          enterCurve: HeroMotion.outFluid,
          child: HeroFocusRing(
            radius: radius,
            builder: (context, node, focused) => MouseRegion(
              cursor: widget.enabled
                  ? SystemMouseCursors.text
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
                    right: HeroTokens.space2.resolve(context),
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
                        child: TextField(
                          controller: _textController,
                          focusNode: node,
                          enabled: widget.enabled,
                          style: TextStyle(
                            fontSize:
                                HeroTokens.doubleInputFontSize.resolve(context),
                            color: HeroTokens.colorFieldForeground.resolve(context),
                          ),
                          decoration: InputDecoration(
                            isCollapsed: true,
                            border: InputBorder.none,
                            hintText: widget.placeholder,
                            hintStyle: TextStyle(
                              fontSize:
                                  HeroTokens.doubleInputFontSize.resolve(context),
                              color: HeroTokens.colorFieldPlaceholder.resolve(context),
                            ),
                          ),
                          onChanged: (v) {
                            widget.onChanged?.call(v);
                            _controller.open();
                          },
                          onTap: widget.enabled ? () => _controller.open() : null,
                        ),
                      ),
                      if (widget.enabled)
                        HeroPopupClearButton(
                          visible: _query.isNotEmpty,
                          onPressed: () {
                            _textController.clear();
                            setState(() => _query = '');
                            widget.onChanged?.call('');
                            _controller.close();
                          },
                        ),
                      SizedBox(width: HeroTokens.space05.resolve(context)),
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
            final filtered = _filtered;
            final overlayShadow = HeroTokens.shadowOverlay.resolve(context);
            return Container(
              padding: EdgeInsets.only(top: HeroTokens.space2.resolve(context)),
              decoration: BoxDecoration(
                color: HeroTokens.colorOverlay.resolve(context),
                borderRadius: BorderRadius.circular(
                  HeroTokens.radius3xl.resolve(context).x,
                ),
                boxShadow: overlayShadow,
              ),
              child: ConstrainedBox(
                constraints: const BoxConstraints(maxHeight: 320),
                child: filtered.isEmpty
                    ? Padding(
                        padding: EdgeInsets.all(HeroTokens.space4.resolve(context)),
                        child: Text(
                          'No results found.',
                          textAlign: TextAlign.center,
                          style: TextStyle(
                            fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                            color: HeroTokens.colorOverlayForeground
                                .resolve(context)
                                .withValues(alpha: 0.6),
                          ),
                        ),
                      )
                    : HeroListBox(
                        padding: 6,
                        itemPaddingX: 10,
                        children: [
                          for (final item in filtered)
                            HeroListBoxItem(
                              label: item.label,
                              description: item.description,
                              leading: item.leading,
                              danger: item.danger,
                              disabled: item.disabled || !widget.enabled,
                              onPressed: () => _select(item),
                            ),
                        ],
                      ),
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

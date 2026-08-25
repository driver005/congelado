import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';
import 'hero_list_box.dart';
import 'hero_popup.dart';

/// One option of a [HeroComboBox].
class HeroComboBoxItem<T> {
  const HeroComboBoxItem({
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

/// A HeroUI v3 combo box (combo-box.css `.combo-box`) — an editable field with
/// a popup list of selectable options.
///
/// The field reuses the input anatomy; the `.combo-box__trigger` chevron sits
/// absolutely at the inline end (`end-0 pe-2 size-4 text-field-placeholder`,
/// `text-field-foreground` on hover, rotated 180° when open). The popup is
/// `bg-overlay` radius 24 `shadow-overlay` with the list-box `p-1.5`, items
/// `px-2.5` and a checkmark on the selected option.
class HeroComboBox<T> extends StatefulWidget {
  const HeroComboBox({
    super.key,
    required this.items,
    this.selectedValue,
    this.onChanged,
    this.onInputChanged,
    this.controller,
    this.placeholder = 'Select an option',
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.fullWidth = false,
  });

  final List<HeroComboBoxItem<T>> items;
  final T? selectedValue;
  final ValueChanged<T>? onChanged;

  /// Fired while the user types free text (filtering).
  final ValueChanged<String>? onInputChanged;

  final TextEditingController? controller;
  final String placeholder;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final HeroInputVariant variant;
  final bool fullWidth;

  @override
  State<HeroComboBox<T>> createState() => _HeroComboBoxState<T>();
}

class _HeroComboBoxState<T> extends State<HeroComboBox<T>> {
  final HeroPopupController _controller = HeroPopupController();
  late final TextEditingController _textController;
  bool _hovered = false;
  String _text = '';

  @override
  void initState() {
    super.initState();
    _textController = widget.controller ?? TextEditingController();
    _text = _labelFor(widget.selectedValue) ?? '';
    _syncControllerText(_text);
    _attachListener();
  }

  @override
  void didUpdateWidget(HeroComboBox<T> oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.controller != widget.controller) {
      _detachListener();
      _textController = widget.controller ?? TextEditingController();
      _attachListener();
    }
    if (oldWidget.selectedValue != widget.selectedValue) {
      final label = _labelFor(widget.selectedValue);
      if (label != null && _textController.text != label) {
        _text = label;
        _syncControllerText(label);
      }
    }
  }

  @override
  void dispose() {
    _detachListener();
    if (widget.controller == null) _textController.dispose();
    _controller.dispose();
    super.dispose();
  }

  void _attachListener() => _textController.addListener(_onTextChanged);

  void _detachListener() => _textController.removeListener(_onTextChanged);

  void _onTextChanged() {
    if (_textController.text != _text) {
      setState(() => _text = _textController.text);
    }
  }

  void _syncControllerText(String value) {
    if (_textController.text != value) {
      _textController.text = value;
    }
  }

  String? _labelFor(T? value) {
    if (value == null) return null;
    for (final item in widget.items) {
      if (item.value == value) return item.label;
    }
    return null;
  }

  List<HeroComboBoxItem<T>> get _filtered {
    final q = _text.trim().toLowerCase();
    if (q.isEmpty) return widget.items;
    return widget.items.where((i) => i.label.toLowerCase().contains(q)).toList();
  }

  void _select(HeroComboBoxItem<T> item) {
    setState(() => _text = item.label);
    _syncControllerText(item.label);
    widget.onChanged?.call(item.value);
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
                            hintText: _text.isEmpty ? widget.placeholder : null,
                            hintStyle: TextStyle(
                              fontSize:
                                  HeroTokens.doubleInputFontSize.resolve(context),
                              color: HeroTokens.colorFieldPlaceholder.resolve(context),
                            ),
                          ),
                          onChanged: (v) {
                            setState(() => _text = v);
                            widget.onInputChanged?.call(v);
                            _controller.open();
                          },
                        ),
                      ),
                      GestureDetector(
                        behavior: HitTestBehavior.opaque,
                        onTap: widget.enabled
                            ? () => _controller.toggle()
                            : null,
                        child: Padding(
                          padding: EdgeInsets.symmetric(
                            vertical: 8,
                            horizontal: HeroTokens.space05.resolve(context),
                          ),
                          child: AnimatedRotation(
                            turns: open ? 0.5 : 0,
                            duration: HeroMotion.durationOf(
                              context,
                              const Duration(milliseconds: 150),
                            ),
                            curve: HeroMotion.smooth,
                            child: Icon(
                              Icons.keyboard_arrow_down_rounded,
                              size: 16,
                              color: _hovered
                                  ? HeroTokens.colorFieldForeground.resolve(context)
                                  : HeroTokens.colorFieldPlaceholder.resolve(context),
                            ),
                          ),
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
              decoration: BoxDecoration(
                color: HeroTokens.colorOverlay.resolve(context),
                borderRadius: BorderRadius.circular(
                  HeroTokens.radius3xl.resolve(context).x,
                ),
                boxShadow: overlayShadow,
              ),
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
                            selected: item.value == widget.selectedValue,
                            danger: item.danger,
                            disabled: item.disabled || !widget.enabled,
                            onPressed: () => _select(item),
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

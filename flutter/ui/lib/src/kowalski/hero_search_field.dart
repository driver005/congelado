import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';

/// A HeroUI v3 search field (search-field.css `.search-field`) — a field with
/// a leading search icon and a trailing clear button.
///
/// The `.search-field__group` reuses the input anatomy (rounded-field,
/// `bg-field`, `shadow-field`, hover `bg-field-hover`, focus
/// `status-focused-field`); search-field.css only styles the icon and the
/// clear button (a resized close button: `size-5` with a `size-3` icon).
class HeroSearchField extends StatefulWidget {
  const HeroSearchField({
    super.key,
    this.controller,
    this.variant = HeroInputVariant.primary,
    this.placeholder = 'Search…',
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.fullWidth = false,
    this.onChanged,
    this.onSubmitted,
    this.focusNode,
  });

  final TextEditingController? controller;
  final HeroInputVariant variant;
  final String placeholder;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final bool fullWidth;
  final ValueChanged<String>? onChanged;
  final ValueChanged<String>? onSubmitted;
  final FocusNode? focusNode;

  @override
  State<HeroSearchField> createState() => _HeroSearchFieldState();
}

class _HeroSearchFieldState extends State<HeroSearchField> {
  late final TextEditingController _controller;
  bool _hovered = false;
  String _text = '';

  TextEditingController get _active => widget.controller ?? _controller;

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController();
    _text = widget.controller?.text ?? '';
    _attach();
  }

  @override
  void didUpdateWidget(HeroSearchField oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.controller != widget.controller) {
      _detach();
      _attach();
      _text = widget.controller?.text ?? '';
    }
  }

  @override
  void dispose() {
    _detach();
    _controller.dispose();
    super.dispose();
  }

  void _attach() {
    final c = widget.controller;
    if (c != null) {
      _text = c.text;
      c.addListener(_onControllerChanged);
    }
  }

  void _detach() {
    widget.controller?.removeListener(_onControllerChanged);
  }

  void _onControllerChanged() {
    setState(() => _text = widget.controller?.text ?? '');
  }

  void _handleChanged(String value) {
    _text = value;
    widget.onChanged?.call(value);
  }

  @override
  Widget build(BuildContext context) {
    final radius = HeroTokens.radiusField.resolve(context).x;
    final disabled = !widget.enabled;
    final opacity =
        disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

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
        HeroFocusRing(
          radius: radius,
          builder: (context, node, focused) => MouseRegion(
            cursor: disabled ? SystemMouseCursors.basic : SystemMouseCursors.text,
            onEnter: disabled ? null : (_) => setState(() => _hovered = true),
            onExit: disabled ? null : (_) => setState(() => _hovered = false),
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
                padding: EdgeInsets.zero,
                decoration: BoxDecoration(
                  color: _groupColor(focused),
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
                    SizedBox(width: HeroTokens.space3.resolve(context)),
                    Icon(
                      Icons.search_rounded,
                      size: 16,
                      color: HeroTokens.colorFieldPlaceholder.resolve(context),
                    ),
                    SizedBox(width: HeroTokens.space05.resolve(context)),
                    Expanded(
                      child: TextField(
                        controller: _active,
                        focusNode: node,
                        enabled: widget.enabled,
                        style: TextStyle(
                          fontSize: HeroTokens.doubleInputFontSize.resolve(context),
                          color: HeroTokens.colorFieldForeground.resolve(context),
                        ),
                        decoration: InputDecoration(
                          isCollapsed: true,
                          border: InputBorder.none,
                          hintText: widget.placeholder,
                          hintStyle: TextStyle(
                            fontSize: HeroTokens.doubleInputFontSize.resolve(context),
                            color: HeroTokens.colorFieldPlaceholder.resolve(context),
                          ),
                        ),
                        onChanged: _handleChanged,
                        onSubmitted: widget.onSubmitted,
                      ),
                    ),
                    if (widget.enabled)
                      Padding(
                        padding: EdgeInsets.only(
                          right: HeroTokens.space2.resolve(context),
                        ),
                        child: _HeroSearchClearButton(
                          visible: _text.isNotEmpty,
                          onPressed: () {
                            _active.clear();
                            _text = '';
                            widget.onChanged?.call('');
                          },
                        ),
                      ),
                  ],
                ),
              ),
            ),
          ),
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

  Color _groupColor(bool focused) {
    final variant = widget.variant;
    if (variant == HeroInputVariant.secondary) {
      if (focused) return HeroTokens.colorDefault.resolve(context);
      if (_hovered) return HeroTokens.colorDefaultHover.resolve(context);
      return HeroTokens.colorDefault.resolve(context);
    }
    if (focused) return HeroTokens.colorFieldFocus.resolve(context);
    if (_hovered) return HeroTokens.colorFieldHover.resolve(context);
    return HeroTokens.colorField.resolve(context);
  }
}

/// The search-field clear button — a resized close button (`.search-field__clear-button`:
/// `size-5`, icon `size-3`), fading out when there is nothing to clear.
class _HeroSearchClearButton extends StatefulWidget {
  const _HeroSearchClearButton({
    required this.visible,
    required this.onPressed,
  });

  final bool visible;
  final VoidCallback onPressed;

  @override
  State<_HeroSearchClearButton> createState() => _HeroSearchClearButtonState();
}

class _HeroSearchClearButtonState extends State<_HeroSearchClearButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    return AnimatedOpacity(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 150),
      ),
      curve: HeroMotion.smooth,
      opacity: widget.visible ? 1 : 0,
      child: IgnorePointer(
        ignoring: !widget.visible,
        child: MouseRegion(
          cursor: SystemMouseCursors.click,
          onEnter: (_) => setState(() => _hovered = true),
          onExit: (_) => setState(() => _hovered = false),
          child: GestureDetector(
            onTap: widget.visible ? widget.onPressed : null,
            onTapDown: (_) => setState(() => _pressed = true),
            onTapUp: (_) => setState(() => _pressed = false),
            onTapCancel: () => setState(() => _pressed = false),
            child: AnimatedScale(
              scale: _pressed ? 0.93 : 1.0,
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 250),
              ),
              curve: heroEaseOutQuart,
              child: AnimatedContainer(
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 100),
                ),
                curve: HeroMotion.out,
                width: 20,
                height: 20,
                decoration: BoxDecoration(
                  color: _hovered
                      ? HeroTokens.colorDefaultHover.resolve(context)
                      : HeroTokens.colorDefault.resolve(context),
                  borderRadius: BorderRadius.circular(
                    HeroTokens.radiusXl.resolve(context).x,
                  ),
                ),
                child: Icon(
                  Icons.close_rounded,
                  size: 12,
                  color: HeroTokens.colorMuted.resolve(context),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 scroll shadow (scroll-shadow.css `.scroll-shadow`) — a
/// scrollable region whose edges fade out, signalling more content.
///
/// scroll-shadow.css: `.scroll-shadow` — `relative`; `--horizontal`/
/// `--vertical` scroll the region and apply a linear-gradient mask on the
/// scroll edges (`--scroll-shadow-size: 40px`); `--hide-scrollbar` hides the
/// scrollbar. Flutter has no CSS mask, so the fade is rendered as gradient
/// overlays driven by the scroll position.
class HeroScrollShadow extends StatefulWidget {
  const HeroScrollShadow({
    super.key,
    required this.child,
    this.horizontal = false,
    this.vertical = true,
    this.hideScrollbar = false,
    this.controller,
    this.padding,
  });

  /// Scrolls horizontally (fade on the left/right edges).
  final bool horizontal;

  /// Scrolls vertically (fade on the top/bottom edges).
  final bool vertical;

  /// Hides the scrollbar (`.scroll-shadow--hide-scrollbar`).
  final bool hideScrollbar;

  final ScrollController? controller;
  final EdgeInsetsGeometry? padding;

  final Widget child;

  @override
  State<HeroScrollShadow> createState() => _HeroScrollShadowState();
}

class _HeroScrollShadowState extends State<HeroScrollShadow> {
  late final ScrollController _controller =
      widget.controller ?? ScrollController();
  late final bool _ownsController = widget.controller == null;

  bool _atStart = true;
  bool _atEnd = false;

  @override
  void initState() {
    super.initState();
    _controller.addListener(_update);
    WidgetsBinding.instance.addPostFrameCallback((_) => _update());
  }

  @override
  void dispose() {
    _controller.removeListener(_update);
    if (_ownsController) _controller.dispose();
    super.dispose();
  }

  void _update() {
    if (!_controller.hasClients) return;
    final position = _controller.position;
    final atStart = position.pixels <= position.minScrollExtent;
    final atEnd = position.pixels >= position.maxScrollExtent;
    if (atStart != _atStart || atEnd != _atEnd) {
      setState(() {
        _atStart = atStart;
        _atEnd = atEnd;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    const size = 40.0; // --scroll-shadow-size
    final fade = HeroTokens.colorForeground.resolve(context).withValues(alpha: 0.25);

    Widget content = SingleChildScrollView(
      controller: _controller,
      scrollDirection: widget.horizontal && !widget.vertical
          ? Axis.horizontal
          : Axis.vertical,
      padding: widget.padding,
      child: widget.child,
    );

    if (widget.hideScrollbar) {
      content = Scrollbar(
        controller: _controller,
        thumbVisibility: false,
        child: content,
      );
    }

    final overlays = <Widget>[];
    if (widget.vertical) {
      overlays.addAll([
        if (!_atStart)
          Positioned(
            top: 0,
            left: 0,
            right: 0,
            height: size,
            child: IgnorePointer(
              child: DecoratedBox(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.topCenter,
                    end: Alignment.bottomCenter,
                    colors: [fade, fade.withValues(alpha: 0)],
                  ),
                ),
              ),
            ),
          ),
        if (!_atEnd)
          Positioned(
            bottom: 0,
            left: 0,
            right: 0,
            height: size,
            child: IgnorePointer(
              child: DecoratedBox(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.bottomCenter,
                    end: Alignment.topCenter,
                    colors: [fade, fade.withValues(alpha: 0)],
                  ),
                ),
              ),
            ),
          ),
      ]);
    }
    if (widget.horizontal) {
      overlays.addAll([
        if (!_atStart)
          Positioned(
            top: 0,
            bottom: 0,
            left: 0,
            width: size,
            child: IgnorePointer(
              child: DecoratedBox(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.centerLeft,
                    end: Alignment.centerRight,
                    colors: [fade, fade.withValues(alpha: 0)],
                  ),
                ),
              ),
            ),
          ),
        if (!_atEnd)
          Positioned(
            top: 0,
            bottom: 0,
            right: 0,
            width: size,
            child: IgnorePointer(
              child: DecoratedBox(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.centerRight,
                    end: Alignment.centerLeft,
                    colors: [fade, fade.withValues(alpha: 0)],
                  ),
                ),
              ),
            ),
          ),
      ]);
    }

    return Stack(children: [content, ...overlays]);
  }
}

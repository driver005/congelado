import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 focus ring: `status-focused` = `ring-2 ring-focus ring-offset-2`
/// — a 2px accent ring drawn OUTSIDE the element with a 2px gap. Flutter has
/// no native outside-ring, so this wrapper renders the ring in a 2px margin
/// that appears on focus. The 2px layout growth is animated (150ms, matching
/// HeroUI's 150ms transition) so it does not pop.
///
/// The wrapper owns a [FocusNode] and hands it to the wrapped widget through
/// [builder]; the child attaches the node exactly once (do NOT wrap the child
/// in another `Focus` with the same node — double attachment loops the focus
/// tree). The node listener drives the ring.
class HeroFocusRing extends StatefulWidget {
  const HeroFocusRing({
    super.key,
    required this.radius,
    required this.builder,
  });

  /// The child's corner radius; the ring uses `radius + 2` (ring offset).
  final double radius;

  /// Builds the wrapped widget with the ring's focus node.
  final Widget Function(BuildContext context, FocusNode focusNode, bool focused)
      builder;

  @override
  State<HeroFocusRing> createState() => _HeroFocusRingState();
}

class _HeroFocusRingState extends State<HeroFocusRing> {
  final FocusNode _node = FocusNode();
  bool _focused = false;

  @override
  void initState() {
    super.initState();
    _node.addListener(_onFocus);
  }

  void _onFocus() {
    if (_node.hasFocus != _focused) {
      setState(() => _focused = _node.hasFocus);
    }
  }

  @override
  void dispose() {
    _node.removeListener(_onFocus);
    _node.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final focusColor = HeroTokens.colorFocus.resolve(context);
    return AnimatedContainer(
      duration: const Duration(milliseconds: 150),
      padding: EdgeInsets.all(_focused ? 2.0 : 0.0),
      decoration: _focused
          ? BoxDecoration(
              border: Border.all(color: focusColor, width: 2),
              borderRadius: BorderRadius.circular(widget.radius + 2),
            )
          : null,
      child: widget.builder(context, _node, _focused),
    );
  }
}

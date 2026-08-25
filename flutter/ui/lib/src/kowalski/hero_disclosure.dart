import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// `--ease-out-quad` (shared-theme.css) — the disclosure content height curve
/// (disclosure.css `.disclosure__content`).
const Cubic _easeOutQuad = Cubic(0.25, 0.46, 0.45, 0.94);

/// A HeroUI v3 disclosure (disclosure.css) — a single expand/collapse row
/// with a chevron trigger and a height-animated content panel.
///
/// ```dart
/// HeroDisclosure(
///   title: 'Show details',
///   child: const Text('...'),
/// )
/// ```
class HeroDisclosure extends StatefulWidget {
  const HeroDisclosure({
    super.key,
    required this.title,
    required this.child,
    this.isExpanded,
    this.defaultExpanded = false,
    this.onExpandedChange,
    this.isDisabled = false,
  });

  /// The trigger label.
  final String title;

  /// The collapsible content (`.disclosure__body` — `p-2`).
  final Widget child;

  /// Controlled expansion state. When null the disclosure tracks its own
  /// state (seeded from [defaultExpanded]).
  final bool? isExpanded;

  /// Whether the disclosure starts expanded when uncontrolled.
  final bool defaultExpanded;

  final ValueChanged<bool>? onExpandedChange;

  final bool isDisabled;

  @override
  State<HeroDisclosure> createState() => _HeroDisclosureState();
}

class _HeroDisclosureState extends State<HeroDisclosure> {
  late bool _expanded;

  @override
  void initState() {
    super.initState();
    _expanded = widget.defaultExpanded;
  }

  bool get _isExpanded => widget.isExpanded ?? _expanded;

  void _toggle() {
    final next = !_isExpanded;
    if (widget.isExpanded == null) {
      setState(() => _expanded = next);
    }
    widget.onExpandedChange?.call(next);
  }

  @override
  Widget build(BuildContext context) {
    final expanded = _isExpanded;
    final disabled = widget.isDisabled;

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // `.disclosure__trigger` — inline-block button, `status-focused`
        // ring on focus-visible, `status-disabled` when disabled.
        Opacity(
          opacity: disabled
              ? HeroTokens.doubleDisabledOpacity.resolve(context)
              : 1.0,
          child: HeroFocusRing(
            radius: HeroTokens.radiusMd.resolve(context).x,
            builder: (context, node, focused) => MouseRegion(
              cursor: disabled
                  ? SystemMouseCursors.basic
                  : SystemMouseCursors.click,
              child: Focus(
                focusNode: node,
                canRequestFocus: !disabled,
                onKeyEvent: disabled
                    ? null
                    : (node, event) {
                        if (event is KeyDownEvent &&
                            (event.logicalKey == LogicalKeyboardKey.enter ||
                                event.logicalKey ==
                                    LogicalKeyboardKey.space)) {
                          _toggle();
                          return KeyEventResult.handled;
                        }
                        return KeyEventResult.ignored;
                      },
                child: GestureDetector(
                  behavior: HitTestBehavior.opaque,
                  onTap: disabled ? null : _toggle,
                  child: Row(
                    mainAxisSize: MainAxisSize.max,
                    children: [
                      Flexible(
                        child: Text(
                          widget.title,
                          style: TextStyle(
                            fontSize:
                                HeroTokens.typeSm.resolve(context).fontSize,
                            color: HeroTokens.colorForeground.resolve(context),
                          ),
                        ),
                      ),
                      // `.disclosure__indicator` — `ms-auto size-4
                      // text-inherit`, rotates -180deg on expand.
                      AnimatedRotation(
                        turns: expanded ? -0.5 : 0,
                        duration: HeroMotion.durationOf(
                          context,
                          const Duration(milliseconds: 250),
                        ),
                        curve: heroEaseSmooth,
                        child: Icon(
                          Icons.expand_more,
                          size: HeroTokens.space4.resolve(context),
                          color: HeroTokens.colorForeground.resolve(context),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
        // `.disclosure__content` — height 200ms `--ease-out-quad`, opacity
        // 200ms `--ease-out`; `.disclosure__body` is `p-2`.
        AnimatedCrossFade(
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: 200),
          ),
          firstCurve: heroEaseOut,
          secondCurve: heroEaseOut,
          sizeCurve: _easeOutQuad,
          firstChild: const SizedBox(width: double.infinity),
          secondChild: Padding(
            padding: EdgeInsets.all(HeroTokens.space2.resolve(context)),
            child: widget.child,
          ),
          crossFadeState: expanded
              ? CrossFadeState.showSecond
              : CrossFadeState.showFirst,
        ),
      ],
    );
  }
}

import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_button.dart';

/// HeroUI v3 button group orientations (button-group.css
/// `.button-group--horizontal` / `--vertical`).
enum HeroButtonGroupOrientation { horizontal, vertical }

/// A HeroUI v3 button group (button-group.css `.button-group`) — joins
/// [HeroButton] children into a single rounded cluster: all buttons lose
/// their radius, the outer edges keep `rounded-3xl`, and optional 1px
/// separators (`bg-current opacity-15`) divide the buttons.
///
/// ```dart
/// HeroButtonGroup(
///   showSeparators: true,
///   children: [
///     HeroButton(label: 'Cut', variant: HeroButtonVariant.secondary),
///     HeroButton(label: 'Copy', variant: HeroButtonVariant.secondary),
///     HeroButton(label: 'Paste', variant: HeroButtonVariant.secondary),
///   ],
/// )
/// ```
class HeroButtonGroup extends StatefulWidget {
  const HeroButtonGroup({
    super.key,
    required this.children,
    this.orientation = HeroButtonGroupOrientation.horizontal,
    this.fullWidth = false,
    this.showSeparators = false,
  });

  final List<HeroButton> children;

  final HeroButtonGroupOrientation orientation;

  /// `.button-group--full-width` — the group stretches to the available
  /// width.
  final bool fullWidth;

  /// Inserts `.button-group__separator` dividers between the buttons.
  final bool showSeparators;

  @override
  State<HeroButtonGroup> createState() => _HeroButtonGroupState();
}

class _HeroButtonGroupState extends State<HeroButtonGroup> {
  final GlobalKey _measureKey = GlobalKey();
  Size? _size;

  @override
  Widget build(BuildContext context) {
    final horizontal =
        widget.orientation == HeroButtonGroupOrientation.horizontal;
    final radius = HeroTokens.radius3xl.resolve(context).x;
    final separatorColor = HeroTokens.colorDefaultForeground
        .resolve(context)
        .withValues(alpha: 0.15);

    // `.button-group__separator` — `rounded-sm bg-current opacity-15`;
    // horizontal: `w-px h-1/2`, vertical: `h-px w-1/2` of the group's
    // measured cross extent (measured below, second frame).
    Widget separator() {
      final cross = horizontal
          ? (_size?.height ?? 0) * 0.5
          : (_size?.width ?? 0) * 0.5;
      return SizedBox(
        width: horizontal ? 1 : cross,
        height: horizontal ? cross : 1,
        child: DecoratedBox(
          decoration: BoxDecoration(
            color: separatorColor,
            borderRadius: BorderRadius.circular(
              HeroTokens.radiusSm.resolve(context).x,
            ),
          ),
        ),
      );
    }

    Widget slot(int i) =>
        _buttonSlot(i, radius: radius, horizontal: horizontal);

    // `.button-group` — `inline-flex h-auto items-center justify-center
    // gap-0`, `--horizontal` flex-row / `--vertical` flex-col.
    final content = KeyedSubtree(
      key: _measureKey,
      child: Flex(
        direction: horizontal ? Axis.horizontal : Axis.vertical,
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: [
          for (var i = 0; i < widget.children.length; i++) ...[
            if (i > 0 && widget.showSeparators) separator(),
            slot(i),
          ],
        ],
      ),
    );

    WidgetsBinding.instance.addPostFrameCallback((_) {
      final box = _measureKey.currentContext?.findRenderObject() as RenderBox?;
      if (box != null && box.hasSize && box.size != _size) {
        setState(() => _size = box.size);
      }
    });

    if (!widget.fullWidth) return content;
    // `.button-group--full-width` — `w-full` on the container; the buttons
    // keep their natural size (the CSS only widens the group).
    return SizedBox(
      width: double.infinity,
      child: Align(alignment: Alignment.center, child: content),
    );
  }

  /// Rounds only the outer corners of the first/last button (the spec's
  /// `rounded-s-3xl` / `rounded-e-3xl`, `rounded-none` elsewhere) by
  /// clipping each button to its slot silhouette.
  Widget _buttonSlot(
    int index, {
    required double radius,
    required bool horizontal,
  }) {
    final button = widget.children[index];
    final first = index == 0;
    final last = index == widget.children.length - 1;
    final single = widget.children.length == 1;

    final clipRadius = single
        ? BorderRadius.circular(radius)
        : horizontal
            ? BorderRadius.horizontal(
                left: Radius.circular(first ? radius : 0),
                right: Radius.circular(last ? radius : 0),
              )
            : BorderRadius.vertical(
                top: Radius.circular(first ? radius : 0),
                bottom: Radius.circular(last ? radius : 0),
              );

    return ClipRRect(borderRadius: clipRadius, child: button);
  }
}

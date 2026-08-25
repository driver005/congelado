import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_header.dart';

/// A HeroUI v3 list box (list-box.css `.list-box`) — a vertical list of
/// [HeroListBoxItem]s, optionally grouped into [HeroListBoxSection]s.
///
/// list-box.css: `.list-box` — `relative w-full overflow-clip p-1` (4);
/// direct children get `mt-1` (4). Inside popovers the padding is overridden
/// (`p-1.5` = 6 in the select popover).
class HeroListBox extends StatelessWidget {
  const HeroListBox({
    super.key,
    required this.children,
    this.padding = 4,
    this.itemPaddingX,
  });

  /// The items/sections/separators.
  final List<Widget> children;

  /// Container padding (`.list-box` `p-1` = 4; popovers use 6).
  final double padding;

  /// Optional horizontal padding override for items (select/combo-box popovers
  /// use `px-2.5` = 10 instead of the base `px-2` = 8).
  final double? itemPaddingX;

  @override
  Widget build(BuildContext context) {
    return ClipRect(
      child: Padding(
        padding: EdgeInsets.all(padding),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            for (var i = 0; i < children.length; i++) ...[
              if (i > 0)
                SizedBox(height: HeroTokens.space1.resolve(context)),
              _withItemPadding(children[i]),
            ],
          ],
        ),
      ),
    );
  }

  Widget _withItemPadding(Widget child) {
    final x = itemPaddingX;
    if (x == null || child is! HeroListBoxItem) return child;
    return HeroListBoxItem(
      key: child.key,
      label: child.label,
      description: child.description,
      leading: child.leading,
      selected: child.selected,
      danger: child.danger,
      disabled: child.disabled,
      onPressed: child.onPressed,
      paddingX: x,
    );
  }
}

/// A HeroUI v3 list-box item (list-box-item.css `.list-box-item`).
///
/// `.list-box-item` — `relative flex min-h-9 w-full items-center justify-start
/// gap-3 rounded-2xl px-2 py-1.5`; hover `bg-default`; pressed `scale(0.98)`;
/// focus-visible `status-focused`; the selected state paints a checkmark
/// indicator (`size-4`, check `size-2.5`, `text-default-foreground`) at the
/// inline end (`end-2`, `pe-7`); `--danger` recolors the label + indicator to
/// danger. Disabled fades to `--disabled-opacity`.
class HeroListBoxItem extends StatefulWidget {
  const HeroListBoxItem({
    super.key,
    required this.label,
    this.description,
    this.leading,
    this.selected = false,
    this.danger = false,
    this.disabled = false,
    this.onPressed,
    this.paddingX,
  });

  /// The item label (`.list-box-item [data-slot="label"]`).
  final String label;

  /// Optional secondary line (`[data-slot="description"]`, `text-xs muted`).
  final String? description;

  /// Optional leading widget (avatar, icon, …).
  final Widget? leading;

  /// Selected state — shows the checkmark indicator.
  final bool selected;

  /// Danger variant — label + indicator in the danger color.
  final bool danger;

  final bool disabled;

  /// Invoked when the item is activated.
  final VoidCallback? onPressed;

  /// Horizontal padding override (popovers use `px-2.5` = 10).
  final double? paddingX;

  @override
  State<HeroListBoxItem> createState() => _HeroListBoxItemState();
}

class _HeroListBoxItemState extends State<HeroListBoxItem> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onPressed != null;
    final radius = HeroTokens.radius2xl.resolve(context).x;
    final foreground = widget.danger
        ? HeroTokens.colorDanger.resolve(context)
        : HeroTokens.colorDefaultForeground.resolve(context);
    final opacity =
        widget.disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

    final content = AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 100),
      ),
      curve: HeroMotion.out,
      constraints: const BoxConstraints(minHeight: 36),
      padding: EdgeInsets.symmetric(
        horizontal: widget.paddingX ?? HeroTokens.space2.resolve(context),
        vertical: HeroTokens.space15.resolve(context),
      ),
      decoration: BoxDecoration(
        color: _hovered && enabled
            ? HeroTokens.colorDefault.resolve(context)
            : HeroTokens.colorTransparent.resolve(context),
        borderRadius: BorderRadius.circular(radius),
      ),
      child: Row(
        // max: the item fills the list-box width (the popup is pinned to the
        // trigger width). A min-sized row would leave the rest of the panel
        // unpainted — the "black" gap next to the label in dark themes.
        mainAxisSize: MainAxisSize.max,
        children: [
          if (widget.leading != null) ...[
            widget.leading!,
            SizedBox(width: HeroTokens.space3.resolve(context)),
          ],
          Flexible(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  widget.label,
                  style: TextStyle(
                    fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                    fontWeight: HeroTokens.weightMedium.resolve(context),
                    color: foreground,
                  ),
                ),
                if (widget.description != null)
                  Text(
                    widget.description!,
                    style: TextStyle(
                      fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                      color: HeroTokens.colorMuted.resolve(context),
                    ),
                  ),
              ],
            ),
          ),
        ],
      ),
    );

    return HeroFocusRing(
      radius: radius,
      builder: (context, node, focused) => Opacity(
        opacity: opacity,
        child: MouseRegion(
          cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
          onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
          onExit: enabled ? (_) => setState(() => _hovered = false) : null,
          child: Focus(
            focusNode: node,
            child: GestureDetector(
              onTap: enabled ? widget.onPressed : null,
              onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
              onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
              onTapCancel: enabled ? () => setState(() => _pressed = false) : null,
              child: AnimatedScale(
                scale: _pressed ? 0.98 : 1.0,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
                ),
                curve: heroEaseOutQuart,
                child: Stack(
                  children: [
                    content,
                    if (widget.selected)
                      Align(
                        alignment: Alignment.centerRight,
                        child: Padding(
                          padding: EdgeInsets.only(
                            right: HeroTokens.space2.resolve(context),
                          ),
                          child: Icon(
                            Icons.check_rounded,
                            size: 16,
                            color: foreground,
                          ),
                        ),
                      ),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// A HeroUI v3 list-box section (list-box-section.css `.list-box-section`) —
/// a column of items with an optional [HeroHeader] caption.
///
/// list-box-section.css: `.list-box-section` — `flex flex-col items-start
/// gap-0`.
class HeroListBoxSection extends StatelessWidget {
  const HeroListBoxSection({
    super.key,
    this.header,
    required this.children,
  });

  /// The section caption ([HeroHeader], `text-xs font-medium text-muted`).
  final String? header;

  final List<Widget> children;

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (header != null) HeroHeader(header!),
        ...children,
      ],
    );
  }
}

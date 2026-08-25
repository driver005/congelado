import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_header.dart';
import 'hero_popup.dart';

/// A HeroUI v3 menu (menu.css `.menu` + dropdown.css `.dropdown`) — an
/// anchored popup list of [HeroMenuItem]s, opened by [trigger].
///
/// The popover shell mirrors `.dropdown__popover`: `bg-overlay`, radius
/// `min(32px, var(--radius-3xl))` (24), `shadow-overlay`, `min-w-55` (220),
/// entering `fade-in-0 zoom-in-95` 150ms; the inner `.menu` is
/// `flex flex-col gap-1 p-1`.
class HeroMenu extends StatelessWidget {
  const HeroMenu({
    super.key,
    required this.trigger,
    required this.items,
    this.controller,
    this.minWidth,
  });

  /// The trigger widget (`DropdownButton`-like); tap toggles the menu.
  final Widget trigger;

  /// The menu items / sections.
  final List<Widget> items;

  /// Optional controller for programmatic open/close.
  final HeroPopupController? controller;

  /// Optional minimum popup width (defaults to `min-w-55` = 220).
  final double? minWidth;

  @override
  Widget build(BuildContext context) {
    final overlayShadow = HeroTokens.shadowOverlay.resolve(context);
    final width = MediaQuery.sizeOf(context).width;

    return HeroPopup(
      controller: controller,
      minWidth: minWidth ?? 220,
      child: trigger,
      builder: (context) {
        return Container(
          constraints: BoxConstraints(maxWidth: width * 0.48),
          decoration: BoxDecoration(
            color: HeroTokens.colorOverlay.resolve(context),
            borderRadius: BorderRadius.circular(
              HeroTokens.radius3xl.resolve(context).x,
            ),
            boxShadow: overlayShadow,
          ),
          child: Padding(
            padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                for (var i = 0; i < items.length; i++) ...[
                  if (i > 0)
                    SizedBox(height: HeroTokens.space1.resolve(context)),
                  items[i],
                ],
              ],
            ),
          ),
        );
      },
    );
  }
}

/// HeroUI v3 menu selection modes (menu-item.css
/// `.menu-item[data-selection-mode=...]`).
enum HeroMenuSelectionMode {
  /// No indicator.
  none,

  /// Dot indicator (`size-2`, hidden + scaled 0.7 until selected).
  single,

  /// Checkmark indicator (`size-2.5`).
  multiple,
}

/// A HeroUI v3 menu item (menu-item.css `.menu-item`).
///
/// Same base anatomy as `.list-box-item` (`min-h-9 rounded-2xl px-2 py-1.5
/// gap-3`, hover `bg-default`, pressed `scale(0.98)`, focus-visible
/// `status-focused`) with the indicator on the inline START (`start-2`,
/// `ps-7`): a dot for single selection, a checkmark for multiple selection,
/// both `text-muted` (danger: `text-danger`). Submenu items get a chevron at
/// the inline end (`size-3.5`).
class HeroMenuItem extends StatefulWidget {
  const HeroMenuItem({
    super.key,
    required this.label,
    this.description,
    this.leading,
    this.selected = false,
    this.selectionMode = HeroMenuSelectionMode.none,
    this.danger = false,
    this.disabled = false,
    this.onPressed,
    this.hasSubmenu = false,
  });

  /// The item label.
  final String label;

  /// Optional secondary line (`[data-slot="description"]`, `text-xs muted`).
  final String? description;

  /// Optional leading widget.
  final Widget? leading;

  /// Selected state — shows the selection indicator.
  final bool selected;

  final HeroMenuSelectionMode selectionMode;

  /// Danger variant — label + indicator in the danger color.
  final bool danger;

  final bool disabled;

  /// Invoked when the item is activated.
  final VoidCallback? onPressed;

  /// Shows the submenu chevron at the inline end.
  final bool hasSubmenu;

  @override
  State<HeroMenuItem> createState() => _HeroMenuItemState();
}

class _HeroMenuItemState extends State<HeroMenuItem> {
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
    final showIndicator =
        widget.selectionMode != HeroMenuSelectionMode.none || widget.hasSubmenu;

    Widget? indicator;
    if (widget.selectionMode == HeroMenuSelectionMode.single) {
      indicator = AnimatedScale(
        scale: widget.selected ? 1.0 : 0.7,
        duration: HeroMotion.durationOf(
          context,
          const Duration(milliseconds: 250),
        ),
        curve: HeroMotion.smooth,
        child: AnimatedOpacity(
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: 250),
          ),
          curve: HeroMotion.smooth,
          opacity: widget.selected ? 1 : 0,
          child: Container(
            width: 8,
            height: 8,
            decoration: BoxDecoration(
              color: foreground,
              shape: BoxShape.circle,
            ),
          ),
        ),
      );
    } else if (widget.selectionMode == HeroMenuSelectionMode.multiple) {
      indicator = Icon(
        widget.selected ? Icons.check_rounded : Icons.check_rounded,
        size: 10,
        color: widget.selected ? foreground : HeroTokens.colorTransparent.resolve(context),
      );
    }

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
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 100),
                  ),
                  curve: HeroMotion.out,
                  constraints: const BoxConstraints(minHeight: 36),
                  padding: EdgeInsets.symmetric(
                    horizontal: HeroTokens.space2.resolve(context),
                    vertical: HeroTokens.space15.resolve(context),
                  ),
                  decoration: BoxDecoration(
                    color: _hovered && enabled
                        ? HeroTokens.colorDefault.resolve(context)
                        : HeroTokens.colorTransparent.resolve(context),
                    borderRadius: BorderRadius.circular(radius),
                  ),
                  child: Stack(
                    children: [
                      Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          if (showIndicator) ...[
                            SizedBox(
                              width: 16,
                              child: Center(child: indicator),
                            ),
                            SizedBox(width: HeroTokens.space05.resolve(context)),
                          ],
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
                                    fontSize:
                                        HeroTokens.typeSm.resolve(context).fontSize,
                                    fontWeight:
                                        HeroTokens.weightMedium.resolve(context),
                                    color: foreground,
                                  ),
                                ),
                                if (widget.description != null)
                                  Text(
                                    widget.description!,
                                    style: TextStyle(
                                      fontSize:
                                          HeroTokens.typeXs.resolve(context).fontSize,
                                      color: HeroTokens.colorMuted.resolve(context),
                                    ),
                                  ),
                              ],
                            ),
                          ),
                          if (widget.hasSubmenu) ...[
                            const Spacer(),
                            Icon(
                              Icons.chevron_right_rounded,
                              size: 14,
                              color: HeroTokens.colorMuted.resolve(context),
                            ),
                          ],
                        ],
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// A HeroUI v3 menu section (menu-section.css `.menu-section`) — a column of
/// [HeroMenuItem]s with an optional [HeroHeader] caption.
///
/// menu-section.css: `.menu-section` — `flex flex-col items-start gap-0`.
class HeroMenuSection extends StatelessWidget {
  const HeroMenuSection({
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

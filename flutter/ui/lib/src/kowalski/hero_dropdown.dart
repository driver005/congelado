import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_popover.dart';

/// One entry in a [HeroDropdown] menu.
sealed class HeroDropdownEntry {
  const HeroDropdownEntry();
}

/// A dropdown menu item (menu-item.css `.menu-item`) — a `min-h-9` row with
/// a `rounded-2xl` hover fill, focus ring, 0.98 press scale and an optional
/// leading icon / description / trailing icon.
class HeroDropdownItem extends HeroDropdownEntry {
  const HeroDropdownItem({
    required this.label,
    this.onPressed,
    this.leadingIcon,
    this.trailingIcon,
    this.description,
    this.danger = false,
    this.enabled = true,
    this.selected = false,
    this.closeOnSelect = true,
  });

  final String label;
  final VoidCallback? onPressed;
  final IconData? leadingIcon;
  final IconData? trailingIcon;
  final String? description;

  /// `.menu-item--danger` — label and indicator in `text-danger`.
  final bool danger;

  final bool enabled;

  /// Renders the check indicator (`.menu-item__indicator`) at the start.
  final bool selected;

  /// Whether tapping the item also closes the menu.
  final bool closeOnSelect;
}

/// A thin divider between menu items (`[data-slot="separator"]` —
/// `ms-[3%] w-[94%]`).
class HeroDropdownSeparator extends HeroDropdownEntry {
  const HeroDropdownSeparator();
}

/// A HeroUI v3 dropdown (dropdown.css) — a button trigger that opens an
/// anchored menu popover (`.dropdown__popover` — `bg-overlay shadow-overlay`,
/// radius 24, `max-w-[48svw] md:min-w-55`).
///
/// ```dart
/// HeroDropdown(
///   trigger: (open) => HeroButton(
///     label: 'Actions',
///     variant: HeroButtonVariant.secondary,
///     onPressed: open,
///   ),
///   items: [
///     HeroDropdownItem(label: 'Edit', leadingIcon: Icons.edit_rounded),
///     HeroDropdownItem(label: 'Delete', danger: true, onPressed: () {}),
///   ],
/// )
/// ```
class HeroDropdown extends StatefulWidget {
  const HeroDropdown({
    super.key,
    required this.trigger,
    required this.items,
    this.placement = HeroPopoverPlacement.bottom,
    this.alignment = HeroPopoverAlignment.start,
    this.gap = 4,
  });

  /// Builds the anchor widget (`.dropdown__trigger`). Wire its press handler
  /// to [VoidCallback] to open the menu.
  final Widget Function(VoidCallback open) trigger;

  /// The menu entries.
  final List<HeroDropdownEntry> items;

  final HeroPopoverPlacement placement;
  final HeroPopoverAlignment alignment;
  final double gap;

  @override
  State<HeroDropdown> createState() => _HeroDropdownState();
}

class _HeroDropdownState extends State<HeroDropdown> {
  final GlobalKey _triggerKey = GlobalKey();

  void _open() {
    final box =
        _triggerKey.currentContext?.findRenderObject() as RenderBox?;
    if (box == null || !box.hasSize) return;
    final anchor = box.localToGlobal(Offset.zero) & box.size;
    showHeroDropdown<void>(
      context,
      anchor: anchor,
      items: widget.items,
      placement: widget.placement,
      alignment: widget.alignment,
      gap: widget.gap,
    );
  }

  @override
  Widget build(BuildContext context) {
    return KeyedSubtree(
      key: _triggerKey,
      child: widget.trigger(_open),
    );
  }
}

/// Shows a HeroUI v3 dropdown menu anchored to [anchor] (a rect in global
/// coordinates). The menu closes on outside tap / item tap (see
/// [HeroDropdownItem.closeOnSelect]).
Future<T?> showHeroDropdown<T>(
  BuildContext context, {
  required Rect anchor,
  required List<HeroDropdownEntry> items,
  HeroPopoverPlacement placement = HeroPopoverPlacement.bottom,
  HeroPopoverAlignment alignment = HeroPopoverAlignment.start,
  double gap = 4,
  bool barrierDismissible = true,
}) {
  return showHeroPopover<T>(
    context,
    anchor: anchor,
    placement: placement,
    alignment: alignment,
    showArrow: false,
    gap: gap,
    barrierDismissible: barrierDismissible,
    barrierLabel: 'Dismiss dropdown',
    // The menu is as wide as the trigger (`min-w-(--trigger-width)`): pin
    // both bounds to the anchor width so the menu never outgrows the input
    // (a stretch Column inside a loose popover would paint a full-width
    // panel). Long labels wrap instead of widening the menu.
    minWidth: anchor.width,
    maxWidth: anchor.width,
    builder: (context) => _HeroDropdownMenu(items: items),
  );
}

class _HeroDropdownMenu extends StatelessWidget {
  const _HeroDropdownMenu({required this.items});

  final List<HeroDropdownEntry> items;

  @override
  Widget build(BuildContext context) {
    // `[data-slot="dropdown-menu"]` — `p-1.5 outline-none`; the menu list
    // itself is `gap-0.5` (`dropdown.css .dropdown__menu`).
    return Padding(
      padding: const EdgeInsets.all(6),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          for (final entry in items)
            switch (entry) {
              HeroDropdownItem() => Padding(
                  padding: const EdgeInsets.symmetric(vertical: 1),
                  child: _HeroDropdownItemView(item: entry),
                ),
              HeroDropdownSeparator() => const _HeroDropdownSeparatorView(),
            },
        ],
      ),
    );
  }
}

class _HeroDropdownSeparatorView extends StatelessWidget {
  const _HeroDropdownSeparatorView();

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        final width = constraints.maxWidth;
        return Align(
          alignment: Alignment.centerLeft,
          child: Container(
            height: 1,
            width: width * 0.94,
            margin: EdgeInsets.only(left: width * 0.03),
            color: HeroTokens.colorSeparator.resolve(context),
          ),
        );
      },
    );
  }
}

class _HeroDropdownItemView extends StatefulWidget {
  const _HeroDropdownItemView({required this.item});

  final HeroDropdownItem item;

  @override
  State<_HeroDropdownItemView> createState() => _HeroDropdownItemViewState();
}

class _HeroDropdownItemViewState extends State<_HeroDropdownItemView> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final item = widget.item;
    // Items without a handler stay visually enabled; tapping only closes
    // the menu (`status-disabled` is driven by `enabled` alone).
    final enabled = item.enabled;
    final labelColor = item.danger
        ? HeroTokens.colorDanger.resolve(context)
        : HeroTokens.colorForeground.resolve(context);

    // `.menu-item` — `min-h-9 w-full gap-3 rounded-2xl px-2 py-1.5`;
    // dropdown items override `px-2.5` (10).
    final row = Row(
      children: [
        if (item.leadingIcon != null) ...[
          Icon(
            item.leadingIcon,
            size: HeroTokens.space4.resolve(context),
            color: labelColor,
          ),
          const SizedBox(width: 12), // gap-3
        ],
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                item.label,
                style: TextStyle(
                  fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                  color: labelColor,
                ),
              ),
              if (item.description != null)
                Text(
                  item.description!,
                  style: TextStyle(
                    fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                    color: HeroTokens.colorMuted.resolve(context),
                  ),
                ),
            ],
          ),
        ),
        if (item.trailingIcon != null) ...[
          const SizedBox(width: 12),
          Icon(
            item.trailingIcon,
            size: HeroTokens.space4.resolve(context),
            color: HeroTokens.colorMuted.resolve(context),
          ),
        ],
      ],
    );

    final content = Opacity(
      // `status-disabled` — opacity var(--disabled-opacity).
      opacity: enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: HeroFocusRing(
        radius: HeroTokens.radius2xl.resolve(context).x,
        builder: (context, node, focused) => MouseRegion(
          cursor:
              enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
          onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
          onExit: enabled ? (_) => setState(() => _hovered = false) : null,
          child: Focus(
            focusNode: node,
            canRequestFocus: enabled,
            onKeyEvent: enabled
                ? (node, event) {
                    if (event is KeyDownEvent &&
                        (event.logicalKey == LogicalKeyboardKey.enter ||
                            event.logicalKey == LogicalKeyboardKey.space)) {
                      _activate();
                      return KeyEventResult.handled;
                    }
                    return KeyEventResult.ignored;
                  }
                : null,
            child: GestureDetector(
              behavior: HitTestBehavior.opaque,
              onTap: enabled ? _activate : null,
              onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
              onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
              onTapCancel:
                  enabled ? () => setState(() => _pressed = false) : null,
              child: AnimatedScale(
                // `&:active` — scale(0.98) over 250ms `--ease-out-quart`.
                scale: _pressed ? 0.98 : 1,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
                ),
                curve: heroEaseOutQuart,
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 150),
                  ),
                  curve: heroEaseOut,
                  constraints: const BoxConstraints(minHeight: 36),
                  padding: const EdgeInsets.symmetric(
                    horizontal: 10,
                    vertical: 6,
                  ),
                  decoration: BoxDecoration(
                    // `&:hover` — `bg-default`.
                    color: _hovered
                        ? HeroTokens.colorDefault.resolve(context)
                        : HeroTokens.colorTransparent.resolve(context),
                    borderRadius: BorderRadius.circular(
                      HeroTokens.radius2xl.resolve(context).x,
                    ),
                  ),
                  child: row,
                ),
              ),
            ),
          ),
        ),
      ),
    );

    // Selected items get the check indicator (`menu-item__indicator` +
    // `ps-7`) at the inline start. Align + Padding instead of a stretching
    // Positioned: the item lives inside an unbounded popup Column, and a
    // Positioned with top/bottom set would demand bounded Stack height.
    if (item.selected) {
      return Stack(
        children: [
          Padding(padding: const EdgeInsets.only(left: 28), child: content),
          Align(
            alignment: Alignment.centerLeft,
            child: Padding(
              padding: const EdgeInsets.only(left: 8),
              child: Icon(
                Icons.check_rounded,
                size: 10, // size-2.5
                color: item.danger
                    ? HeroTokens.colorDanger.resolve(context)
                    : HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
        ],
      );
    }
    return content;
  }

  void _activate() {
    final item = widget.item;
    item.onPressed?.call();
    if (item.closeOnSelect) {
      Navigator.of(context).pop();
    }
  }
}

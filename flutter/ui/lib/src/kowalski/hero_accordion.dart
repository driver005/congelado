import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// `--ease-out-quad` (shared-theme.css) — the accordion panel height curve
/// (accordion.css `.accordion__panel` / disclosure.css `.disclosure__content`).
const Cubic _easeOutQuad = Cubic(0.25, 0.46, 0.45, 0.94);

/// HeroUI v3 accordion variants (accordion.css `.accordion--surface`).
enum HeroAccordionVariant {
  /// Bare list — separators only (default).
  light,

  /// Filled `bg-surface` container with `--radius-3xl` corners.
  surface,
}

/// One accordion entry (accordion.css `.accordion__item`).
class HeroAccordionItem {
  const HeroAccordionItem({
    required this.title,
    required this.child,
    this.value,
    this.hideSeparator = false,
    this.disabled = false,
  });

  /// The trigger title (`text-sm font-medium`).
  final String title;

  /// The panel content (`.accordion__body-inner` — `px-4 pt-0 pb-4 text-muted`).
  final Widget child;

  /// Stable identity for expansion tracking. Defaults to the item's index.
  final String? value;

  /// Suppresses this item's separator (`data-hide-separator="true"`).
  final bool hideSeparator;

  /// Disables this item's trigger.
  final bool disabled;
}

/// A HeroUI v3 accordion (accordion.css) — a vertically stacked set of
/// expandable items with a chevron trigger and a height-animated panel.
///
/// ```dart
/// HeroAccordion(
///   variant: HeroAccordionVariant.surface,
///   items: [
///     HeroAccordionItem(title: 'Item 1', child: const Text('...')),
///     HeroAccordionItem(title: 'Item 2', child: const Text('...')),
///   ],
/// )
/// ```
class HeroAccordion extends StatefulWidget {
  const HeroAccordion({
    super.key,
    required this.items,
    this.variant = HeroAccordionVariant.light,
    this.expandedKeys,
    this.defaultExpandedKeys = const {},
    this.onExpandedChange,
    this.allowsMultipleExpanded = true,
    this.hideSeparator = false,
    this.isDisabled = false,
  });

  final List<HeroAccordionItem> items;
  final HeroAccordionVariant variant;

  /// Controlled expansion set. When null the accordion tracks its own state
  /// (seeded from [defaultExpandedKeys]).
  final Set<String>? expandedKeys;

  /// Keys expanded on first build when [expandedKeys] is null.
  final Set<String> defaultExpandedKeys;

  final ValueChanged<Set<String>>? onExpandedChange;

  /// When false, opening one item collapses the others (react-aria
  /// DisclosureGroup `allowsMultipleExpanded`).
  final bool allowsMultipleExpanded;

  /// Hides the separator of every item.
  final bool hideSeparator;

  /// Disables every trigger in the group.
  final bool isDisabled;

  @override
  State<HeroAccordion> createState() => _HeroAccordionState();
}

class _HeroAccordionState extends State<HeroAccordion> {
  late Set<String> _expanded;

  @override
  void initState() {
    super.initState();
    _expanded = {...widget.defaultExpandedKeys};
  }

  Set<String> get _keys {
    final controlled = widget.expandedKeys;
    return controlled ?? _expanded;
  }

  void _toggle(String key) {
    final current = _keys;
    final expanded = current.contains(key);
    final next = {...current};
    if (expanded) {
      next.remove(key);
    } else {
      if (!widget.allowsMultipleExpanded) {
        next.clear();
      }
      next.add(key);
    }
    if (widget.expandedKeys == null) {
      setState(() => _expanded = next);
    }
    widget.onExpandedChange?.call(next);
  }

  @override
  Widget build(BuildContext context) {
    final surface = widget.variant == HeroAccordionVariant.surface;
    final radius = HeroTokens.radius3xl.resolve(context).x;

    final container = Container(
      decoration: BoxDecoration(
        color: surface
            ? HeroTokens.colorSurface.resolve(context)
            : HeroTokens.colorTransparent.resolve(context),
        borderRadius: BorderRadius.circular(surface ? radius : 0),
      ),
      clipBehavior: surface ? Clip.antiAlias : Clip.none,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          for (var i = 0; i < widget.items.length; i++) ...[
            _HeroAccordionItemView(
              key: ValueKey('hero-accordion-item-$i'),
              item: widget.items[i],
              index: i,
              isLast: i == widget.items.length - 1,
              surface: surface,
              radius: radius,
              expanded: _keys.contains(
                widget.items[i].value ?? '$i',
              ),
              onToggle: () => _toggle(widget.items[i].value ?? '$i'),
              isDisabled: widget.isDisabled || widget.items[i].disabled,
              hideSeparator:
                  widget.hideSeparator || widget.items[i].hideSeparator,
            ),
          ],
        ],
      ),
    );
    return container;
  }
}

class _HeroAccordionItemView extends StatefulWidget {
  const _HeroAccordionItemView({
    super.key,
    required this.item,
    required this.index,
    required this.isLast,
    required this.surface,
    required this.radius,
    required this.expanded,
    required this.onToggle,
    required this.isDisabled,
    required this.hideSeparator,
  });

  final HeroAccordionItem item;
  final int index;
  final bool isLast;
  final bool surface;
  final double radius;
  final bool expanded;
  final VoidCallback onToggle;
  final bool isDisabled;
  final bool hideSeparator;

  @override
  State<_HeroAccordionItemView> createState() => _HeroAccordionItemViewState();
}

class _HeroAccordionItemViewState extends State<_HeroAccordionItemView> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final item = widget.item;
    final disabled = widget.isDisabled;

    // `.accordion__trigger` — `px-4 py-4`, `text-sm font-medium`, rounded
    // to the container in the surface variant for the first/last items.
    final topRounded = widget.surface && widget.index == 0;
    final bottomRounded =
        widget.surface && widget.isLast && !widget.expanded;

    final triggerBackground = !disabled &&
            _hovered &&
            !widget.expanded
        ? (widget.surface
            ? HeroTokens.colorDefault.resolve(context)
            : HeroTokens.colorForeground.resolve(context).withValues(alpha: 0.03))
        : HeroTokens.colorTransparent.resolve(context);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Opacity(
          // HeroUI disabled state: `status-disabled` = `opacity:
          // var(--disabled-opacity)` (0.5).
          opacity: disabled
              ? HeroTokens.doubleDisabledOpacity.resolve(context)
              : 1.0,
          child: HeroFocusRing(
            // The ring follows the trigger's corner radius: rounded only in
            // the surface variant (first/last item edges).
            radius: widget.surface ? widget.radius : 0,
            builder: (context, node, focused) => MouseRegion(
              cursor: disabled
                  ? SystemMouseCursors.basic
                  : SystemMouseCursors.click,
              onEnter: disabled ? null : (_) => setState(() => _hovered = true),
              onExit: disabled ? null : (_) => setState(() => _hovered = false),
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
                          widget.onToggle();
                          return KeyEventResult.handled;
                        }
                        return KeyEventResult.ignored;
                      },
                child: GestureDetector(
                  behavior: HitTestBehavior.opaque,
                  onTap: disabled ? null : widget.onToggle,
                  child: Container(
                    padding: const EdgeInsets.symmetric(
                      horizontal: 16,
                      vertical: 16,
                    ),
                    decoration: BoxDecoration(
                      color: triggerBackground,
                      borderRadius: BorderRadius.vertical(
                        top: Radius.circular(topRounded ? widget.radius : 0),
                        bottom:
                            Radius.circular(bottomRounded ? widget.radius : 0),
                      ),
                    ),
                    child: Row(
                      children: [
                        Expanded(
                          child: Text(
                            item.title,
                            style: TextStyle(
                              fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                              fontWeight:
                                  HeroTokens.weightMedium.resolve(context),
                              color: HeroTokens.colorForeground.resolve(context),
                            ),
                          ),
                        ),
                        // `.accordion__indicator` — `size-4 text-muted`,
                        // rotates -180deg on expand (duration-250).
                        AnimatedRotation(
                          turns: widget.expanded ? -0.5 : 0,
                          duration: HeroMotion.durationOf(
                            context,
                            const Duration(milliseconds: 250),
                          ),
                          curve: heroEaseSmooth,
                          child: Icon(
                            Icons.expand_more,
                            size: HeroTokens.space4.resolve(context),
                            color: HeroTokens.colorMuted.resolve(context),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
        // `.accordion__panel` — height 200ms `--ease-out-quad`, opacity
        // 200ms `--ease-out`; the panel body is `px-4 pt-0 pb-4 text-muted`
        // inside `.accordion__body` (`text-sm`).
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
            padding: const EdgeInsets.fromLTRB(16, 0, 16, 16),
            child: DefaultTextStyle.merge(
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                color: HeroTokens.colorMuted.resolve(context),
              ),
              child: item.child,
            ),
          ),
          crossFadeState: widget.expanded
              ? CrossFadeState.showSecond
              : CrossFadeState.showFirst,
        ),
        // `.accordion__item::after` — separator under each item except the
        // last; the surface variant insets it to `start-[3%] w-[94%]` and
        // tints it `surface-foreground/6`.
        if (!widget.hideSeparator && !widget.isLast)
          LayoutBuilder(
            builder: (context, constraints) {
              final width = constraints.maxWidth;
              return Align(
                alignment: Alignment.centerLeft,
                child: Container(
                  height: 1,
                  width: widget.surface ? width * 0.94 : width,
                  margin: EdgeInsets.only(
                    left: widget.surface ? width * 0.03 : 0,
                  ),
                  color: widget.surface
                      ? HeroTokens.colorSurfaceForeground
                          .resolve(context)
                          .withValues(alpha: 0.06)
                      : HeroTokens.colorSeparator.resolve(context),
                ),
              );
            },
          ),
      ],
    );
  }
}

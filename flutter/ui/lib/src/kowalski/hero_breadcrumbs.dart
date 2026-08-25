import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// One breadcrumb entry (breadcrumbs.css `.breadcrumbs__link`).
class HeroBreadcrumbItem {
  const HeroBreadcrumbItem({
    required this.label,
    this.icon,
    this.onPressed,
    this.current,
  });

  final String label;

  /// Optional leading icon.
  final IconData? icon;

  /// Tap handler; when null the item is non-interactive text.
  final VoidCallback? onPressed;

  /// Marks the item as the current page. When null, the last item is
  /// considered current (`data-current`).
  final bool? current;
}

/// A HeroUI v3 breadcrumbs row (breadcrumbs.css) — items separated by a
/// chevron, hover underline on links, and the current page in `text-link`.
///
/// ```dart
/// HeroBreadcrumbs(items: [
///   HeroBreadcrumbItem(label: 'Home', onPressed: () {}),
///   HeroBreadcrumbItem(label: 'Components', onPressed: () {}),
///   HeroBreadcrumbItem(label: 'Breadcrumbs'),
/// ])
/// ```
class HeroBreadcrumbs extends StatelessWidget {
  const HeroBreadcrumbs({
    super.key,
    required this.items,
    this.separator,
  });

  final List<HeroBreadcrumbItem> items;

  /// Custom separator (defaults to a 12px muted chevron-right).
  final Widget? separator;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        for (var i = 0; i < items.length; i++) ...[
          if (i > 0)
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 2),
              child: separator ??
                  Icon(
                    Icons.chevron_right,
                    size: HeroTokens.space3.resolve(context),
                    color: HeroTokens.colorMuted.resolve(context),
                  ),
            ),
          _HeroBreadcrumbLink(
            item: items[i],
            // RAC Breadcrumb: the last item is the current page.
            current: items[i].current ?? (i == items.length - 1),
          ),
        ],
      ],
    );
  }
}

class _HeroBreadcrumbLink extends StatefulWidget {
  const _HeroBreadcrumbLink({required this.item, required this.current});

  final HeroBreadcrumbItem item;
  final bool current;

  @override
  State<_HeroBreadcrumbLink> createState() => _HeroBreadcrumbLinkState();
}

class _HeroBreadcrumbLinkState extends State<_HeroBreadcrumbLink> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final item = widget.item;
    final interactive = item.onPressed != null;
    // `.breadcrumbs__item` — `flex shrink-0 items-center justify-center
    // gap-0.5 px-0.5`; the link is `relative px-0.5 text-sm leading-5
    // font-medium text-muted` with `text-link` when current.
    final color = widget.current
        ? HeroTokens.colorLink.resolve(context)
        : HeroTokens.colorMuted.resolve(context);

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 2),
      child: MouseRegion(
        cursor:
            interactive ? SystemMouseCursors.click : SystemMouseCursors.basic,
        onEnter: interactive ? (_) => setState(() => _hovered = true) : null,
        onExit: interactive ? (_) => setState(() => _hovered = false) : null,
        child: GestureDetector(
          behavior: HitTestBehavior.opaque,
          onTap: interactive ? item.onPressed : null,
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 2),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                if (item.icon != null) ...[
                  Icon(
                    item.icon,
                    size: HeroTokens.space4.resolve(context),
                    color: color,
                  ),
                  const SizedBox(width: 2), // gap-0.5
                ],
                Text(
                  item.label,
                  style: TextStyle(
                    fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                    height: 20.0 / 14.0, // leading-5
                    fontWeight: HeroTokens.weightMedium.resolve(context),
                    color: color,
                    decoration:
                        _hovered && interactive && !widget.current
                        ? TextDecoration.underline
                        : TextDecoration.none,
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

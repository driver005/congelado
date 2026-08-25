import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 tabs variants (tabs.css `.tabs` / `.tabs--secondary`).
enum HeroTabsVariant {
  /// Pill container with a segment indicator (default).
  primary,

  /// Flat list with an underline indicator (`.tabs--secondary`).
  secondary,
}

final Map<HeroTabsVariant, RemixTabBarStyle> _heroTabBarStyleCache = {};
final Map<HeroTabsVariant, RemixTabStyle> _heroTabStyleCache = {};

/// Returns the [RemixTabBarStyle] for a HeroUI v3 tab list container.
///
/// tabs.css: `.tabs__list-container` — `bg-default`, radius
/// `calc(var(--radius) * 2.5)` = 20, `.tabs__list` `p-1` (4). Secondary:
/// transparent, no radius, `border-b border-border`.
RemixTabBarStyle heroTabBarStyle({HeroTabsVariant variant = HeroTabsVariant.primary}) {
  return _heroTabBarStyleCache.putIfAbsent(variant, () {
    if (variant == HeroTabsVariant.secondary) {
      return RemixTabBarStyle()
          .container(
            FlexBoxStyler()
                .direction(Axis.horizontal)
                .color(HeroTokens.colorTransparent())
                .borderBottom(
                  color: HeroTokens.colorBorder(),
                  width: HeroTokens.doubleBorderWidth(),
                ),
          );
    }
    return RemixTabBarStyle()
        .container(
          FlexBoxStyler()
              .direction(Axis.horizontal)
              .color(HeroTokens.colorDefault())
              .borderRadiusAll(HeroTokens.radius2xl())
              .paddingAll(HeroTokens.space1()),
        );
  });
}

/// Returns the [RemixTabStyle] for a HeroUI v3 tab.
///
/// tabs.css: `.tabs__tab` — `h-8 rounded-3xl px-4 text-sm font-medium
/// text-muted`; selected `text-segment-foreground` (primary) / `text-foreground`
/// with the accent underline (secondary). Remix has no sliding indicator slot,
/// so the selected state is styled directly on the tab (approximation, see the
/// worksheet).
RemixTabStyle heroTabStyle({HeroTabsVariant variant = HeroTabsVariant.primary}) {
  return _heroTabStyleCache.putIfAbsent(variant, () {
    final base = RemixTabStyle()
        .container(
          FlexBoxStyler()
              .height(HeroTokens.doubleTabsTabHeight())
              .paddingX(HeroTokens.doubleTabsTabPaddingX())
              .borderRadiusAll(HeroTokens.radius3xl())
              // `.tabs__tab` is `items-center justify-center`.
              .alignment(Alignment.center),
        )
        .label(
          TextStyler()
              .style(HeroTokens.typeSm.mix())
              .fontWeight(HeroTokens.weightMedium())
              .color(HeroTokens.colorMuted()),
        );

    if (variant == HeroTabsVariant.secondary) {
      return base
          // Hover: opacity 0.7 (tabs.css @media hover).
          .onHovered(RemixTabStyle().label(TextStyler().color(HeroTokens.colorMuted().withOpacity(0.7))))
          // Selected: ACCENT text + 2px accent underline (global emphasis
          // color — product decision; HeroUI itself uses foreground here).
          .onSelected(
            RemixTabStyle()
                .label(TextStyler().color(HeroTokens.colorAccent()))
                .container(
                  FlexBoxStyler().borderBottom(
                    color: HeroTokens.colorAccent(),
                    width: HeroTokens.doubleTabsIndicatorHeight(),
                  ),
                ),
          );
    }

    return base
        .onHovered(RemixTabStyle().label(TextStyler().color(HeroTokens.colorMuted().withOpacity(0.7))))
        // Selected: accent-family pill (rounded-3xl) — global emphasis color.
        .onSelected(
          RemixTabStyle()
              .container(
                FlexBoxStyler().color(HeroTokens.colorAccentSoft()),
              )
              .label(TextStyler().color(HeroTokens.colorAccent())),
        );
  });
}

/// One tab entry for [HeroTabs].
class HeroTab {
  const HeroTab({required this.id, required this.label, required this.child});

  final String id;
  final String label;
  final Widget child;
}

/// A HeroUI v3 tabs component (tabs.css).
///
/// ```dart
/// HeroTabs(
///   selectedTabId: 'overview',
///   onChanged: (id) => setState(() => _selected = id),
///   tabs: [
///     HeroTab(id: 'overview', label: 'Overview', child: ...),
///     HeroTab(id: 'activity', label: 'Activity', child: ...),
///   ],
/// )
/// ```
class HeroTabs extends StatelessWidget {
  const HeroTabs({
    super.key,
    required this.tabs,
    this.variant = HeroTabsVariant.primary,
    this.selectedTabId,
    this.onChanged,
    this.enabled = true,
  });

  final List<HeroTab> tabs;
  final HeroTabsVariant variant;
  final String? selectedTabId;
  final ValueChanged<String>? onChanged;
  final bool enabled;

  @override
  Widget build(BuildContext context) {
    return RemixTabs(
      selectedTabId: selectedTabId,
      onChanged: onChanged,
      enabled: enabled,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          RemixTabBar(
            style: heroTabBarStyle(variant: variant),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                for (final tab in tabs)
                  RemixTab(
                    tabId: tab.id,
                    label: tab.label,
                    style: heroTabStyle(variant: variant),
                  ),
              ],
            ),
          ),
          for (final tab in tabs)
            RemixTabView(
              tabId: tab.id,
              style: RemixTabViewStyle().container(
                BoxStyler().paddingAll(HeroTokens.doubleTabsPanelPadding()),
              ),
              child: Padding(
                padding: EdgeInsets.only(top: HeroTokens.doubleTabsPanelGap.resolve(context)),
                child: tab.child,
              ),
            ),
        ],
      ),
    );
  }
}

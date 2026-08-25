import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Display component use cases: Chip, Badge, Tabs, Skeleton, Tooltip, Modal,
/// Progress, Spinner, Divider, Avatar.
List<WidgetbookNode> displayUseCases() {
  return [
    WidgetbookComponent(
      name: 'Chip',
      useCases: [
        WidgetbookUseCase(
          name: 'Colors & variants',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final color in HeroChipColor.values)
                HeroChip(label: color.name, color: color),
              for (final color in HeroChipColor.values)
                HeroChip(
                  label: color.name,
                  color: color,
                  variant: HeroChipVariant.solid,
                ),
              for (final color in HeroChipColor.values)
                HeroChip(
                  label: color.name,
                  color: color,
                  variant: HeroChipVariant.tertiary,
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroChipSize.values)
                HeroChip(label: size.name, size: size),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With avatar, icon & close',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              HeroChip(
                label: 'Jane Doe',
                avatar: const HeroAvatar(label: 'JD', size: HeroAvatarSize.sm),
              ),
              HeroChip(
                label: 'Add tag',
                icon: Icons.add_rounded,
                variant: HeroChipVariant.tertiary,
              ),
              HeroChip(
                label: 'Dismissible',
                color: HeroChipColor.danger,
                onClose: () {},
              ),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Badge',
      useCases: [
        WidgetbookUseCase(
          name: 'Colors & variants',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final color in HeroBadgeColor.values)
                HeroBadge(label: color.name, color: color),
              for (final color in HeroBadgeColor.values)
                HeroBadge(
                  label: color.name,
                  color: color,
                  variant: HeroBadgeVariant.solid,
                ),
              for (final color in HeroBadgeColor.values)
                HeroBadge(
                  label: color.name,
                  color: color,
                  variant: HeroBadgeVariant.soft,
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroBadgeSize.values)
                HeroBadge(label: size.name, size: size),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Tabs',
      useCases: [
        WidgetbookUseCase(
          name: 'Primary',
          builder: (context) => const _TabsDemo(),
        ),
        WidgetbookUseCase(
          name: 'Secondary',
          builder: (context) => const _TabsDemo(variant: HeroTabsVariant.secondary),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Skeleton',
      useCases: [
        WidgetbookUseCase(
          name: 'Animations',
          builder: (context) => HeroCard(
            // surface-tertiary skeleton reads on the WHITE card, same as the
            // heroui.com docs preview — on the page background it is invisible.
            child: Wrap(
              spacing: 16,
              runSpacing: 16,
              children: [
                for (final animation in HeroSkeletonAnimation.values)
                  SizedBox(
                    width: 160,
                    child: HeroSkeleton(
                      height: 16,
                      animation: animation,
                    ),
                  ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Card loading state',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroCard(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: const [
                  HeroSkeleton(width: 140, height: 14),
                  SizedBox(height: 8),
                  HeroSkeleton(height: 12),
                  SizedBox(height: 8),
                  HeroSkeleton(width: 220, height: 12),
                ],
              ),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Tooltip',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroTooltip(
            message: 'A HeroUI v3 tooltip',
            child: HeroBadge(
              label: 'Hover me',
              color: HeroBadgeColor.accent,
              variant: HeroBadgeVariant.soft,
            ),
          ),
        ),
        // Mirrors the HeroUI storybook "Placement" story: a tooltip on every
        // side of the trigger. delay: 0 so the demo is usable immediately.
        WidgetbookUseCase(
          name: 'Placement',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(48),
            child: Wrap(
              spacing: 16,
              runSpacing: 16,
              children: [
                for (final placement in HeroTooltipPlacement.values)
                  HeroTooltip(
                    message: 'Placed ${placement.name}',
                    placement: placement,
                    delay: Duration.zero,
                    child: HeroButton(
                      label: placement.name,
                      variant: HeroButtonVariant.ghost,
                      onPressed: () {},
                    ),
                  ),
              ],
            ),
          ),
        ),
        // Mirrors the HeroUI storybook "With Arrow" story (offset 7).
        WidgetbookUseCase(
          name: 'With arrow',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(48),
            child: Wrap(
              spacing: 16,
              runSpacing: 16,
              children: [
                for (final placement in [
                  HeroTooltipPlacement.top,
                  HeroTooltipPlacement.right,
                  HeroTooltipPlacement.bottom,
                  HeroTooltipPlacement.left,
                ])
                  HeroTooltip(
                    message: 'Arrow ${placement.name}',
                    placement: placement,
                    showArrow: true,
                    delay: Duration.zero,
                    child: HeroButton(
                      label: placement.name,
                      variant: HeroButtonVariant.ghost,
                      onPressed: () {},
                    ),
                  ),
              ],
            ),
          ),
        ),
        // Mirrors the HeroUI storybook "Custom Offset" story.
        WidgetbookUseCase(
          name: 'Custom offset',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(48),
            child: HeroTooltip(
              message: 'Offset far from the trigger',
              offset: 24,
              delay: Duration.zero,
              child: HeroButton(
                label: 'Hover me',
                variant: HeroButtonVariant.ghost,
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'On a button',
          builder: (context) => const HeroTooltip(
            message: 'Deletes the selected item',
            child: HeroButton(
              label: 'Delete',
              variant: HeroButtonVariant.dangerSoft,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Modal',
      useCases: [
        WidgetbookUseCase(
          name: 'Open dialog',
          builder: (context) => HeroButton(
            label: 'Open modal',
            onPressed: () => showHeroModal<void>(
              context,
              size: HeroModalSize.md,
              title: 'Modal title',
              description: 'A HeroUI v3 modal built on RemixDialog.',
              actions: [
                Builder(
                  builder: (dialogContext) => HeroButton(
                    label: 'Cancel',
                    variant: HeroButtonVariant.ghost,
                    onPressed: () => Navigator.of(dialogContext).pop(),
                  ),
                ),
                Builder(
                  builder: (dialogContext) => HeroButton(
                    label: 'Confirm',
                    onPressed: () => Navigator.of(dialogContext).pop(),
                  ),
                ),
              ],
              builder: (context) => const SizedBox(
                width: double.infinity,
                child: Text('Modal content goes here.'),
              ),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Progress',
      useCases: [
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => SizedBox(
            width: 320,
            child: Column(
              children: [
                for (final color in HeroProgressColor.values)
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    child: HeroProgress(value: 0.7, color: color),
                  ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Values',
          builder: (context) => SizedBox(
            width: 320,
            child: Column(
              children: [
                for (final value in const [0.0, 0.25, 0.5, 0.75, 1.0])
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    child: HeroProgress(value: value),
                  ),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Spinner',
      useCases: [
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 16,
            children: [
              for (final color in HeroSpinnerColor.values)
                HeroSpinner(color: color),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 16,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroSpinnerSize.values)
                HeroSpinner(size: size, color: HeroSpinnerColor.accent),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'In a button',
          builder: (context) => HeroButton(
            label: 'Loading',
            loading: true,
            onPressed: () {},
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Divider',
      useCases: [
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => SizedBox(
            width: 320,
            child: Column(
              children: [
                for (final color in HeroDividerColor.values)
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 12),
                    child: HeroDivider(color: color),
                  ),
                // default separator is subtle on the page background; on a
                // surface card it reads correctly.
                const SizedBox(height: 8),
                HeroCard(
                  child: Column(
                    children: [
                      const HeroCardTitle('Inside a card'),
                      const HeroDivider(),
                      const Text('Divided content.'),
                    ],
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Avatar',
      useCases: [
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 16,
            runSpacing: 16,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroAvatarSize.values)
                HeroAvatar(label: 'JD', size: size),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Fallback colors',
          builder: (context) => Wrap(
            spacing: 16,
            runSpacing: 16,
            children: [
              for (final color in HeroAvatarColor.values)
                HeroAvatar(label: 'AK', color: color),
            ],
          ),
        ),
      ],
    ),
  ];
}

class _TabsDemo extends StatefulWidget {
  const _TabsDemo({this.variant = HeroTabsVariant.primary});

  final HeroTabsVariant variant;

  @override
  State<_TabsDemo> createState() => _TabsDemoState();
}

class _TabsDemoState extends State<_TabsDemo> {
  String _selected = 'overview';

  @override
  Widget build(BuildContext context) {
    return HeroTabs(
      variant: widget.variant,
      selectedTabId: _selected,
      onChanged: (id) => setState(() => _selected = id),
      tabs: const [
        HeroTab(id: 'overview', label: 'Overview', child: Text('Overview content')),
        HeroTab(id: 'activity', label: 'Activity', child: Text('Activity content')),
        HeroTab(id: 'settings', label: 'Settings', child: Text('Settings content')),
      ],
    );
  }
}

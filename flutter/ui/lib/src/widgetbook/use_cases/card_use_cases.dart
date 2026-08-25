import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// HeroCard use cases: variants and the title/description composition.
List<WidgetbookNode> cardUseCases() {
  return [
    WidgetbookComponent(
      name: 'Card',
      useCases: [
        for (final variant in HeroCardVariant.values)
          WidgetbookUseCase(
            name: variant.name,
            builder: (context) => HeroCard(
              variant: variant,
              child: Text('Card content goes here.'),
            ),
          ),
        WidgetbookUseCase(
          name: 'With header',
          builder: (context) => SizedBox(
            width: 340,
            child: HeroCard(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const HeroCardTitle('Product'),
                  const HeroCardDescription('Details about this product.'),
                  const SizedBox(height: 12),
                  HeroButton(
                    label: 'Open modal',
                    variant: HeroButtonVariant.secondary,
                    onPressed: () {},
                  ),
                ],
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Kitchen sink',
          builder: (context) => SizedBox(
            width: 340,
            child: HeroCard(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const HeroCardTitle('Deployment'),
                  const HeroCardDescription('Last deployed 2 hours ago.'),
                  const SizedBox(height: 12),
                  const Wrap(
                    spacing: 8,
                    runSpacing: 8,
                    children: [
                      HeroBadge(label: 'Healthy', color: HeroBadgeColor.success),
                      HeroBadge(label: 'v1.2.0'),
                      HeroChip(label: 'prod', variant: HeroChipVariant.solid),
                    ],
                  ),
                  const SizedBox(height: 16),
                  const HeroDivider(),
                  const SizedBox(height: 16),
                  Row(
                    children: [
                      const HeroAvatar(label: 'JD'),
                      const SizedBox(width: 12),
                      Expanded(
                        child: HeroProgress(value: 0.72),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ),
      ],
    ),
  ];
}

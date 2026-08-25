import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// HeroButton use cases: every variant, plus sizes, states and a knob-driven
/// interactive case.
List<WidgetbookNode> buttonUseCases() {
  return [
    WidgetbookComponent(
      name: 'Button',
      useCases: [
        for (final variant in HeroButtonVariant.values)
          WidgetbookUseCase(
            name: variant.name,
            builder: (context) => HeroButton(
              label: variant.name,
              variant: variant,
              onPressed: () {},
            ),
          ),
        WidgetbookUseCase(
          name: 'Loading',
          builder: (context) => HeroButton(
            label: 'Saving…',
            variant: HeroButtonVariant.primary,
            loading: true,
            onPressed: () {},
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              const HeroButton(label: 'Disabled'),
              const HeroButton(label: 'Disabled', variant: HeroButtonVariant.ghost),
              const HeroButton(label: 'Disabled', variant: HeroButtonVariant.outline),
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
              for (final size in HeroButtonSize.values)
                HeroButton(label: size.name, size: size, onPressed: () {}),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With icons',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              HeroButton(
                label: 'Add',
                leadingIcon: Icons.add_rounded,
                onPressed: () {},
              ),
              HeroButton(
                label: 'Next',
                trailingIcon: Icons.arrow_forward_rounded,
                onPressed: () {},
              ),
              HeroButton(
                label: 'Export',
                variant: HeroButtonVariant.secondary,
                leadingIcon: Icons.download_rounded,
                onPressed: () {},
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final variant = context.knobs.object.dropdown<HeroButtonVariant>(
              label: 'Variant',
              options: HeroButtonVariant.values,
            );
            final label = context.knobs.string(
              label: 'Label',
              initialValue: 'Button',
            );
            final loading = context.knobs.boolean(label: 'Loading');
            final enabled = context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            );
            return HeroButton(
              label: label,
              variant: variant,
              loading: loading,
              enabled: enabled,
              onPressed: () {},
            );
          },
        ),
      ],
    ),
  ];
}

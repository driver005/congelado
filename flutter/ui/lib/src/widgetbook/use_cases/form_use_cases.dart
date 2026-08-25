import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Form control use cases: HeroInput, HeroSwitch, HeroCheckbox, HeroRadio.
List<WidgetbookNode> formUseCases() {
  return [
    WidgetbookComponent(
      name: 'Input',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInput(hintText: 'Enter something…'),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label & helper',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInput(
              label: 'Email',
              hintText: 'you@example.com',
              helperText: 'We will never share your email.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInput(
              hintText: 'Enter something…',
              label: 'Field',
              error: true,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInput(hintText: 'Read only', enabled: false),
          ),
        ),
        WidgetbookUseCase(
          name: 'With adornments',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInput(
              hintText: 'Search',
              leading: const Icon(Icons.search_rounded, size: 18),
              trailing: IconButton(
                icon: const Icon(Icons.close_rounded, size: 18),
                onPressed: () {},
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Secondary variant',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInput(
              variant: HeroInputVariant.secondary,
              hintText: 'Secondary field',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final error = context.knobs.boolean(label: 'Error');
            final enabled = context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            );
            final obscure = context.knobs.boolean(label: 'Obscure text');
            return SizedBox(
              width: 320,
              child: HeroInput(
                hintText: obscure ? '••••••••' : 'Type here…',
                error: error,
                enabled: enabled,
                obscureText: obscure,
              ),
            );
          },
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Switch',
      useCases: [
        WidgetbookUseCase(
          name: 'Toggle',
          builder: (context) => const _SwitchDemo(),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => const _SwitchSizesDemo(),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Wrap(
            spacing: 16,
            children: [
              HeroSwitch(selected: false, onChanged: (_) {}, enabled: false),
              HeroSwitch(selected: true, onChanged: (_) {}, enabled: false),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Checkbox',
      useCases: [
        WidgetbookUseCase(
          name: 'Toggle',
          builder: (context) => const _CheckboxDemo(),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Wrap(
            spacing: 16,
            children: [
              HeroCheckbox(selected: false, onChanged: (_) {}, enabled: false),
              HeroCheckbox(selected: true, onChanged: (_) {}, enabled: false),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Radio',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _RadioDemo(),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'RadioGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Group',
          builder: (context) => const _RadioDemo(),
        ),
      ],
    ),
  ];
}

class _SwitchDemo extends StatefulWidget {
  const _SwitchDemo();

  @override
  State<_SwitchDemo> createState() => _SwitchDemoState();
}

class _SwitchDemoState extends State<_SwitchDemo> {
  bool _on = true;

  @override
  Widget build(BuildContext context) {
    return HeroSwitch(
      selected: _on,
      onChanged: (v) => setState(() => _on = v),
    );
  }
}

class _SwitchSizesDemo extends StatefulWidget {
  const _SwitchSizesDemo();

  @override
  State<_SwitchSizesDemo> createState() => _SwitchSizesDemoState();
}

class _SwitchSizesDemoState extends State<_SwitchSizesDemo> {
  final Map<HeroSwitchSize, bool> _on = {
    HeroSwitchSize.sm: true,
    HeroSwitchSize.md: true,
    HeroSwitchSize.lg: true,
  };

  @override
  Widget build(BuildContext context) {
    return Wrap(
      spacing: 24,
      runSpacing: 16,
      crossAxisAlignment: WrapCrossAlignment.center,
      children: [
        for (final size in HeroSwitchSize.values)
          HeroSwitch(
            selected: _on[size]!,
            size: size,
            onChanged: (v) => setState(() => _on[size] = v),
          ),
      ],
    );
  }
}

class _CheckboxDemo extends StatefulWidget {
  const _CheckboxDemo();

  @override
  State<_CheckboxDemo> createState() => _CheckboxDemoState();
}

class _CheckboxDemoState extends State<_CheckboxDemo> {
  bool _checked = true;

  @override
  Widget build(BuildContext context) {
    return HeroCheckbox(
      selected: _checked,
      onChanged: (v) => setState(() => _checked = v),
    );
  }
}

class _RadioDemo extends StatefulWidget {
  const _RadioDemo();

  @override
  State<_RadioDemo> createState() => _RadioDemoState();
}

class _RadioDemoState extends State<_RadioDemo> {
  String _value = 'a';

  @override
  Widget build(BuildContext context) {
    return HeroRadioGroup<String>(
      groupValue: _value,
      onChanged: (v) => setState(() => _value = v),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          HeroRadio(value: 'a'),
          const SizedBox(width: 8),
          HeroRadio(value: 'b'),
          const SizedBox(width: 8),
          HeroRadio(value: 'c'),
        ],
      ),
    );
  }
}

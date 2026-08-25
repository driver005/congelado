import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Form/selection use cases: Textarea, SearchField, NumberField, InputOtp,
/// Select, Slider, ToggleButton, ToggleButtonGroup, ListBox, Menu,
/// Autocomplete, ComboBox, TimeField, InputGroup, CheckboxGroup, SwitchGroup.
///
/// Sub-parts are demonstrated inside their parents: 'ListBoxItem' and
/// 'ListBoxSection' stories live in the 'ListBox' component (HeroListBoxItem /
/// HeroListBoxSection), 'MenuItem' and 'MenuSection' in the 'Menu' component
/// (HeroMenuItem / HeroMenuSection).
List<WidgetbookNode> selectionUseCases() {
  return [
    WidgetbookComponent(
      name: 'Textarea',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroTextarea(placeholder: 'Enter your message…'),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label & helper',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroTextarea(
              label: 'Message',
              placeholder: 'Enter your message…',
              helperText: 'Max 100 characters.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroTextarea(
              label: 'Message',
              placeholder: 'Enter your message…',
              helperText: 'Message is required.',
              error: true,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroTextarea(
              label: 'Message',
              placeholder: 'Read only',
              enabled: false,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Secondary',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroTextarea(
              variant: HeroInputVariant.secondary,
              placeholder: 'Secondary textarea',
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'SearchField',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSearchField(placeholder: 'Search…'),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSearchField(
              label: 'Search',
              placeholder: 'Search users…',
              helperText: 'Find people by name.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _SearchFieldDemo(),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSearchField(
              label: 'Search',
              placeholder: 'Search…',
              error: true,
              helperText: 'Query is too short.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSearchField(placeholder: 'Search…', enabled: false),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'NumberField',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroNumberField(value: 42),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroNumberField(
              value: 5,
              label: 'Quantity',
              helperText: 'Between 0 and 10.',
              min: 0,
              max: 10,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _NumberFieldDemo(),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroNumberField(
              value: 99,
              label: 'Quantity',
              error: true,
              helperText: 'Must be at most 10.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroNumberField(value: 42, enabled: false),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'InputOtp',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInputOtp(),
          ),
        ),
        WidgetbookUseCase(
          name: 'With value',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInputOtp(value: '4826'),
          ),
        ),
        WidgetbookUseCase(
          name: 'Grouped',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInputOtp(length: 6, groupEvery: 3),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _InputOtpDemo(),
        ),
        WidgetbookUseCase(
          name: 'Invalid',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInputOtp(value: '1234', error: true),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroInputOtp(value: '123456', enabled: false),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Select',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _SelectDemo(),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSelect<String>(
              label: 'Animal',
              helperText: 'Pick your favourite.',
              placeholder: 'Select an animal',
              items: [
                HeroSelectItem(value: 'cat', label: 'Cat'),
                HeroSelectItem(value: 'dog', label: 'Dog'),
                HeroSelectItem(value: 'bird', label: 'Bird', description: 'Wings!'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With selected value',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSelect<String>(
              value: 'dog',
              placeholder: 'Select an animal',
              items: [
                HeroSelectItem(value: 'cat', label: 'Cat'),
                HeroSelectItem(value: 'dog', label: 'Dog'),
                HeroSelectItem(value: 'bird', label: 'Bird'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Danger item',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSelect<String>(
              placeholder: 'Choose an action',
              items: [
                HeroSelectItem(value: 'edit', label: 'Edit'),
                HeroSelectItem(value: 'delete', label: 'Delete', danger: true),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSelect<String>(
              label: 'Animal',
              placeholder: 'Select an animal',
              error: true,
              helperText: 'Please choose one.',
              items: [
                HeroSelectItem(value: 'cat', label: 'Cat'),
                HeroSelectItem(value: 'dog', label: 'Dog'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSelect<String>(
              placeholder: 'Select an animal',
              enabled: false,
              items: [
                HeroSelectItem(value: 'cat', label: 'Cat'),
                HeroSelectItem(value: 'dog', label: 'Dog'),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Slider',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _SliderDemo(),
        ),
        WidgetbookUseCase(
          name: 'With label & output',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSlider(value: 40, label: 'Volume'),
          ),
        ),
        WidgetbookUseCase(
          name: 'With step',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSlider(value: 30, step: 10, label: 'Zoom'),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _SliderDemo(initial: 60),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroSlider(value: 70, label: 'Volume', enabled: false),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ToggleButton',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _ToggleButtonDemo(),
        ),
        WidgetbookUseCase(
          name: 'Ghost',
          builder: (context) => const _ToggleButtonDemo(
            variant: HeroToggleButtonVariant.ghost,
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => const Wrap(
            spacing: 12,
            runSpacing: 12,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              HeroToggleButton(label: 'Small', size: HeroToggleButtonSize.sm),
              HeroToggleButton(label: 'Medium'),
              HeroToggleButton(label: 'Large', size: HeroToggleButtonSize.lg),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With icon',
          builder: (context) => const Wrap(
            spacing: 12,
            children: [
              HeroToggleButton(
                label: 'Bold',
                icon: Icons.format_bold_rounded,
                selected: true,
              ),
              HeroToggleButton(
                label: 'Italic',
                icon: Icons.format_italic_rounded,
              ),
              HeroToggleButton(label: 'Underline', icon: Icons.format_underlined_rounded),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Icon only',
          builder: (context) => const Wrap(
            spacing: 12,
            children: [
              HeroToggleButton(
                label: 'Bold',
                icon: Icons.format_bold_rounded,
                iconOnly: true,
                selected: true,
              ),
              HeroToggleButton(
                label: 'Italic',
                icon: Icons.format_italic_rounded,
                iconOnly: true,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const Wrap(
            spacing: 12,
            children: [
              HeroToggleButton(label: 'Disabled', disabled: true),
              HeroToggleButton(
                label: 'Disabled selected',
                selected: true,
                disabled: true,
              ),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ToggleButtonGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Horizontal',
          builder: (context) => const _ToggleGroupDemo(),
        ),
        WidgetbookUseCase(
          name: 'Vertical',
          builder: (context) => const _ToggleGroupDemo(
            orientation: Axis.vertical,
          ),
        ),
        WidgetbookUseCase(
          name: 'Detached',
          builder: (context) => const _ToggleGroupDemo(detached: true),
        ),
        WidgetbookUseCase(
          name: 'Full width',
          builder: (context) => const _ToggleGroupDemo(fullWidth: true),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ListBox',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 260,
            child: HeroListBox(
              children: [
                HeroListBoxItem(label: 'New file', onPressed: _noop),
                HeroListBoxItem(label: 'Copy link', onPressed: _noop),
                HeroListBoxItem(label: 'Edit file', onPressed: _noop),
                HeroListBoxItem(label: 'Delete', danger: true, onPressed: _noop),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With descriptions',
          builder: (context) => const SizedBox(
            width: 260,
            child: HeroListBox(
              children: [
                HeroListBoxItem(
                  label: 'Forward',
                  description: 'Send to another device',
                  onPressed: _noop,
                ),
                HeroListBoxItem(
                  label: 'Reply',
                  description: 'Quick response',
                  onPressed: _noop,
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With sections',
          builder: (context) => const SizedBox(
            width: 260,
            child: HeroListBox(
              children: [
                HeroListBoxSection(
                  header: 'Actions',
                  children: [
                    HeroListBoxItem(label: 'Copy', onPressed: _noop),
                    HeroListBoxItem(label: 'Paste', onPressed: _noop),
                  ],
                ),
                HeroListBoxSection(
                  header: 'Danger zone',
                  children: [
                    HeroListBoxItem(label: 'Delete', danger: true, onPressed: _noop),
                  ],
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Selected',
          builder: (context) => const SizedBox(
            width: 260,
            child: HeroListBox(
              children: [
                HeroListBoxItem(label: 'Compact', selected: true, onPressed: _noop),
                HeroListBoxItem(label: 'Comfortable', onPressed: _noop),
                HeroListBoxItem(label: 'Spacious', onPressed: _noop),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled item',
          builder: (context) => const SizedBox(
            width: 260,
            child: HeroListBox(
              children: [
                HeroListBoxItem(label: 'Available', onPressed: _noop),
                HeroListBoxItem(label: 'Locked', disabled: true),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Menu',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _MenuDemo(),
        ),
        WidgetbookUseCase(
          name: 'With sections',
          builder: (context) => const _MenuSectionsDemo(),
        ),
        WidgetbookUseCase(
          name: 'Selection modes',
          builder: (context) => const _MenuSelectionDemo(),
        ),
        WidgetbookUseCase(
          name: 'Danger item',
          builder: (context) => const _MenuDangerDemo(),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Autocomplete',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroAutocomplete<String>(
              placeholder: 'Search fruits…',
              items: [
                HeroAutocompleteItem(value: 'apple', label: 'Apple'),
                HeroAutocompleteItem(value: 'banana', label: 'Banana'),
                HeroAutocompleteItem(value: 'cherry', label: 'Cherry'),
                HeroAutocompleteItem(value: 'date', label: 'Date'),
                HeroAutocompleteItem(value: 'elderberry', label: 'Elderberry'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroAutocomplete<String>(
              label: 'Fruit',
              helperText: 'Pick a fruit from the list.',
              placeholder: 'Search fruits…',
              items: [
                HeroAutocompleteItem(value: 'apple', label: 'Apple'),
                HeroAutocompleteItem(value: 'banana', label: 'Banana'),
                HeroAutocompleteItem(value: 'cherry', label: 'Cherry'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _AutocompleteDemo(),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroAutocomplete<String>(
              placeholder: 'Search…',
              enabled: false,
              items: [HeroAutocompleteItem(value: 'a', label: 'Apple')],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ComboBox',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _ComboBoxDemo(),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroComboBox<String>(
              label: 'Country',
              helperText: 'Start typing to filter.',
              placeholder: 'Select a country',
              items: [
                HeroComboBoxItem(value: 'fr', label: 'France'),
                HeroComboBoxItem(value: 'de', label: 'Germany'),
                HeroComboBoxItem(value: 'es', label: 'Spain'),
                HeroComboBoxItem(value: 'it', label: 'Italy'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With selected value',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroComboBox<String>(
              selectedValue: 'de',
              placeholder: 'Select a country',
              items: [
                HeroComboBoxItem(value: 'fr', label: 'France'),
                HeroComboBoxItem(value: 'de', label: 'Germany'),
                HeroComboBoxItem(value: 'es', label: 'Spain'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroComboBox<String>(
              placeholder: 'Select a country',
              enabled: false,
              items: [HeroComboBoxItem(value: 'fr', label: 'France')],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'TimeField',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 220,
            child: HeroTimeField(),
          ),
        ),
        WidgetbookUseCase(
          name: 'With value',
          builder: (context) => const SizedBox(
            width: 220,
            child: HeroTimeField(value: TimeOfDay(hour: 14, minute: 30)),
          ),
        ),
        WidgetbookUseCase(
          name: '24 hour',
          builder: (context) => const SizedBox(
            width: 180,
            child: HeroTimeField(value: TimeOfDay(hour: 14, minute: 30), hourCycle: 24),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const SizedBox(
            width: 220,
            child: HeroTimeField(
              label: 'Alarm',
              helperText: '24 hour format.',
              hourCycle: 24,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const _TimeFieldDemo(),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 220,
            child: HeroTimeField(
              value: TimeOfDay(hour: 9, minute: 0),
              enabled: false,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'InputGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInputGroup(
              prefix: const Text('€'),
              children: [_bareInput('0.00')],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With suffix',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInputGroup(
              suffix: const Icon(Icons.check_rounded, size: 16),
              children: [_bareInput('you@example.com')],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Two fields',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInputGroup(
              children: [
                _bareInput('First'),
                _bareInput('Last'),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInputGroup(
              error: true,
              prefix: const Text('€'),
              children: [_bareInput('0.00')],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => SizedBox(
            width: 320,
            child: HeroInputGroup(
              enabled: false,
              prefix: const Text('€'),
              children: [_bareInput('0.00')],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'CheckboxGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _CheckboxGroupDemo(),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const _CheckboxGroupDemo(labeled: true),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 320,
            child: HeroCheckboxGroup(
              children: [
                HeroCheckbox(selected: true, onChanged: _noopBool, enabled: false),
                HeroCheckbox(selected: false, onChanged: _noopBool, enabled: false),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'SwitchGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const _SwitchGroupDemo(),
        ),
        WidgetbookUseCase(
          name: 'Horizontal',
          builder: (context) => const _SwitchGroupDemo(horizontal: true),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const _SwitchGroupDemo(labeled: true),
        ),
      ],
    ),
  ];
}

void _noop() {}
void _noopBool(bool _) {}

Widget _bareInput(String hint) {
  return Expanded(
    child: Padding(
      padding: const EdgeInsets.symmetric(horizontal: 12),
      child: TextField(
        decoration: InputDecoration(
          isCollapsed: true,
          border: InputBorder.none,
          hintText: hint,
          hintStyle: const TextStyle(fontSize: 16),
        ),
        style: const TextStyle(fontSize: 16),
      ),
    ),
  );
}

class _SearchFieldDemo extends StatefulWidget {
  const _SearchFieldDemo();

  @override
  State<_SearchFieldDemo> createState() => _SearchFieldDemoState();
}

class _SearchFieldDemoState extends State<_SearchFieldDemo> {
  final TextEditingController _controller = TextEditingController();

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroSearchField(
        controller: _controller,
        placeholder: 'Search users…',
      ),
    );
  }
}

class _NumberFieldDemo extends StatefulWidget {
  const _NumberFieldDemo();

  @override
  State<_NumberFieldDemo> createState() => _NumberFieldDemoState();
}

class _NumberFieldDemoState extends State<_NumberFieldDemo> {
  double _value = 3;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroNumberField(
        value: _value,
        min: 0,
        max: 10,
        step: 1,
        label: 'Quantity',
        onChanged: (v) => setState(() => _value = v),
      ),
    );
  }
}

class _InputOtpDemo extends StatefulWidget {
  const _InputOtpDemo();

  @override
  State<_InputOtpDemo> createState() => _InputOtpDemoState();
}

class _InputOtpDemoState extends State<_InputOtpDemo> {
  String _value = '';

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroInputOtp(
        value: _value,
        onChanged: (v) => setState(() => _value = v),
      ),
    );
  }
}

class _SelectDemo extends StatefulWidget {
  const _SelectDemo();

  @override
  State<_SelectDemo> createState() => _SelectDemoState();
}

class _SelectDemoState extends State<_SelectDemo> {
  String? _value;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroSelect<String>(
        value: _value,
        onChanged: (v) => setState(() => _value = v),
        placeholder: 'Select an animal',
        items: const [
          HeroSelectItem(value: 'cat', label: 'Cat'),
          HeroSelectItem(value: 'dog', label: 'Dog'),
          HeroSelectItem(value: 'bird', label: 'Bird', description: 'Can fly'),
          HeroSelectItem(value: 'fish', label: 'Fish'),
        ],
      ),
    );
  }
}

class _SliderDemo extends StatefulWidget {
  const _SliderDemo({this.initial = 50});

  final double initial;

  @override
  State<_SliderDemo> createState() => _SliderDemoState();
}

class _SliderDemoState extends State<_SliderDemo> {
  late double _value = widget.initial;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroSlider(
        value: _value,
        onChanged: (v) => setState(() => _value = v),
        label: 'Volume',
      ),
    );
  }
}

class _ToggleButtonDemo extends StatefulWidget {
  const _ToggleButtonDemo({
    this.variant = HeroToggleButtonVariant.default_,
  });

  final HeroToggleButtonVariant variant;

  @override
  State<_ToggleButtonDemo> createState() => _ToggleButtonDemoState();
}

class _ToggleButtonDemoState extends State<_ToggleButtonDemo> {
  bool _selected = false;

  @override
  Widget build(BuildContext context) {
    return HeroToggleButton(
      label: _selected ? 'Selected' : 'Toggle',
      selected: _selected,
      variant: widget.variant,
      onPressed: () => setState(() => _selected = !_selected),
    );
  }
}

class _ToggleGroupDemo extends StatefulWidget {
  const _ToggleGroupDemo({
    this.orientation = Axis.horizontal,
    this.detached = false,
    this.fullWidth = false,
  });

  final Axis orientation;
  final bool detached;
  final bool fullWidth;

  @override
  State<_ToggleGroupDemo> createState() => _ToggleGroupDemoState();
}

class _ToggleGroupDemoState extends State<_ToggleGroupDemo> {
  String? _selected;

  @override
  Widget build(BuildContext context) {
    Widget button(String value) {
      return HeroToggleButton(
        label: value,
        selected: _selected == value,
        onPressed: () => setState(() => _selected = value),
      );
    }

    return SizedBox(
      width: widget.fullWidth ? 320 : null,
      child: HeroToggleButtonGroup(
        orientation: widget.orientation,
        detached: widget.detached,
        fullWidth: widget.fullWidth,
        children: [button('Bold'), button('Italic'), button('Underline')],
      ),
    );
  }
}

class _MenuDemo extends StatefulWidget {
  const _MenuDemo();

  @override
  State<_MenuDemo> createState() => _MenuDemoState();
}

class _MenuDemoState extends State<_MenuDemo> {
  @override
  Widget build(BuildContext context) {
    return Center(
      child: HeroMenu(
        trigger: HeroButton(label: 'Actions', onPressed: () {}),
        items: const [
          HeroMenuItem(label: 'New file', onPressed: _noop),
          HeroMenuItem(label: 'Copy link', onPressed: _noop),
          HeroMenuItem(label: 'Edit file', onPressed: _noop),
          HeroMenuItem(label: 'Delete file', danger: true, onPressed: _noop),
        ],
      ),
    );
  }
}

class _MenuSectionsDemo extends StatelessWidget {
  const _MenuSectionsDemo();

  @override
  Widget build(BuildContext context) {
    return Center(
      child: HeroMenu(
        trigger: HeroButton(label: 'Actions', onPressed: () {}),
        items: const [
          HeroMenuSection(
            header: 'Actions',
            children: [
              HeroMenuItem(label: 'Copy', onPressed: _noop),
              HeroMenuItem(label: 'Paste', onPressed: _noop),
            ],
          ),
          HeroMenuSection(
            header: 'Danger zone',
            children: [
              HeroMenuItem(label: 'Delete', danger: true, onPressed: _noop),
            ],
          ),
        ],
      ),
    );
  }
}

class _MenuSelectionDemo extends StatefulWidget {
  const _MenuSelectionDemo();

  @override
  State<_MenuSelectionDemo> createState() => _MenuSelectionDemoState();
}

class _MenuSelectionDemoState extends State<_MenuSelectionDemo> {
  String _size = 'md';

  @override
  Widget build(BuildContext context) {
    return Center(
      child: HeroMenu(
        trigger: HeroButton(label: 'Size', onPressed: () {}),
        items: [
          HeroMenuItem(
            label: 'Small',
            selectionMode: HeroMenuSelectionMode.single,
            selected: _size == 'sm',
            onPressed: () => setState(() => _size = 'sm'),
          ),
          HeroMenuItem(
            label: 'Medium',
            selectionMode: HeroMenuSelectionMode.single,
            selected: _size == 'md',
            onPressed: () => setState(() => _size = 'md'),
          ),
          HeroMenuItem(
            label: 'Large',
            selectionMode: HeroMenuSelectionMode.single,
            selected: _size == 'lg',
            onPressed: () => setState(() => _size = 'lg'),
          ),
        ]
      ),
    );
  }
}

class _MenuDangerDemo extends StatelessWidget {
  const _MenuDangerDemo();

  @override
  Widget build(BuildContext context) {
    return Center(
      child: HeroMenu(
        trigger: HeroButton(label: 'Actions', onPressed: () {}),
        items: const [
          HeroMenuItem(label: 'Archive', onPressed: _noop),
          HeroMenuItem(label: 'Delete', danger: true, onPressed: _noop),
        ],
      ),
    );
  }
}

class _AutocompleteDemo extends StatefulWidget {
  const _AutocompleteDemo();

  @override
  State<_AutocompleteDemo> createState() => _AutocompleteDemoState();
}

class _AutocompleteDemoState extends State<_AutocompleteDemo> {
  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroAutocomplete<String>(
        placeholder: 'Search fruits…',
        onSelected: (_) {},
        items: const [
          HeroAutocompleteItem(value: 'apple', label: 'Apple'),
          HeroAutocompleteItem(value: 'banana', label: 'Banana'),
          HeroAutocompleteItem(value: 'cherry', label: 'Cherry'),
          HeroAutocompleteItem(value: 'date', label: 'Date'),
          HeroAutocompleteItem(value: 'elderberry', label: 'Elderberry'),
        ],
      ),
    );
  }
}

class _ComboBoxDemo extends StatefulWidget {
  const _ComboBoxDemo();

  @override
  State<_ComboBoxDemo> createState() => _ComboBoxDemoState();
}

class _ComboBoxDemoState extends State<_ComboBoxDemo> {
  String? _selected;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroComboBox<String>(
        selectedValue: _selected,
        onChanged: (v) => setState(() => _selected = v),
        placeholder: 'Select a country',
        items: const [
          HeroComboBoxItem(value: 'fr', label: 'France'),
          HeroComboBoxItem(value: 'de', label: 'Germany'),
          HeroComboBoxItem(value: 'es', label: 'Spain'),
          HeroComboBoxItem(value: 'it', label: 'Italy'),
        ],
      ),
    );
  }
}

class _TimeFieldDemo extends StatefulWidget {
  const _TimeFieldDemo();

  @override
  State<_TimeFieldDemo> createState() => _TimeFieldDemoState();
}

class _TimeFieldDemoState extends State<_TimeFieldDemo> {
  TimeOfDay? _value;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 220,
      child: HeroTimeField(
        value: _value,
        onChanged: (v) => setState(() => _value = v),
      ),
    );
  }
}

class _CheckboxGroupDemo extends StatefulWidget {
  const _CheckboxGroupDemo({this.labeled = false});

  final bool labeled;

  @override
  State<_CheckboxGroupDemo> createState() => _CheckboxGroupDemoState();
}

class _CheckboxGroupDemoState extends State<_CheckboxGroupDemo> {
  bool _a = true;
  bool _b = false;
  bool _c = true;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroCheckboxGroup(
        label: widget.labeled ? const HeroLabel(text: 'Choose your toppings') : null,
        children: [
          HeroCheckbox(
            selected: _a,
            onChanged: (v) => setState(() => _a = v),
          ),
          HeroCheckbox(
            selected: _b,
            onChanged: (v) => setState(() => _b = v),
          ),
          HeroCheckbox(
            selected: _c,
            onChanged: (v) => setState(() => _c = v),
          ),
        ],
      ),
    );
  }
}

class _SwitchGroupDemo extends StatefulWidget {
  const _SwitchGroupDemo({this.horizontal = false, this.labeled = false});

  final bool horizontal;
  final bool labeled;

  @override
  State<_SwitchGroupDemo> createState() => _SwitchGroupDemoState();
}

class _SwitchGroupDemoState extends State<_SwitchGroupDemo> {
  bool _a = true;
  bool _b = false;
  bool _c = true;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 320,
      child: HeroSwitchGroup(
        orientation: widget.horizontal ? Axis.horizontal : Axis.vertical,
        label: widget.labeled ? const HeroLabel(text: 'Notifications') : null,
        children: [
          HeroSwitch(selected: _a, onChanged: (v) => setState(() => _a = v)),
          HeroSwitch(selected: _b, onChanged: (v) => setState(() => _b = v)),
          HeroSwitch(selected: _c, onChanged: (v) => setState(() => _c = v)),
        ],
      ),
    );
  }
}

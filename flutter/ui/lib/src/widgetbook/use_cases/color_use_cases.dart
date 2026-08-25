import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Color component use cases: ColorArea, ColorField, ColorInputGroup,
/// ColorPicker, ColorSlider, ColorSwatch, ColorSwatchPicker.
List<WidgetbookNode> colorUseCases() {
  return [
    WidgetbookComponent(
      name: 'ColorArea',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroColorArea(
            value: HSVColor.fromAHSV(1, 210, 0.7, 0.9),
            onChanged: _noopHsv,
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorAreaDemo(
            showDots: context.knobs.boolean(
              label: 'Show dots',
              initialValue: true,
            ),
            enabled: context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorField',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => SizedBox(
            width: 240,
            child: HeroColorField(
              value: _defaultColor,
              label: 'Color',
              helperText: 'Hex value',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorFieldDemo(
            error: context.knobs.boolean(label: 'Error'),
            enabled: context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorInputGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => SizedBox(
            width: 240,
            child: HeroColorInputGroup(value: _defaultColor),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorInputGroupDemo(
            variant: context.knobs.object.segmented<HeroColorInputGroupVariant>(
              label: 'Variant',
              options: HeroColorInputGroupVariant.values,
            ),
            error: context.knobs.boolean(label: 'Error'),
            enabled: context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorPicker',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => HeroColorPicker(
            color: _defaultColor,
            label: 'Pick a color',
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorPickerDemo(
            enabled: context.knobs.boolean(
              label: 'Enabled',
              initialValue: true,
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorSlider',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 280,
            child: HeroColorSlider(
              channel: HeroColorSliderChannel.hue,
              value: 0.58,
              label: 'Hue',
              output: '210°',
              onChanged: _noopDouble,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorSliderDemo(
            channel:
                context.knobs.object.segmented<HeroColorSliderChannel>(
              label: 'Channel',
              options: HeroColorSliderChannel.values,
            ),
            value: context.knobs.double.slider(
              label: 'Value',
              min: 0,
              max: 1,
              initialValue: 0.5,
              divisions: 100,
              precision: 2,
            ),
            vertical: context.knobs.boolean(label: 'Vertical'),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorSwatch',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Wrap(
            spacing: 16,
            runSpacing: 16,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroColorSwatchSize.values)
                HeroColorSwatch(color: _defaultColor, size: size),
              const HeroColorSwatch(color: null, size: HeroColorSwatchSize.md),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => Wrap(
            spacing: 16,
            runSpacing: 16,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              HeroColorSwatch(
                color: context.knobs.color(label: 'Color', initialValue: _defaultColor),
                size: context.knobs.object.segmented<HeroColorSwatchSize>(
                  label: 'Size',
                  options: HeroColorSwatchSize.values,
                ),
                shape: context.knobs.object.segmented<HeroColorSwatchShape>(
                  label: 'Shape',
                  options: HeroColorSwatchShape.values,
                ),
                selected: context.knobs.boolean(label: 'Selected'),
              ),
              HeroColorSwatch(
                color: _defaultColor,
                onTap: () {},
              ),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ColorSwatchPicker',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => HeroColorSwatchPicker(
            colors: heroDefaultColorSwatches(),
            value: _defaultColor,
            onChanged: _noopColor,
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => _ColorSwatchPickerDemo(
            size: context.knobs.object.segmented<HeroColorSwatchPickerSize>(
              label: 'Size',
              options: HeroColorSwatchPickerSize.values,
            ),
            shape: context.knobs.object.segmented<HeroColorSwatchPickerShape>(
              label: 'Shape',
              options: HeroColorSwatchPickerShape.values,
            ),
            layout: context.knobs.object.segmented<HeroColorSwatchPickerLayout>(
              label: 'Layout',
              options: HeroColorSwatchPickerLayout.values,
            ),
          ),
        ),
      ],
    ),
  ];
}

/// The demo color being picked (data, not chrome) — constructed from HSV so
/// the use cases stay free of raw literal colors.
Color _hsvColor(double hue, double saturation, double value) =>
    HSVColor.fromAHSV(1, hue, saturation, value).toColor();

final Color _defaultColor = _hsvColor(210, 0.85, 0.97);
void _noopHsv(HSVColor _) {}
void _noopDouble(double _) {}
void _noopColor(Color _) {}

class _ColorAreaDemo extends StatefulWidget {
  const _ColorAreaDemo({required this.showDots, required this.enabled});

  final bool showDots;
  final bool enabled;

  @override
  State<_ColorAreaDemo> createState() => _ColorAreaDemoState();
}

class _ColorAreaDemoState extends State<_ColorAreaDemo> {
  HSVColor _color = const HSVColor.fromAHSV(1, 210, 0.7, 0.9);

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        HeroColorArea(
          value: _color,
          showDots: widget.showDots,
          enabled: widget.enabled,
          onChanged: (c) => setState(() => _color = c),
        ),
        const SizedBox(height: 12),
        HeroColorSwatch(
          color: _color.toColor(),
          size: HeroColorSwatchSize.md,
        ),
      ],
    );
  }
}

class _ColorFieldDemo extends StatefulWidget {
  const _ColorFieldDemo({required this.error, required this.enabled});

  final bool error;
  final bool enabled;

  @override
  State<_ColorFieldDemo> createState() => _ColorFieldDemoState();
}

class _ColorFieldDemoState extends State<_ColorFieldDemo> {
  Color? _color = _defaultColor;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 240,
      child: HeroColorField(
        value: _color,
        label: 'Color',
        helperText: 'Hex value',
        error: widget.error,
        enabled: widget.enabled,
        onChanged: (c) => setState(() => _color = c),
      ),
    );
  }
}

class _ColorInputGroupDemo extends StatefulWidget {
  const _ColorInputGroupDemo({
    required this.variant,
    required this.error,
    required this.enabled,
  });

  final HeroColorInputGroupVariant variant;
  final bool error;
  final bool enabled;

  @override
  State<_ColorInputGroupDemo> createState() => _ColorInputGroupDemoState();
}

class _ColorInputGroupDemoState extends State<_ColorInputGroupDemo> {
  Color? _color = _defaultColor;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 240,
      child: HeroColorInputGroup(
        value: _color,
        variant: widget.variant,
        error: widget.error,
        enabled: widget.enabled,
        onChanged: (c) => setState(() => _color = c),
        onPick: () async {
          final picked = await showHeroColorPicker(
            context,
            initialColor: _color ?? _defaultColor,
          );
          if (picked != null) setState(() => _color = picked);
        },
      ),
    );
  }
}

class _ColorPickerDemo extends StatefulWidget {
  const _ColorPickerDemo({required this.enabled});

  final bool enabled;

  @override
  State<_ColorPickerDemo> createState() => _ColorPickerDemoState();
}

class _ColorPickerDemoState extends State<_ColorPickerDemo> {
  Color _color = _defaultColor;

  @override
  Widget build(BuildContext context) {
    return HeroColorPicker(
      color: _color,
      label: 'Pick a color',
      enabled: widget.enabled,
      onChanged: (c) => setState(() => _color = c),
    );
  }
}

class _ColorSliderDemo extends StatefulWidget {
  const _ColorSliderDemo({
    required this.channel,
    required this.value,
    required this.vertical,
  });

  final HeroColorSliderChannel channel;
  final double value;
  final bool vertical;

  @override
  State<_ColorSliderDemo> createState() => _ColorSliderDemoState();
}

class _ColorSliderDemoState extends State<_ColorSliderDemo> {
  late double _value;

  @override
  void initState() {
    super.initState();
    _value = widget.value;
  }

  @override
  void didUpdateWidget(_ColorSliderDemo oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value) _value = widget.value;
  }

  @override
  Widget build(BuildContext context) {
    final channel = widget.channel;
    final (label, output) = switch (channel) {
      HeroColorSliderChannel.hue => ('Hue', '${(_value * 360).round()}°'),
      HeroColorSliderChannel.saturation => (
          'Saturation',
          '${(_value * 100).round()}%',
        ),
      HeroColorSliderChannel.lightness => (
          'Lightness',
          '${(_value * 100).round()}%',
        ),
      HeroColorSliderChannel.alpha => ('Alpha', '${(_value * 100).round()}%'),
    };
    return Center(
      child: widget.vertical
          ? SizedBox(
              height: 220,
              child: HeroColorSlider(
                channel: channel,
                value: _value,
                label: label,
                output: output,
                orientation: Axis.vertical,
                onChanged: (v) => setState(() => _value = v),
              ),
            )
          : SizedBox(
              width: 280,
              child: HeroColorSlider(
                channel: channel,
                value: _value,
                label: label,
                output: output,
                onChanged: (v) => setState(() => _value = v),
              ),
            ),
    );
  }
}

class _ColorSwatchPickerDemo extends StatefulWidget {
  const _ColorSwatchPickerDemo({
    required this.size,
    required this.shape,
    required this.layout,
  });

  final HeroColorSwatchPickerSize size;
  final HeroColorSwatchPickerShape shape;
  final HeroColorSwatchPickerLayout layout;

  @override
  State<_ColorSwatchPickerDemo> createState() => _ColorSwatchPickerDemoState();
}

class _ColorSwatchPickerDemoState extends State<_ColorSwatchPickerDemo> {
  Color? _value = _defaultColor;

  @override
  Widget build(BuildContext context) {
    return HeroColorSwatchPicker(
      colors: heroDefaultColorSwatches(),
      value: _value,
      size: widget.size,
      shape: widget.shape,
      layout: widget.layout,
      onChanged: (c) => setState(() => _value = c),
    );
  }
}

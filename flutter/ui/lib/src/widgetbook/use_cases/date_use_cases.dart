import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Date/time use cases: DateField, DateInputGroup, DatePicker,
/// DateRangePicker, RangeCalendar, Calendar, CalendarYearPicker — mirroring
/// the HeroUI storybook (default, with label, disabled, interactive).
List<WidgetbookNode> dateUseCases() {
  return [
    WidgetbookComponent(
      name: 'DateField',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateField(value: _july22),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label & description',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateField(
              value: _july22,
              label: 'Birthday',
              description: 'Your date of birth.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Empty',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: HeroDateField(label: 'Event date'),
          ),
        ),
        WidgetbookUseCase(
          name: 'Error',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateField(
              value: _july22,
              label: 'Birthday',
              description: 'Your date of birth.',
              error: true,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateField(value: _july22, enabled: false),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateField(
              value: _july22,
              label: 'Birthday',
              error: context.knobs.boolean(label: 'Error'),
              enabled: context.knobs.boolean(
                label: 'Enabled',
                initialValue: true,
              ),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'DateInputGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateInputGroup(value: _july22),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label & helper',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateInputGroup(
              value: _july22,
              label: 'Departure',
              helperText: 'When does the trip start?',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Empty',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: HeroDateInputGroup(label: 'Event date'),
          ),
        ),
        WidgetbookUseCase(
          name: 'With clear',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateInputGroup(
              value: _july22,
              onClear: _noop,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Secondary variant',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateInputGroup(
              value: _july22,
              variant: HeroDateInputGroupVariant.secondary,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateInputGroup(value: _july22, enabled: false),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'DatePicker',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDatePicker(value: _july22, onChanged: _noopDate),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDatePicker(
              value: _july22,
              label: 'Start date',
              helperText: 'Pick the first day.',
              onChanged: _noopDate,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Empty',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDatePicker(label: 'Start date', onChanged: _noopDate),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDatePicker(
              value: _july22,
              enabled: false,
              onChanged: _noopDate,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: _DatePickerDemo(),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'DateRangePicker',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateRangePicker(
              value: _julyRange,
              onChanged: _noopRange,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateRangePicker(
              value: _julyRange,
              label: 'Trip dates',
              helperText: 'Pick the first and last day.',
              onChanged: _noopRange,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Empty',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateRangePicker(label: 'Trip dates', onChanged: _noopRange),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroDateRangePicker(
              value: _julyRange,
              enabled: false,
              onChanged: _noopRange,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: _DateRangePickerDemo(),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'RangeCalendar',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroRangeCalendar(onChanged: _noopRange),
          ),
        ),
        WidgetbookUseCase(
          name: 'With range',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroRangeCalendar(value: _julyRange, onChanged: _noopRange),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroRangeCalendar(
              value: _julyRange,
              enabled: false,
              onChanged: _noopRange,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: _RangeCalendarDemo(),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Calendar',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendar(onChanged: _noopDate),
          ),
        ),
        WidgetbookUseCase(
          name: 'With value',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendar(value: _july22, onChanged: _noopDate),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendar(
              value: _july22,
              enabled: false,
              onChanged: _noopDate,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: _CalendarDemo(),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'CalendarYearPicker',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendarYearPicker(
              selectedYear: 2025,
              onYearSelected: _noopYear,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With initial year',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendarYearPicker(
              initialYear: 2030,
              selectedYear: 2025,
              onYearSelected: _noopYear,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => Padding(
            padding: const EdgeInsets.all(16),
            child: HeroCalendarYearPicker(
              selectedYear: 2025,
              enabled: false,
              onYearSelected: _noopYear,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const Padding(
            padding: EdgeInsets.all(16),
            child: _YearPickerDemo(),
          ),
        ),
      ],
    ),
  ];
}

final DateTime _july22 = DateTime(2025, 7, 22);

final (DateTime?, DateTime?) _julyRange = (DateTime(2025, 7, 10), DateTime(2025, 7, 22));

void _noop() {}
void _noopDate(DateTime _) {}
void _noopYear(int _) {}
void _noopRange((DateTime?, DateTime?) _) {}

class _DatePickerDemo extends StatefulWidget {
  const _DatePickerDemo();

  @override
  State<_DatePickerDemo> createState() => _DatePickerDemoState();
}

class _DatePickerDemoState extends State<_DatePickerDemo> {
  DateTime? _value;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        HeroDatePicker(value: _value, onChanged: (d) => setState(() => _value = d)),
        const SizedBox(height: 8),
        Text(
          _value == null
              ? 'No date selected'
              : 'Selected: ${_value!.year}-${_value!.month}-${_value!.day}',
          style: const TextStyle(fontSize: 14),
        ),
      ],
    );
  }
}

class _DateRangePickerDemo extends StatefulWidget {
  const _DateRangePickerDemo();

  @override
  State<_DateRangePickerDemo> createState() => _DateRangePickerDemoState();
}

class _DateRangePickerDemoState extends State<_DateRangePickerDemo> {
  (DateTime?, DateTime?)? _range;

  @override
  Widget build(BuildContext context) {
    final (start, end) = _range ?? (null, null);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        HeroDateRangePicker(
          value: _range,
          onChanged: (r) => setState(() => _range = r),
        ),
        const SizedBox(height: 8),
        Text(
          'Range: ${start?.day ?? '--'} – ${end?.day ?? '--'}',
          style: const TextStyle(fontSize: 14),
        ),
      ],
    );
  }
}

class _RangeCalendarDemo extends StatefulWidget {
  const _RangeCalendarDemo();

  @override
  State<_RangeCalendarDemo> createState() => _RangeCalendarDemoState();
}

class _RangeCalendarDemoState extends State<_RangeCalendarDemo> {
  (DateTime?, DateTime?)? _range;

  @override
  Widget build(BuildContext context) {
    return HeroRangeCalendar(
      value: _range,
      onChanged: (r) => setState(() => _range = r),
    );
  }
}

class _CalendarDemo extends StatefulWidget {
  const _CalendarDemo();

  @override
  State<_CalendarDemo> createState() => _CalendarDemoState();
}

class _CalendarDemoState extends State<_CalendarDemo> {
  DateTime? _value;

  @override
  Widget build(BuildContext context) {
    return HeroCalendar(
      value: _value,
      onChanged: (d) => setState(() => _value = d),
    );
  }
}

class _YearPickerDemo extends StatefulWidget {
  const _YearPickerDemo();

  @override
  State<_YearPickerDemo> createState() => _YearPickerDemoState();
}

class _YearPickerDemoState extends State<_YearPickerDemo> {
  int? _year;

  @override
  Widget build(BuildContext context) {
    return HeroCalendarYearPicker(
      selectedYear: _year,
      initialYear: _year,
      onYearSelected: (y) => setState(() => _year = y),
    );
  }
}

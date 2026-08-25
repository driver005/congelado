import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// HeroTable use cases mirroring the HeroUI v3 table storybook: default,
/// long content, row selection, secondary variant, empty state and footer.
List<WidgetbookNode> tableUseCases() {
  return [
    WidgetbookComponent(
      name: 'Table',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => SizedBox(
            width: 640,
            child: HeroTable(
              columns: _defaultColumns,
              rows: _defaultRows,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With long content',
          builder: (context) => SizedBox(
            width: 640,
            child: HeroTable(
              columns: _longColumns,
              rows: _longRows,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Row selection',
          builder: (context) => const _SelectableHeroTable(),
        ),
        WidgetbookUseCase(
          name: 'Secondary',
          builder: (context) => SizedBox(
            width: 640,
            child: HeroTable(
              variant: HeroTableVariant.secondary,
              columns: _defaultColumns,
              rows: _defaultRows,
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Empty',
          builder: (context) => SizedBox(
            width: 640,
            child: HeroTable(
              columns: _defaultColumns,
              rows: [],
              emptyMessage: 'No matching records.',
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With footer',
          builder: (context) => SizedBox(
            width: 640,
            child: HeroTable(
              columns: _defaultColumns,
              rows: _defaultRows,
              footer: _TableFooter(),
            ),
          ),
        ),
      ],
    ),
  ];
}

final _defaultColumns = [
  HeroTableColumn('Name'),
  HeroTableColumn('Role'),
  HeroTableColumn('Status'),
  HeroTableColumn('Price', numeric: true),
];

const _defaultRows = [
  [
    Text('Tony Reichert'),
    Text('CEO'),
    HeroBadge(label: 'Active', color: HeroBadgeColor.success, size: HeroBadgeSize.sm),
    Text('\$129.00'),
  ],
  [
    Text('Zoey Lang'),
    Text('Technical Lead'),
    HeroBadge(label: 'Paused', color: HeroBadgeColor.warning, size: HeroBadgeSize.sm),
    Text('\$98.00'),
  ],
  [
    Text('Jane Fisher'),
    Text('Front-end Developer'),
    HeroBadge(label: 'Active', color: HeroBadgeColor.success, size: HeroBadgeSize.sm),
    Text('\$89.00'),
  ],
  [
    Text('William Howard'),
    Text('Back-end Developer'),
    HeroBadge(label: 'Vacational', color: HeroBadgeColor.danger, size: HeroBadgeSize.sm),
    Text('\$109.00'),
  ],
  [
    Text('Emily Jones'),
    Text('Design Lead'),
    HeroBadge(label: 'Active', color: HeroBadgeColor.success, size: HeroBadgeSize.sm),
    Text('\$118.00'),
  ],
];

final _longColumns = [
  HeroTableColumn('Component'),
  HeroTableColumn('Notes'),
];

const _longRows = [
  [
    Text('Accordion'),
    Text(
      'A vertical list of panels where each panel can be expanded or '
      'collapsed to reveal the content associated with that panel, helping '
      'users manage large amounts of information in a compact space.',
    ),
  ],
  [
    Text('Table'),
    Text(
      'A data display component for structured tabular data. Rows and cells '
      'wrap long content naturally and keep the header band pinned above the '
      'scrolling body, mirroring the HeroUI v3 table styling exactly.',
    ),
  ],
  [
    Text('Tooltip'),
    Text(
      'A popup that displays information related to an element when the '
      'element receives keyboard focus or the mouse hovers over it, '
      'typically shown after a short delay.',
    ),
  ],
];

/// Row-selection demo — tapping a row toggles its selected state
/// (`data-selected` -> cells `bg-surface/10`).
class _SelectableHeroTable extends StatefulWidget {
  const _SelectableHeroTable();

  @override
  State<_SelectableHeroTable> createState() => _SelectableHeroTableState();
}

class _SelectableHeroTableState extends State<_SelectableHeroTable> {
  final Set<int> _selected = <int>{};

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 640,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          HeroTable(
            columns: _defaultColumns,
            rows: _defaultRows,
            selectedRows: _selected,
            onRowTap: (index) => setState(() {
              if (!_selected.add(index)) {
                _selected.remove(index);
              }
            }),
          ),
          const SizedBox(height: 12),
          Text(
            _selected.isEmpty
                ? 'Tap a row to select it.'
                : '${_selected.length} row(s) selected: '
                    '${_selected.toList()..sort()}',
            style: TextStyle(
              fontSize: HeroTokens.typeSm.resolve(context).fontSize,
              color: HeroTokens.colorMuted.resolve(context),
            ),
          ),
        ],
      ),
    );
  }
}

/// Footer strip (`flex items-center px-4 py-2.5`) below the table.
class _TableFooter extends StatelessWidget {
  const _TableFooter();

  @override
  Widget build(BuildContext context) {
    return Text(
      'Showing 1–5 of 5 results',
      style: TextStyle(
        fontSize: HeroTokens.typeSm.resolve(context).fontSize,
        color: HeroTokens.colorMuted.resolve(context),
      ),
    );
  }
}

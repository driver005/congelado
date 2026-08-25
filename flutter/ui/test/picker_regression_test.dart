import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

/// Regression tests for the picker family:
/// - select/dropdown selected items used stretching `Positioned` children in a
///   Stack inside an unbounded popup Column — that crashed with "A Stack
///   requires bounded constraints from its parent".
/// - the color picker opened as a modal dialog; it must be an anchored
///   popover now.
/// - popups must be as wide as the trigger: a stretch Column inside the loose
///   overlay painted a full-width (dark) panel.
Widget host(Widget child) => HeroScope(
      theme: HeroTheme.light,
      child: MaterialApp(
        home: Scaffold(body: SingleChildScrollView(child: child)),
      ),
    );

void main() {
  testWidgets('select with a selected item builds (Stack bounds)',
      (tester) async {
    await tester.pumpWidget(host(
      const HeroSelect<String>(
        placeholder: 'Pick one',
        value: 'b',
        items: [
          HeroSelectItem(value: 'a', label: 'Alpha'),
          HeroSelectItem(value: 'b', label: 'Beta'),
        ],
      ),
    ));
    expect(tester.takeException(), isNull);
  });

  testWidgets('dropdown with a selected item opens without crashing',
      (tester) async {
    await tester.pumpWidget(host(
      HeroDropdown(
        items: const [
          HeroDropdownItem(label: 'Alpha', selected: true),
          HeroDropdownItem(label: 'Beta'),
        ],
        trigger: (open) => HeroButton(label: 'Open', onPressed: open),
      ),
    ));
    expect(tester.takeException(), isNull);
    await tester.tap(find.text('Open'));
    await tester.pumpAndSettle();
    expect(tester.takeException(), isNull);
    expect(find.text('Alpha'), findsOneWidget);
  });

  testWidgets('select popup is as wide as the input', (tester) async {
    await tester.pumpWidget(host(
      const Center(
        child: SizedBox(
          width: 200,
          child: HeroSelect<String>(
            placeholder: 'Pick',
            value: 'b',
            items: [
              HeroSelectItem(value: 'a', label: 'Alpha'),
              HeroSelectItem(
                value: 'b',
                label: 'Beta with a much longer label than the field',
              ),
            ],
          ),
        ),
      ),
    ));
    await tester.tap(find.text('Beta with a much longer label than the field'));
    await tester.pumpAndSettle();
    expect(tester.takeException(), isNull);
    final box =
        tester.renderObject(find.byType(HeroListBox)) as RenderBox;
    expect(box.size.width, 200);
    // Items fill the popup width (minus list-box + item padding): no
    // unpainted "black" gap next to the label in dark themes.
    final itemRow = tester.renderObject(
      find.ancestor(of: find.text('Alpha'), matching: find.byType(Row)).first,
    ) as RenderBox;
    // 200 - 6px list-box pad x2 - 10px item pad x2 = 168.
    expect(itemRow.size.width, closeTo(168, 1));
  });

  testWidgets('dropdown menu is as wide as the trigger and never throws',
      (tester) async {
    await tester.pumpWidget(host(
      HeroDropdown(
        items: const [
          HeroDropdownItem(label: 'Alpha', selected: true),
          HeroDropdownItem(label: 'Beta'),
        ],
        trigger: (open) => HeroButton(label: 'Open', onPressed: open),
      ),
    ));
    final triggerBox = tester.renderObject(find.byType(HeroButton)) as RenderBox;
    await tester.tap(find.text('Open'));
    await tester.pumpAndSettle();
    expect(tester.takeException(), isNull);
    // The menu is the ConstrainedBox pinned to the trigger width (no
    // full-width dark panel, no clamp ArgumentError when the trigger is as
    // wide as the screen).
    final widthBox = find.byWidgetPredicate((w) =>
        w is ConstrainedBox &&
        w.constraints.maxWidth == triggerBox.size.width);
    expect(widthBox, findsOneWidget);
    final menuBox = tester.renderObject(widthBox) as RenderBox;
    expect(menuBox.size.width, closeTo(triggerBox.size.width, 1));
    expect(menuBox.size.width, lessThanOrEqualTo(triggerBox.size.width + 1));
  });

  testWidgets('color picker opens as an anchored popover, resolves on Done',
      (tester) async {
    Color? result;
    await tester.pumpWidget(host(
      Builder(
        builder: (context) => HeroButton(
          label: 'Pick color',
          onPressed: () async {
            result = await showHeroColorPicker(
              context,
              initialColor: const Color(0xFF0485F7),
            );
          },
        ),
      ),
    ));
    await tester.tap(find.text('Pick color'));
    await tester.pumpAndSettle();
    expect(tester.takeException(), isNull);
    // Anchored popover, not a modal dialog: the panel is present and there is
    // no route push (the button is still in the tree).
    expect(find.byType(HeroPopoverPanel), findsOneWidget);
    expect(find.text('Pick color'), findsOneWidget);
    await tester.ensureVisible(find.text('Done'));
    await tester.pumpAndSettle();
    await tester.tap(find.text('Done'));
    await tester.pumpAndSettle();
    expect(result, isNotNull);
    expect(tester.takeException(), isNull);
  });

  testWidgets('popup panel uses the INNER theme, not the outer platform scope',
      (tester) async {
    // Like the real widgetbook: an outer HeroScope follows the platform
    // brightness (dark on CI), the ThemeAddon installs a LIGHTER inner
    // HeroScope. The popup entry must resolve tokens against the inner scope
    // — otherwise the panel paints the outer (dark) overlay: the "black
    // panel" bug.
    await tester.pumpWidget(HeroScope(
      theme: HeroTheme.dark,
      child: MaterialApp(
        home: Scaffold(
          body: HeroScope(
            theme: HeroTheme.light,
            child: Builder(
              builder: (context) => const Center(
                child: SizedBox(
                  width: 200,
                  child: HeroSelect<String>(
                    placeholder: 'Pick',
                    value: 'b',
                    items: [
                      HeroSelectItem(value: 'a', label: 'Alpha'),
                      HeroSelectItem(value: 'b', label: 'Beta'),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    ));
    await tester.tap(find.text('Beta'));
    await tester.pumpAndSettle();
    expect(tester.takeException(), isNull);
    final panelFinder = find.ancestor(
      of: find.byType(HeroListBox),
      matching: find.byWidgetPredicate((w) =>
          w is Container &&
          w.decoration is BoxDecoration &&
          (w.decoration as BoxDecoration).color != null),
    );
    final panel = tester.widget<Container>(panelFinder.first);
    final color = (panel.decoration as BoxDecoration).color;
    // Light overlay (white), NOT the outer dark scope's 0xFF18181B.
    expect(color, const Color(0xFFFFFFFF));
  });
}

import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:widgetbook/widgetbook.dart';

// Widgetbook use-case registry is catalogue-internal, not exported from the
// package barrel.
// ignore_for_file: avoid_relative_lib_imports
import '../lib/src/widgetbook/use_cases/table_use_cases.dart';

/// Regression: every table use case must build inside an unbounded-height
/// context (the widgetbook use-case frame). HeroTable used to crash with
/// "BoxConstraints forces an infinite height" because rows stretched and the
/// cell focus ring used a Stack/Positioned.fill — both demand a bounded
/// height.
void main() {
  testWidgets('every table use case builds in an unbounded frame',
      (tester) async {
    BuildContext? ctx;
    await tester.pumpWidget(Builder(builder: (c) {
      ctx = c;
      return const SizedBox();
    }));
    for (final node in tableUseCases()) {
      final component = node as WidgetbookComponent;
      for (final useCase in component.useCases) {
        final widget = useCase.build(ctx!);
        await tester.pumpWidget(HeroScope(
          theme: HeroTheme.light,
          child: MaterialApp(
            home: Scaffold(
              body: SingleChildScrollView(child: widget),
            ),
          ),
        ));
        await tester.pump();
        expect(tester.takeException(), isNull, reason: useCase.name);
      }
    }
  });
}

import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('PluginWidgetFactory', () {
    testWidgets('renders text, badge, button and fires action', (tester) async {
      final actions = <String>[];
      // ignore: prefer_const_constructors — the onAction closure captures
      // `actions`, so the factory cannot be const.
      final factory = PluginWidgetFactory(onAction: actions.add);
      const spec = PluginSpec(
        id: 'a',
        name: 'A',
        slot: PluginSlot.main,
        root: PluginNode(
          type: 'column',
          children: [
            PluginNode(type: 'text', props: {'text': 'Hello'}),
            PluginNode(type: 'badge', props: {'label': 'spec', 'color': 'green'}),
            PluginNode(type: 'button', props: {'label': 'Go', 'action': 'go'}),
          ],
        ),
      );
      await tester.pumpWidget(
        MaterialApp(home: Scaffold(body: factory.build(spec.root))),
      );
      expect(find.text('Hello'), findsOneWidget);
      expect(find.text('spec'), findsOneWidget);
      expect(find.text('Go'), findsOneWidget);
      await tester.tap(find.text('Go'));
      expect(actions, ['go']);
    });

    testWidgets('unknown type renders placeholder, does not throw', (tester) async {
      const factory = PluginWidgetFactory();
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: factory.build(const PluginNode(type: 'warp_drive')),
          ),
        ),
      );
      expect(find.textContaining('[unknown widget'), findsOneWidget);
    });
  });

  group('PluginHost', () {
    testWidgets('renders entries for its slot only', (tester) async {
      final reg = PluginRegistry();
      reg.addSpec(const PluginSpec(
        id: 'main1',
        name: 'Main One',
        slot: PluginSlot.main,
        root: PluginNode(type: 'text', props: {'text': 'MAIN'}),
      ));
      reg.addSpec(const PluginSpec(
        id: 'side1',
        name: 'Side One',
        slot: PluginSlot.sidebar,
        root: PluginNode(type: 'text', props: {'text': 'SIDE'}),
      ));
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: PluginHost(registry: reg, slot: PluginSlot.main),
          ),
        ),
      );
      expect(find.text('MAIN'), findsOneWidget);
      expect(find.text('SIDE'), findsNothing);

      reg.addSpec(const PluginSpec(
        id: 'main2',
        name: 'Main Two',
        slot: PluginSlot.main,
        root: PluginNode(type: 'text', props: {'text': 'MAIN2'}),
      ));
      await tester.pump();
      expect(find.text('MAIN2'), findsOneWidget);

      reg.remove('main1');
      await tester.pump();
      expect(find.text('MAIN'), findsNothing);
      reg.dispose();
    });
  });
}

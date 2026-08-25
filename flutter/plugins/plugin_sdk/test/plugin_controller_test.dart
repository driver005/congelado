import 'dart:io';

import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('PluginController (IO)', () {
    late Directory tmp;
    late PluginController controller;

    setUp(() {
      tmp = Directory.systemTemp.createTempSync('plugins_test');
      controller = PluginController(pluginsDir: tmp.path);
    });

    tearDown(() {
      controller.dispose();
      tmp.deleteSync(recursive: true);
    });

    test('loads specs from directory and reconciles removals', () {
      File('${tmp.path}/hello/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: hello\nname: Hello\nwidgets:\n  type: text\n  props:\n    text: Hi\n');

      expect(controller.reload(), 1);
      expect(controller.specs.containsKey('hello'), isTrue);
      expect(controller.registry.contains('hello'), isTrue);

      // A newly dropped plugin appears on the next reload.
      File('${tmp.path}/metrics/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: metrics\nslot: sidebar\nwidgets:\n  type: badge\n  props:\n    label: M\n');
      expect(controller.reload(), 2);
      expect(controller.registry.contains('metrics'), isTrue);

      // Deleting the directory removes it on reload.
      Directory('${tmp.path}/hello').deleteSync(recursive: true);
      expect(controller.reload(), 1);
      expect(controller.registry.contains('hello'), isFalse);
      expect(controller.registry.contains('metrics'), isTrue);
    });

    test('code plugins survive reload', () {
      controller.register(_FakeCodePlugin());
      expect(controller.registry.contains('fake'), isTrue);
      controller.reload();
      expect(controller.registry.contains('fake'), isTrue);
    });

    test('broken yaml is skipped, does not kill scan', () {
      File('${tmp.path}/bad/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('slot: not-a-slot\n');
      File('${tmp.path}/good/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: good\nwidgets:\n  type: text\n');
      expect(controller.reload(), 1);
      expect(controller.specs.containsKey('good'), isTrue);
      expect(controller.specs.containsKey('bad'), isFalse);
    });

    test('available lists discoverable specs without activating them', () {
      File('${tmp.path}/a/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: a\nwidgets:\n  type: text\n');
      File('${tmp.path}/b/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: b\nwidgets:\n  type: text\n');
      expect(controller.available(), hasLength(2));
      expect(controller.registry.entries, isEmpty);
      expect(controller.isActive('a'), isFalse);
    });

    test('activate/deactivate manage the registry', () {
      File('${tmp.path}/a/plugin.yaml')
        ..createSync(recursive: true)
        ..writeAsStringSync('id: a\nwidgets:\n  type: text\n');
      final spec = controller.available().single;
      controller.activate(spec);
      expect(controller.isActive('a'), isTrue);
      expect(controller.registry.contains('a'), isTrue);
      expect(controller.deactivate('a'), isTrue);
      expect(controller.isActive('a'), isFalse);
      expect(controller.deactivate('a'), isFalse);
    });
  });
}

class _FakeCodePlugin extends FlutterPlugin {
  @override
  String get id => 'fake';

  @override
  String get name => 'Fake';

  @override
  Widget build(BuildContext context, PluginSlot slot) => const SizedBox.shrink();
}

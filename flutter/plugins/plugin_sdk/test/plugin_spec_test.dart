import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('PluginSpecLoader', () {
    const loader = PluginSpecLoader();

    test('parses a full spec', () {
      const yaml = '''
id: example_hello
name: Hello Plugin
version: 0.1.0
slot: sidebar
widgets:
  type: card
  props:
    title: Hello
  children:
    - type: text
      props:
        text: Hi there
    - type: row
      children:
        - type: button
          props:
            label: Go
            action: hello.go
''';
      final spec = loader.parse(yaml);
      expect(spec.id, 'example_hello');
      expect(spec.name, 'Hello Plugin');
      expect(spec.slot, PluginSlot.sidebar);
      expect(spec.root.type, 'card');
      expect(spec.root.propString('title'), 'Hello');
      expect(spec.root.children, hasLength(2));
      final button = spec.root.children[1].children.single;
      expect(button.type, 'button');
      expect(button.propString('action'), 'hello.go');
    });

    test('parses author, description and tags', () {
      const yaml = '''
id: demo
name: Demo
author: Jane Doe
description: A demo plugin.
tags: [a, b, c]
slot: main
widgets:
  type: text
''';
      final spec = loader.parse(yaml);
      expect(spec.author, 'Jane Doe');
      expect(spec.description, 'A demo plugin.');
      expect(spec.tags, ['a', 'b', 'c']);
      expect(spec.matches('jane'), isTrue);
      expect(spec.matches('demo plugin'), isTrue);
      expect(spec.matches('b'), isTrue);
      expect(spec.matches('zzz'), isFalse);
      expect(spec.matches(''), isTrue);
    });

    test('uses fallback id when id missing', () {
      const yaml = '''
name: X
widgets:
  type: text
  props:
    text: x
''';
      expect(loader.parse(yaml, fallbackId: 'dir_a').id, 'dir_a');
    });

    test('throws on unknown slot', () {
      const yaml = 'id: a\nslot: nope\nwidgets:\n  type: text\n';
      expect(() => loader.parse(yaml), throwsFormatException);
    });

    test('parses bundled map', () {
      final specs = loader.parseBundled({
        'a': 'id: a\nwidgets:\n  type: text\n  props:\n    text: A\n',
        'b': 'id: b\nslot: footer\nwidgets:\n  type: badge\n  props:\n    label: B\n',
      });
      expect(specs, hasLength(2));
      expect(specs[0].slot, PluginSlot.main);
      expect(specs[1].slot, PluginSlot.footer);
    });
  });

  group('PluginRegistry', () {
    test('registers, replaces, removes, clears', () {
      final reg = PluginRegistry();
      reg.addSpec(const PluginSpec(
        id: 'a',
        name: 'A',
        slot: PluginSlot.main,
        root: PluginNode(type: 'text'),
      ));
      expect(reg.contains('a'), isTrue);
      expect(reg.entriesFor(PluginSlot.main), hasLength(1));
      expect(reg.entriesFor(PluginSlot.sidebar), isEmpty);

      reg.addSpec(const PluginSpec(
        id: 'b',
        name: 'B',
        slot: PluginSlot.sidebar,
        root: PluginNode(type: 'text'),
      ));
      expect(reg.entriesFor(PluginSlot.sidebar), hasLength(1));

      expect(reg.remove('a'), isTrue);
      expect(reg.contains('a'), isFalse);
      reg.clear();
      expect(reg.entries, isEmpty);
      reg.dispose();
    });

    test('notifies listeners on mutation', () {
      final reg = PluginRegistry();
      var notified = 0;
      reg.addListener(() => notified++);
      reg.addSpec(const PluginSpec(
        id: 'a',
        name: 'A',
        slot: PluginSlot.main,
        root: PluginNode(type: 'text'),
      ));
      reg.remove('a');
      expect(notified, 2);
      reg.dispose();
    });
  });
}

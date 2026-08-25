import 'dart:convert';

import 'package:yaml/yaml.dart';

import 'plugin_slot.dart';
import 'plugin_spec.dart';

/// Parses `plugin.yaml` files. Pure Dart — no `dart:io`, so it compiles on
/// every platform including web.
class PluginSpecLoader {
  const PluginSpecLoader();

  /// Parses the content of one `plugin.yaml`.
  PluginSpec parse(String yamlSource, {String? fallbackId}) {
    final doc = loadYaml(yamlSource);
    if (doc is! YamlMap) {
      throw const FormatException('plugin.yaml must be a YAML mapping');
    }
    final id = doc['id'] as String? ?? fallbackId;
    if (id == null || id.isEmpty) {
      throw const FormatException('plugin.yaml missing "id"');
    }
    final slot = switch (doc['slot'] as String? ?? 'main') {
      'main' => PluginSlot.main,
      'sidebar' => PluginSlot.sidebar,
      'toolbar' => PluginSlot.toolbar,
      'footer' => PluginSlot.footer,
      final other => throw FormatException('unknown slot "$other"'),
    };
    final tags = switch (doc['tags']) {
      YamlList list => list
          .map((t) => t.toString())
          .where((t) => t.isNotEmpty)
          .toList(),
      _ => const <String>[],
    };
    return PluginSpec(
      id: id,
      name: doc['name'] as String? ?? id,
      version: doc['version'] as String? ?? '0.0.0',
      author: doc['author'] as String?,
      description: doc['description'] as String?,
      tags: tags,
      slot: slot,
      root: _parseNode(doc['widgets']),
    );
  }

  PluginNode _parseNode(Object? raw) {
    if (raw is! YamlMap) {
      throw const FormatException('each widget must be a mapping');
    }
    final type = raw['type'] as String?;
    if (type == null || type.isEmpty) {
      throw const FormatException('widget missing "type"');
    }
    final props = <String, Object?>{};
    final rawProps = raw['props'];
    if (rawProps is YamlMap) {
      rawProps.forEach((k, v) => props[k.toString()] = _scalar(v));
    }
    final children = <PluginNode>[];
    final rawChildren = raw['children'];
    if (rawChildren is YamlList) {
      children.addAll(rawChildren.map(_parseNode));
    }
    return PluginNode(type: type, props: props, children: children);
  }

  Object? _scalar(Object? v) {
    if (v is YamlScalar) return v.value;
    if (v is num || v is bool || v is String || v == null) return v;
    // Nested maps/lists in props are not supported yet — keep spec flat.
    return v.toString();
  }

  /// Parses spec plugins bundled as assets. The map keys are plugin ids, the
  /// values the raw yaml strings (used on web, where `dart:io` is
  /// unavailable).
  List<PluginSpec> parseBundled(Map<String, String> bundledYaml) {
    return bundledYaml.entries
        .map((e) => parse(e.value, fallbackId: e.key))
        .toList();
  }
}

/// Encodes [PluginSpec] back to yaml — used by tests and debugging.
String encodeSpec(PluginSpec spec) {
  String indent(int n) => '  ' * n;
  final buf = StringBuffer()
    ..writeln('id: ${spec.id}')
    ..writeln('name: ${spec.name}')
    ..writeln('version: ${spec.version}')
    ..writeln('slot: ${spec.slot.name}');
  void node(PluginNode n, int depth) {
    buf
      ..write('${indent(depth)}- type: ${n.type}\n')
      ..write('${indent(depth)}  props: ${jsonEncode(n.props)}\n');
    if (n.children.isNotEmpty) {
      buf.write('${indent(depth)}  children:\n');
      for (final c in n.children) {
        node(c, depth + 1);
      }
    }
  }

  buf.writeln('widgets:');
  node(spec.root, 1);
  return buf.toString();
}

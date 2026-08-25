import 'plugin_slot.dart';

/// A declarative plugin: a `plugin.yaml` describing a widget tree plus
/// metadata (author, description, tags) shown in plugin-manager UIs.
///
/// Specs are pure data — no Dart code — so they can be loaded from disk at
/// runtime and rendered by the host's widget factory. A spec plugin targets a
/// single [slot].
class PluginSpec {
  const PluginSpec({
    required this.id,
    required this.name,
    required this.slot,
    this.version = '0.0.0',
    this.author,
    this.description,
    this.tags = const [],
    required this.root,
  });

  final String id;
  final String name;
  final String version;

  /// Displayed by plugin-manager UIs. Absent for specs authored without it.
  final String? author;
  final String? description;

  /// Free-form search tags (e.g. `metrics`, `sidebar`).
  final List<String> tags;

  final PluginSlot slot;
  final PluginNode root;

  /// Whether [query] matches this plugin's id/name/author/description/tags.
  bool matches(String query) {
    final q = query.trim().toLowerCase();
    if (q.isEmpty) return true;
    bool has(String? s) => s?.toLowerCase().contains(q) ?? false;
    return has(id) ||
        has(name) ||
        has(author) ||
        has(description) ||
        tags.any((t) => t.toLowerCase().contains(q));
  }
}

/// One node of a spec widget tree. [type] names a factory-registered widget
/// kind (e.g. `text`, `button`, `card`, `row`, `column`); [props] are
/// type-specific string/number/bool values; [children] nest the tree.
class PluginNode {
  const PluginNode({
    required this.type,
    this.props = const {},
    this.children = const [],
  });

  final String type;
  final Map<String, Object?> props;
  final List<PluginNode> children;

  /// Typed prop accessors used by the widget factory.
  String? propString(String key) => props[key] as String?;

  double? propDouble(String key) {
    final v = props[key];
    if (v is num) return v.toDouble();
    return v is String ? double.tryParse(v) : null;
  }

  bool? propBool(String key) => props[key] as bool?;

  List<PluginNode> get childNodes => children;
}

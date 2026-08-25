import 'package:flutter/foundation.dart';

import 'plugin.dart';
import 'plugin_slot.dart';
import 'plugin_spec.dart';

/// One active contribution: either a code plugin ([plugin]) or a loaded
/// declarative spec ([spec]). Exactly one of the two is non-null.
class PluginEntry {
  const PluginEntry.code(FlutterPlugin this.plugin) : spec = null;

  const PluginEntry.spec(PluginSpec this.spec) : plugin = null;

  final FlutterPlugin? plugin;
  final PluginSpec? spec;

  String get id => plugin?.id ?? spec!.id;
  String get name => plugin?.name ?? spec!.name;
  bool get isCode => plugin != null;

  PluginSlot get slot => plugin != null ? PluginSlot.main : spec!.slot;

  @override
  String toString() => 'PluginEntry($id, ${isCode ? 'code' : 'spec'})';
}

/// Registry of active plugins (code + spec) and their slots.
///
/// Extends [ChangeNotifier]: hosts and chrome listen and re-render when
/// plugins are registered/unregistered (e.g. after a directory rescan).
class PluginRegistry extends ChangeNotifier {
  final Map<String, PluginEntry> _entries = {};

  /// All registered entries, insertion-ordered.
  List<PluginEntry> get entries => List.unmodifiable(_entries.values);

  /// Entries targeting [slot].
  List<PluginEntry> entriesFor(PluginSlot slot) =>
      entries.where((e) => e.slot == slot).toList();

  bool contains(String id) => _entries.containsKey(id);

  PluginEntry? entry(String id) => _entries[id];

  /// Registers a code plugin. Replaces any existing entry with the same id.
  void register(FlutterPlugin plugin) {
    _entries[plugin.id] = PluginEntry.code(plugin);
    notifyListeners();
  }

  /// Loads a declarative spec. Replaces any existing entry with the same id.
  void addSpec(PluginSpec spec) {
    _entries[spec.id] = PluginEntry.spec(spec);
    notifyListeners();
  }

  /// Removes a plugin by id (code or spec).
  bool remove(String id) {
    final removed = _entries.remove(id) != null;
    if (removed) notifyListeners();
    return removed;
  }

  /// Removes every entry; used on a full reload sweep.
  void clear() {
    if (_entries.isEmpty) return;
    _entries.clear();
    notifyListeners();
  }
}

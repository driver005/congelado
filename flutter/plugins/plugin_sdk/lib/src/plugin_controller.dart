import 'dart:async';

import 'package:flutter/foundation.dart';

import 'plugin.dart';
import 'plugin_dir_scanner_platform.dart';
import 'plugin_registry.dart';
import 'plugin_spec.dart';
import 'plugin_spec_loader.dart';

/// Owns the [PluginRegistry] and the runtime reload machinery.
///
/// Typical lifecycle:
///
/// ```dart
/// final controller = PluginController(pluginsDir: 'flutter/plugins');
/// controller.register(MyCodePlugin());
/// controller.reload();            // scan dir, add/remove spec plugins
/// controller.startWatching();     // optional periodic rescan
/// ```
///
/// [PluginController] is a [ChangeNotifier]; it notifies after every
/// registry mutation, so [PluginHost] re-renders automatically.
///
/// Plugin-manager flows (list available, activate/deactivate) use
/// [available], [activate] and [deactivate]; they never auto-register
/// anything, so a manager UI can search and pick.
class PluginController extends ChangeNotifier {
  PluginController({
    required this.pluginsDir,
    PluginSpecLoader? loader,
    Map<String, String> bundledYaml = const {},
  })  : _loader = loader ?? const PluginSpecLoader(),
        _bundledYaml = bundledYaml;

  /// Directory scanned for spec plugins (may be relative to the CWD).
  final String pluginsDir;

  /// Spec plugins bundled as yaml (web fallback — no filesystem). The keys are
  /// plugin ids, the values raw `plugin.yaml` strings.
  final Map<String, String> _bundledYaml;

  final PluginSpecLoader _loader;

  final PluginRegistry _registry = PluginRegistry();

  Timer? _watchTimer;

  /// The registry backing this controller.
  PluginRegistry get registry => _registry;

  /// Active spec plugins (the ones loaded from disk/bundles), by id.
  Map<String, PluginSpec> get specs => Map.unmodifiable(_specs);
  final Map<String, PluginSpec> _specs = {};

  /// Spec plugins discoverable on this platform but NOT necessarily active:
  /// the filesystem scan on IO platforms, or the bundled set on web.
  /// Includes plugins already active — callers filter with [isActive].
  List<PluginSpec> available() {
    if (pluginDirScanner.supportsFileSystem) {
      return pluginDirScanner.scan(pluginsDir);
    }
    return _loader.parseBundled(_bundledYaml);
  }

  /// Whether a spec plugin with [id] is currently active.
  bool isActive(String id) => _registry.contains(id);

  /// Activates a spec plugin (from [available]) and notifies.
  void activate(PluginSpec spec) {
    _specs[spec.id] = spec;
    _registry.addSpec(spec);
    notifyListeners();
  }

  /// Deactivates a spec plugin by id and notifies. Returns false when it was
  /// not active.
  bool deactivate(String id) {
    if (!_registry.contains(id)) return false;
    _specs.remove(id);
    _registry.remove(id);
    notifyListeners();
    return true;
  }

  /// Registers a code plugin and notifies.
  void register(FlutterPlugin plugin) {
    _registry.register(plugin);
    notifyListeners();
  }

  /// Unregisters a code plugin by id and notifies.
  bool unregister(String id) {
    final removed = _registry.remove(id);
    if (removed) notifyListeners();
    return removed;
  }

  /// Rescans [pluginsDir] and reconciles the registry: newly seen specs are
  /// added, vanished ones removed. Code plugins are never touched.
  ///
  /// Returns the number of spec plugins active afterwards. Safe to call
  /// repeatedly — this is what "extend the UI while running" uses.
  int reload() {
    final found = <String, PluginSpec>{
      for (final spec in pluginDirScanner.scan(pluginsDir)) spec.id: spec,
    };
    // Drop specs whose directory vanished.
    for (final id in _specs.keys.toList()) {
      if (!found.containsKey(id)) {
        _specs.remove(id);
        _registry.remove(id);
      }
    }
    // Add / refresh specs that appeared or changed.
    for (final spec in found.values) {
      _specs[spec.id] = spec;
      _registry.addSpec(spec);
    }
    notifyListeners();
    return _specs.length;
  }

  /// Loads spec plugins from bundled yaml (web fallback, or tests).
  void loadBundled(Map<String, String> bundledYaml) {
    for (final spec in _loader.parseBundled(bundledYaml)) {
      _specs[spec.id] = spec;
      _registry.addSpec(spec);
    }
    notifyListeners();
  }

  /// Starts a periodic [reload] every [interval] (default 5s). Use to pick up
  /// new plugin directories without manual action. No-op on web (scanner
  /// returns nothing); harmless otherwise.
  void startWatching({Duration interval = const Duration(seconds: 5)}) {
    _watchTimer ??= Timer.periodic(interval, (_) => reload());
  }

  void stopWatching() {
    _watchTimer?.cancel();
    _watchTimer = null;
  }

  @override
  void dispose() {
    stopWatching();
    _registry.dispose();
    super.dispose();
  }
}

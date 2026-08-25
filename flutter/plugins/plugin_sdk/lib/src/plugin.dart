import 'package:flutter/widgets.dart';

import 'plugin_slot.dart';

/// A code plugin: an arbitrarily rich widget contribution registered directly
/// with [PluginRegistry] (as opposed to a declarative spec plugin, which is
/// data loaded at runtime).
abstract class FlutterPlugin {
  const FlutterPlugin();

  /// Stable identifier, e.g. `example_clock`.
  String get id;

  /// Human-readable name shown in the host chrome.
  String get name;

  /// Metadata shown by plugin-manager UIs. Defaults to null (hidden).
  String? get author => null;
  String? get description => null;
  String get version => '0.0.0';

  /// Free-form search tags shown by plugin-manager UIs.
  List<String> get tags => const [];

  /// Builds the plugin's contribution for [slot].
  ///
  /// [context] is a BuildContext below [PluginHost], so plugins may resolve
  /// inherited themes, navigate, etc.
  Widget build(BuildContext context, PluginSlot slot);
}

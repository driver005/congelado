import 'plugin_spec.dart';

/// Scans a plugins directory for spec plugins. Platform-split: the IO
/// implementation reads the real filesystem (desktop/mobile); the web
/// implementation has no filesystem and always returns an empty list — hosts
/// on web should feed specs via [PluginSpecLoader.parseBundled] instead.
abstract class PluginDirScanner {
  const PluginDirScanner();

  /// Returns the parsed specs found under [root] (one level deep:
  /// `root/<plugin-id>/plugin.yaml`), or an empty list when [root] does not
  /// exist / is not readable.
  List<PluginSpec> scan(String root);

  /// True when this build can read the real filesystem.
  bool get supportsFileSystem;
}

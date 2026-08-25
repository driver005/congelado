import 'plugin_dir_scanner.dart';
import 'plugin_spec.dart';

/// Web implementation of [PluginDirScanner]: no filesystem access, so it
/// always reports no plugins. Web hosts feed specs via
/// [PluginSpecLoader.parseBundled] (e.g. from bundled assets).
class WebPluginDirScanner extends PluginDirScanner {
  const WebPluginDirScanner();

  @override
  bool get supportsFileSystem => false;

  @override
  List<PluginSpec> scan(String root) => const [];
}

/// Platform-selected scanner instance: no-op on web.
const PluginDirScanner pluginDirScanner = WebPluginDirScanner();

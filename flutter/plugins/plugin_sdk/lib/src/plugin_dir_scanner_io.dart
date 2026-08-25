import 'dart:io';

import 'plugin_dir_scanner.dart';
import 'plugin_spec.dart';
import 'plugin_spec_loader.dart';

/// IO implementation of [PluginDirScanner] — reads the real filesystem.
class IoPluginDirScanner extends PluginDirScanner {
  const IoPluginDirScanner();

  @override
  bool get supportsFileSystem => true;

  @override
  List<PluginSpec> scan(String root) {
    final dir = Directory(root);
    if (!dir.existsSync()) return const [];
    const loader = PluginSpecLoader();
    final specs = <PluginSpec>[];
    for (final entry in dir.listSync(followLinks: false)) {
      if (entry is! Directory) continue;
      final manifest = File('${entry.path}${Platform.pathSeparator}plugin.yaml');
      if (!manifest.existsSync()) continue;
      try {
        specs.add(loader.parse(manifest.readAsStringSync(),
            fallbackId: entry.path.split(Platform.pathSeparator).last));
      } on FormatException catch (e) {
        // One broken plugin must not kill the whole scan.
        stderr.writeln('[plugins] skip ${entry.path}: $e');
      }
    }
    return specs;
  }
}

/// Platform-selected scanner instance: reads the real filesystem.
const PluginDirScanner pluginDirScanner = IoPluginDirScanner();

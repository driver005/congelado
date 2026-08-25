/// Runtime plugin system for the congelado Flutter UI.
///
/// Two plugin kinds:
///
/// 1. **Declarative spec plugins** — a `plugin.yaml` describing a widget tree
///    (type + props + children). Loaded from the plugins directory at runtime
///    by [PluginController]; the tree is rendered by
///    `PluginWidgetFactory`. Adding a new `plugin.yaml` while the app runs and
///    triggering a rescan extends the UI without a rebuild.
/// 2. **Code plugins** — a [FlutterPlugin] implementation registered at
///    startup (or later) via [PluginRegistry.register]. Arbitrary widget code.
///
/// [PluginHost] renders the active contributions of a given [PluginSlot];
/// [PluginController] owns the registry, the directory rescan and the
/// change notification.
library;

export 'src/plugin.dart';
export 'src/plugin_registry.dart';
export 'src/plugin_controller.dart';
export 'src/plugin_host.dart';
export 'src/plugin_slot.dart';
export 'src/plugin_spec.dart';
export 'src/plugin_spec_loader.dart';
export 'src/plugin_widget_factory.dart';
export 'src/plugin_dir_scanner_platform.dart';

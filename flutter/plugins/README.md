# congelado Flutter plugin system

Runtime-extensible UI for the congelado Flutter app (`flutter/ui/`). Plugins
add widgets to the running app without a rebuild.

## Two plugin kinds

| Kind | What it is | Loaded |
| --- | --- | --- |
| **Spec plugin** | Pure data: a `plugin.yaml` describing a widget tree (type + props + children) | From disk at runtime by `PluginController.reload()` |
| **Code plugin** | A Dart `FlutterPlugin` implementation (arbitrary widgets) | Registered at startup (or later) via `PluginController.register()` |

Spec plugins are the "extend while running" story: the controller rescans the
plugins directory, new `plugin.yaml` folders appear as UI immediately — no
Dart code, no rebuild. Code plugins cover anything data can't (state, timers,
navigation, custom painting).

## Layout

```
flutter/plugins/
├── plugin_sdk/            # the framework: congelado_plugin_sdk
│   ├── lib/src/
│   │   ├── plugin.dart              # FlutterPlugin interface
│   │   ├── plugin_spec.dart         # declarative spec model
│   │   ├── plugin_spec_loader.dart  # yaml -> PluginSpec (pure Dart)
│   │   ├── plugin_dir_scanner_*.dart# filesystem scan (IO) / no-op (web)
│   │   ├── plugin_registry.dart     # active plugins by slot, ChangeNotifier
│   │   ├── plugin_controller.dart   # registry + reload + optional watching
│   │   ├── plugin_widget_factory.dart # spec tree -> widgets
│   │   └── plugin_host.dart         # renders one slot, live-updates
│   └── test/              # parsing, registry, controller, host, factory
├── example_hello/         # spec plugin: card + text + button + badge (main)
├── example_metrics/       # spec plugin: sidebar card
└── example_clock/         # code plugin: live ticking clock
```

## Plugin manager UI

The catalogue ships a plugin manager screen (**Plugins → PluginManagerView →
Manage plugins**) built entirely on Hero* components:

- **Active** — every registered plugin (code + activated specs) as a card with
  name, description, tags, version and slot badge; spec plugins can be
  **Deactivated** at runtime.
- **Search** — filters active + available plugins by name, id, author,
  description and tags.
- **Available** — discoverable-but-inactive plugins (filesystem scan on
  desktop/mobile, bundled specs on web); **Activate** adds them to the
  registry and every `PluginHost` updates immediately.

## Adding a spec plugin at runtime

1. Create a folder in `flutter/plugins/` with a `plugin.yaml` (metadata is
   optional but shown by the manager UI):

```yaml
id: my_plugin
name: My Plugin
version: 0.1.0
author: Your Name
description: What this plugin adds.
tags: [demo, main]
slot: main            # main | sidebar | toolbar | footer
widgets:
  type: card
  props:
    title: Hello
  children:
    - type: text
      props:
        text: Loaded while running
    - type: button
      props:
        label: Go
        action: my_plugin.go
```

2. In the running app hit **Reload plugins** (or enable the `watch` switch for
   a 5s auto-rescan). The new UI appears — no restart.

### Widget vocabulary

| type | props |
| --- | --- |
| `text` | `text`, `style` (`body`/`bodyLarge`/`title`/`headline`), `color` (`primary`/`green`/`red`/`amber`/`purple`) |
| `button` | `label`, `action` (fired via the host's `onAction` callback) |
| `card` | `title`, `children` |
| `column` / `row` | `children`, `spacing` |
| `divider` | — |
| `badge` | `label`, `color` |

Unknown types render a red `[unknown widget …]` placeholder — one bad spec
never takes down the host.

## Adding a code plugin

```dart
class MyPlugin extends FlutterPlugin {
  @override String get id => 'my_plugin';
  @override String get name => 'My Plugin';
  @override String? get author => 'Your Name';          // shown in the manager
  @override String? get description => 'What it adds.'; // shown in the manager
  @override List<String> get tags => const ['demo'];    // searchable
  @override Widget build(BuildContext context, PluginSlot slot) =>
      Text('Hello from code');
}
```

Register it: `controller.register(MyPlugin());` — see
`flutter/ui/lib/src/widgetbook/use_cases/plugins_use_cases.dart` for the live
example (registers `ClockPlugin` + loads the spec plugins into `PluginHost`
for the main and sidebar slots).

## Platform note

The filesystem scan uses `dart:io`, so it runs on desktop and mobile. On web
there is no filesystem: the catalogue falls back to bundled specs
(`PluginController.loadBundled` / `parseBundled`), which still reload on
demand but are compiled into the app. The controller API is identical on all
platforms.

## Usage from code

```dart
final controller = PluginController(pluginsDir: '../plugins');
controller.register(MyCodePlugin());
controller.reload();          // scan dir -> specs appear
controller.startWatching();   // optional: auto-rescan every 5s

// In the widget tree:
PluginHost(registry: controller.registry, slot: PluginSlot.main);
```

`PluginController` and `PluginRegistry` are `ChangeNotifier`s — hosts
re-render automatically on any registration/reload.

## Run the demo

```bash
cd flutter/ui
flutter pub get
flutter run -d chrome   # web: bundled specs fallback
# or, for live filesystem scanning:
flutter run -d linux    # desktop: drop plugin.yaml folders, hit Reload
```

Catalogue → **Plugins → PluginHost → Runtime host**.

# congelado UI shell

The one Flutter project spanning web, desktop, and mobile. It contains **no
plugin-specific code** — `lib/shell/shell_scaffold.dart` is a generic
sidebar/tabs chrome, and `lib/registry.dart` is the one hand-maintained file
that merges every installed plugin's UI contributions (see
`plugins/*/ui/README.md` for how a plugin ships one).

## One-time setup

This checkout doesn't yet have the platform runner folders (`android/`,
`ios/`, `linux/`, `macos/`, `windows/`, `web/`) that `flutter create`
normally scaffolds alongside `lib/` and `pubspec.yaml` — generate them once
with the Flutter SDK installed:

```bash
cd app
flutter create --platforms=web,linux,windows,macos,android,ios --project-name congelado_app .
flutter pub get
```

`flutter create` won't overwrite `lib/`, `pubspec.yaml`, or this README —
it only fills in the missing platform folders.

## Running

```bash
flutter run -d linux   # or -d chrome, -d macos, -d windows
```

## Building

```bash
flutter build web
flutter build linux    # or windows, macos
flutter build apk      # or ios
```

## Adding a plugin's UI

1. Give the plugin a `ui/` package (see `plugins/engine/ui/` as the
   reference example) implementing `congelado_ui_sdk`'s
   `PluginUiContribution`.
2. Add a `path:` dependency on it in `pubspec.yaml`.
3. Import its `register.dart` in `lib/registry.dart` and append its
   `contributions` list.
4. `flutter pub get` and rebuild — no C++ rebuild or engine restart needed.

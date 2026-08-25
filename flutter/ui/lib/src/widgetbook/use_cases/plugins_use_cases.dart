import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:congelado_plugin_clock/congelado_plugin_clock.dart';
import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

import 'plugins_bundled_specs.dart';
import 'plugin_manager_view.dart';

/// Widgetbook folder showcasing the runtime plugin system
/// (`flutter/plugins/`): a code plugin (live clock) plus spec plugins loaded
/// from `plugin.yaml` files on disk.
///
/// Runtime extension: on IO platforms (desktop/mobile) the controller scans
/// `flutter/plugins/` — drop a new `plugin.yaml` folder there while the app
/// runs and hit **Reload plugins** (or enable watching) and the new UI
/// appears without rebuilding. On web there is no filesystem, so the
/// catalogue falls back to the bundled specs below.
List<WidgetbookComponent> pluginsUseCases() => [
      WidgetbookComponent(
        name: 'PluginHost',
        useCases: [
          WidgetbookUseCase(
            name: 'Runtime host',
            builder: (context) => const _PluginShowcase(),
          ),
        ],
      ),
      WidgetbookComponent(
        name: 'PluginManagerView',
        useCases: [
          WidgetbookUseCase(
            name: 'Manage plugins',
            builder: (context) => const _PluginManagerHost(),
          ),
        ],
      ),
    ];

/// Hosts [PluginManagerView] with a fresh controller (code clock plugin +
/// spec plugins from disk or bundle), same setup as the showcase.
class _PluginManagerHost extends StatefulWidget {
  const _PluginManagerHost();

  @override
  State<_PluginManagerHost> createState() => _PluginManagerHostState();
}

class _PluginManagerHostState extends State<_PluginManagerHost> {
  late final PluginController _controller;

  @override
  void initState() {
    super.initState();
    _controller = PluginController(
      pluginsDir: '../plugins',
      bundledYaml: bundledPluginSpecs,
    );
    _controller.register(const ClockPlugin());
    if (pluginDirScanner.supportsFileSystem) {
      _controller.reload();
    } else {
      _controller.loadBundled(bundledPluginSpecs);
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return PluginManagerView(controller: _controller);
  }
}

class _PluginShowcase extends StatefulWidget {
  const _PluginShowcase();

  @override
  State<_PluginShowcase> createState() => _PluginShowcaseState();
}

class _PluginShowcaseState extends State<_PluginShowcase> {
  late final PluginController _controller;

  @override
  void initState() {
    super.initState();
    _controller = PluginController(
      // Relative to the CWD when run from flutter/ui/ (flutter run -d …).
      pluginsDir: '../plugins',
      bundledYaml: bundledPluginSpecs,
    );
    // Code plugin — always registered.
    _controller.register(const ClockPlugin());
    if (pluginDirScanner.supportsFileSystem) {
      _controller.reload();
    } else {
      _controller.loadBundled(bundledPluginSpecs);
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: _controller,
      builder: (context, _) {
        final specs = _controller.specs.length;
        return Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _chrome(context),
            if (_lastAction != null)
              Padding(
                padding: const EdgeInsets.only(top: 4),
                child: Text(
                  'last action: $_lastAction',
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              ),
            const SizedBox(height: 12),
            if (pluginDirScanner.supportsFileSystem)
              _dirHost(context)
            else
              _bundledHost(context, specs),
          ],
        );
      },
    );
  }

  Widget _chrome(BuildContext context) {
    return Material(
      color: HeroTokens.colorBackground.resolve(context),
      child: DefaultTextStyle(
        style: Theme.of(context).textTheme.bodyMedium!,
        child: Row(
          children: [
            const Text('Plugin system — flutter/plugins'),
            const SizedBox(width: 8),
            const _SpecBadge('code', _BadgeKind.code),
            if (!pluginDirScanner.supportsFileSystem) ...[
              const SizedBox(width: 8),
              const _SpecBadge('bundled', _BadgeKind.bundled),
            ],
            const SizedBox(width: 16),
            FilledButton.tonal(
              onPressed: () {
                if (pluginDirScanner.supportsFileSystem) {
                  _controller.reload();
                } else {
                  _controller.loadBundled(bundledPluginSpecs);
                }
              },
              child: const Text('Reload plugins'),
            ),
            const SizedBox(width: 8),
            Switch(
              value: _watching,
              onChanged: (v) => setState(() {
                _watching = v;
                v ? _controller.startWatching() : _controller.stopWatching();
              }),
            ),
            const Text('watch'),
          ],
        ),
      ),
    );
  }

  bool _watching = false;

  /// Last plugin button action, shown inline — the Widgetbook frame has no
  /// descendant Scaffold, so `ScaffoldMessenger.showSnackBar` would throw.
  String? _lastAction;

  Widget _dirHost(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Scanning ../plugins — add a plugin.yaml folder and hit Reload, '
          'or enable watch (5s rescan).',
          style: Theme.of(context).textTheme.bodySmall,
        ),
        const SizedBox(height: 8),
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
              child: PluginHost(
                registry: _controller.registry,
                slot: PluginSlot.main,
                onAction: (a) => _onAction(a),
                header: const Text('main slot'),
              ),
            ),
            const SizedBox(width: 24),
            Expanded(
              child: PluginHost(
                registry: _controller.registry,
                slot: PluginSlot.sidebar,
                header: const Text('sidebar slot'),
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _bundledHost(BuildContext context, int specs) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Web fallback: $specs bundled spec plugin(s) (no filesystem). '
          'Desktop/mobile scans ../plugins live.',
          style: Theme.of(context).textTheme.bodySmall,
        ),
        const SizedBox(height: 8),
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
              child: PluginHost(
                registry: _controller.registry,
                slot: PluginSlot.main,
                onAction: (a) => _onAction(a),
                header: const Text('main slot'),
              ),
            ),
            const SizedBox(width: 24),
            Expanded(
              child: PluginHost(
                registry: _controller.registry,
                slot: PluginSlot.sidebar,
                header: const Text('sidebar slot'),
              ),
            ),
          ],
        ),
      ],
    );
  }

  void _onAction(String action) {
    // Inline status instead of a SnackBar: the Widgetbook use-case frame has
    // no descendant Scaffold, so ScaffoldMessenger.showSnackBar throws
    // "no descendant Scaffolds to present to".
    setState(() => _lastAction = action);
  }
}

/// Badge colors: HeroUI token accent for "code", warning token for
/// "bundled" (material color constants and raw hex color literals are
/// forbidden in lib/ by the design-system static audit). Resolved against
/// the ambient HeroScope at build time.
class _SpecBadge extends StatelessWidget {
  const _SpecBadge(this.label, this.kind);

  final String label;
  final _BadgeKind kind;

  @override
  Widget build(BuildContext context) {
    final color = kind == _BadgeKind.code
        ? HeroTokens.colorAccent.resolve(context)
        : HeroTokens.colorWarning.resolve(context);
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
      decoration: BoxDecoration(
        color: color.withValues(alpha: 0.15),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Text(
        label,
        style: TextStyle(color: color, fontSize: 12, fontWeight: FontWeight.w600),
      ),
    );
  }
}

enum _BadgeKind { code, bundled }

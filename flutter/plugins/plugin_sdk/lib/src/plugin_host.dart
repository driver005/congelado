import 'package:flutter/material.dart';

import 'plugin_registry.dart';
import 'plugin_slot.dart';
import 'plugin_spec.dart';
import 'plugin_widget_factory.dart';

/// Renders the active plugins for one [slot].
///
/// Listens to [registry] and rebuilds whenever plugins are registered,
/// unregistered or reloaded — so a directory rescan while the app runs shows
/// up immediately.
class PluginHost extends StatelessWidget {
  const PluginHost({
    super.key,
    required this.registry,
    this.slot = PluginSlot.main,
    this.onAction,
    this.header,
  });

  final PluginRegistry registry;
  final PluginSlot slot;

  /// Forwards `button` actions from spec plugins.
  final ValueChanged<String>? onAction;

  /// Optional header shown above the contributions (e.g. slot title).
  final Widget? header;

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: registry,
      builder: (context, _) {
        final entries = registry.entriesFor(slot);
        if (entries.isEmpty) {
          return const SizedBox.shrink();
        }
        final factory = PluginWidgetFactory(onAction: onAction);
        return Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (header != null) header!,
            ...entries.map((e) => _entry(context, e, factory)),
          ],
        );
      },
    );
  }

  Widget _entry(BuildContext context, PluginEntry entry, PluginWidgetFactory factory) {
    if (entry.isCode) {
      return entry.plugin!.build(context, slot);
    }
    final spec = entry.spec!;
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: _SpecFrame(
        spec: spec,
        child: factory.build(spec.root),
      ),
    );
  }
}

/// Visual frame around a spec contribution: the plugin name as a caption plus
/// the rendered widget tree.
class _SpecFrame extends StatelessWidget {
  const _SpecFrame({required this.spec, required this.child});

  final PluginSpec spec;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(
          spec.name,
          style: Theme.of(context).textTheme.labelSmall?.copyWith(
                color: Theme.of(context).colorScheme.outline,
              ),
        ),
        child,
      ],
    );
  }
}

import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter/material.dart';

/// Plugin manager screen (built on Hero* components): lists active plugins,
/// searches the discoverable ones and activates/deactivates them at runtime.
///
/// Data comes from a [PluginController]:
///
/// - **Active** — the registry entries (code plugins + activated specs),
///   filtered by the search query.
/// - **Available** — `controller.available()` (filesystem scan on IO
///   platforms, bundled specs on web), minus the active ones, filtered by the
///   query. "Activate" adds the spec to the registry; the change propagates to
///   every [PluginHost] immediately (both are ChangeNotifiers).
class PluginManagerView extends StatefulWidget {
  const PluginManagerView({super.key, required this.controller});

  final PluginController controller;

  @override
  State<PluginManagerView> createState() => _PluginManagerViewState();
}

class _PluginManagerViewState extends State<PluginManagerView> {
  String _query = '';

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: widget.controller,
      builder: (context, _) {
        final active = widget.controller.registry.entries
            .where((e) => _matches(e.id, e.name, e.spec?.author, e.spec?.description,
                e.spec?.tags ?? const []))
            .toList();
        final available = widget.controller
            .available()
            .where((s) => !widget.controller.isActive(s.id))
            .where((s) => _matches(
                s.id, s.name, s.author, s.description, s.tags))
            .toList();
        return Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            HeroSearchField(
              placeholder: 'Search plugins by name, id, author, tag…',
              fullWidth: true,
              onChanged: (v) => setState(() => _query = v),
            ),
            const SizedBox(height: 16),
            Text(
              'Active (${active.length})',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 8),
            if (active.isEmpty)
              const _EmptyNote('No active plugins match the query.')
            else
              ...active.map((e) => _entryCard(context, e)),
            const SizedBox(height: 24),
            Text(
              'Available (${available.length})',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 8),
            if (available.isEmpty)
              const _EmptyNote(
                  'Nothing new here. Drop a plugin.yaml folder into '
                  'flutter/plugins/ and press Reload — or install from the '
                  'bundle on web.')
            else
              ...available.map((s) => _availableCard(context, s)),
          ],
        );
      },
    );
  }

  bool _matches(String id, String name, String? author, String? description,
      List<String> tags) {
    final q = _query.trim().toLowerCase();
    if (q.isEmpty) return true;
    bool has(String? s) => s?.toLowerCase().contains(q) ?? false;
    return has(id) || has(name) || has(author) || has(description) ||
        tags.any((t) => t.toLowerCase().contains(q));
  }

  /// Card for one active entry (code plugin or activated spec).
  Widget _entryCard(BuildContext context, PluginEntry entry) {
    final spec = entry.spec;
    final tags = entry.isCode ? entry.plugin!.tags : spec!.tags;
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: HeroCard(
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _titleRow(context, entry),
                    if (spec?.description != null || entry.plugin?.description != null)
                      Padding(
                        padding: const EdgeInsets.only(top: 4),
                        child: Text(
                          spec?.description ?? entry.plugin!.description!,
                          style: Theme.of(context).textTheme.bodySmall,
                        ),
                      ),
                    if (tags.isNotEmpty) ...[
                      const SizedBox(height: 8),
                      Wrap(
                        spacing: 4,
                        runSpacing: 4,
                        children: [
                          for (final t in tags)
                            HeroChip(
                              label: t,
                              size: HeroChipSize.sm,
                              variant: HeroChipVariant.tertiary,
                            ),
                        ],
                      ),
                    ],
                  ],
                ),
              ),
              const SizedBox(width: 12),
              Column(
                crossAxisAlignment: CrossAxisAlignment.end,
                children: [
                  HeroButton(
                    label: entry.isCode ? 'Code plugin' : 'Deactivate',
                    variant: HeroButtonVariant.ghost,
                    size: HeroButtonSize.sm,
                    onPressed: entry.isCode
                        ? null
                        : () => widget.controller.deactivate(entry.id),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    entry.isCode ? 'v${entry.plugin!.version}' : 'v${spec!.version}',
                    style: Theme.of(context).textTheme.labelSmall,
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  /// Card for one discoverable-but-inactive spec plugin.
  Widget _availableCard(BuildContext context, PluginSpec spec) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: HeroCard(
        variant: HeroCardVariant.secondary,
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Expanded(
                          child: Text(
                            spec.name,
                            style: Theme.of(context).textTheme.titleSmall,
                          ),
                        ),
                        _slotBadge(spec.slot),
                      ],
                    ),
                    if (spec.author != null)
                      Padding(
                        padding: const EdgeInsets.only(top: 2),
                        child: Text(
                          'by ${spec.author} · v${spec.version}',
                          style: Theme.of(context).textTheme.labelSmall,
                        ),
                      ),
                    if (spec.description != null)
                      Padding(
                        padding: const EdgeInsets.only(top: 4),
                        child: Text(
                          spec.description!,
                          style: Theme.of(context).textTheme.bodySmall,
                        ),
                      ),
                    if (spec.tags.isNotEmpty) ...[
                      const SizedBox(height: 8),
                      Wrap(
                        spacing: 4,
                        runSpacing: 4,
                        children: [
                          for (final t in spec.tags)
                            HeroChip(
                              label: t,
                              size: HeroChipSize.sm,
                              variant: HeroChipVariant.tertiary,
                            ),
                        ],
                      ),
                    ],
                  ],
                ),
              ),
              const SizedBox(width: 12),
              HeroButton(
                label: 'Activate',
                variant: HeroButtonVariant.secondary,
                size: HeroButtonSize.sm,
                onPressed: () => widget.controller.activate(spec),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _titleRow(BuildContext context, PluginEntry entry) {
    final spec = entry.spec;
    return Row(
      children: [
        Expanded(
          child: Text(
            entry.name,
            style: Theme.of(context).textTheme.titleSmall,
          ),
        ),
        if (spec != null) _slotBadge(spec.slot),
      ],
    );
  }

  Widget _slotBadge(PluginSlot slot) {
    return HeroBadge(
      label: slot.name,
      size: HeroBadgeSize.sm,
      variant: HeroBadgeVariant.soft,
      color: switch (slot) {
        PluginSlot.main => HeroBadgeColor.accent,
        PluginSlot.sidebar => HeroBadgeColor.success,
        PluginSlot.toolbar => HeroBadgeColor.warning,
        PluginSlot.footer => HeroBadgeColor.default_,
      },
    );
  }
}

class _EmptyNote extends StatelessWidget {
  const _EmptyNote(this.text);

  final String text;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8),
      child: Text(
        text,
        style: Theme.of(context).textTheme.bodySmall?.copyWith(
              color: HeroTokens.colorMuted.resolve(context),
            ),
      ),
    );
  }
}

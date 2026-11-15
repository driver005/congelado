import 'package:flutter/widgets.dart';

/// Nav placement + page content a plugin's `ui/` package contributes to the
/// shared congelado shell's sidebar/tabs. One instance per top-level page or
/// per tab; nest via [parentId] to group several contributions as tabs under
/// a shared sidebar entry.
///
/// Drill-down navigation within a plugin's own pages (e.g. a list page
/// pushing a detail page) is ordinary `Navigator.push` inside that plugin's
/// widget code — it does not need its own [PluginUiContribution]. This type
/// only describes the shell's persistent nav structure, not every screen a
/// plugin's UI can reach.
class PluginUiContribution {
  const PluginUiContribution({
    required this.id,
    required this.label,
    required this.builder,
    this.parentId,
    this.icon,
    this.order = 0,
  });

  /// Stable, globally unique id — convention: `"<plugin>.<page>"`, e.g.
  /// `"engine.tasks"`.
  final String id;

  /// Id of the contribution this nests under as a tab, or null for a
  /// root-level sidebar entry.
  final String? parentId;

  final String label;
  final IconData? icon;

  /// Sort order among siblings, ascending.
  final int order;

  final WidgetBuilder builder;
}

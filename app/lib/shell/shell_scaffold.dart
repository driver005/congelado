import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source (pulled from pub.dev): FTabs takes a `control:`
// FTabControl object instead of a bare `initialIndex:`/`onChange:` pair — FTabControl.lifted
// matches this widget's existing lifted-state pattern (parent owns `_selected`). FThemeData
// .colors' field names (background/primary/border/mutedForeground) were correct as originally
// written.

/// A root-level sidebar entry plus its child contributions (rendered as tabs
/// when there are any, otherwise the root's own page is shown directly).
class _NavGroup {
  _NavGroup(this.root) : children = [];

  final PluginUiContribution root;
  final List<PluginUiContribution> children;
}

List<_NavGroup> _buildGroups(List<PluginUiContribution> contributions) {
  final byId = {for (final c in contributions) c.id: c};
  final groups = <String, _NavGroup>{};

  for (final c in contributions) {
    if (c.parentId == null) {
      groups[c.id] = _NavGroup(c);
    }
  }
  for (final c in contributions) {
    final parentId = c.parentId;
    if (parentId == null) continue;
    final group = groups[parentId];
    if (group == null) {
      // Unknown parentId — surfaces as a loud assertion in debug rather than
      // a silently dropped page, since this is a plugin-authoring mistake.
      assert(
        false,
        'PluginUiContribution "${c.id}" has parentId "$parentId", which is '
        'not a root-level contribution (known ids: ${byId.keys.join(", ")})',
      );
      continue;
    }
    group.children.add(c);
  }
  for (final group in groups.values) {
    group.children.sort((a, b) => a.order.compareTo(b.order));
  }
  final sorted = groups.values.toList()
    ..sort((a, b) => a.root.order.compareTo(b.root.order));
  return sorted;
}

/// Persistent sidebar/tabs chrome — the entire extent of "shared UI" code in
/// this shell. Every page inside a group comes from a plugin's own `ui/`
/// package via [PluginUiContribution.builder]; this widget only arranges
/// them.
///
/// Structure/behavior is unchanged from before the Forui design system landed
/// — same [_NavGroup]/[_buildGroups] logic, same nav-rail-plus-tabs shape.
/// Only the leaf chrome widgets are Forui-themed: the rail's colors come from
/// [FTheme] (shadcn's `background`/`foreground`/`border` roles) and the tab
/// strip is [FTabs] instead of Material's `TabBar`.
class ShellScaffold extends StatefulWidget {
  const ShellScaffold({super.key, required this.contributions});

  final List<PluginUiContribution> contributions;

  @override
  State<ShellScaffold> createState() => _ShellScaffoldState();
}

class _ShellScaffoldState extends State<ShellScaffold> {
  late final List<_NavGroup> _groups = _buildGroups(widget.contributions);
  int _selectedIndex = 0;

  @override
  Widget build(BuildContext context) {
    if (_groups.isEmpty) {
      return const Scaffold(
        body: Center(child: Text('No plugin UI contributions registered.')),
      );
    }

    final selected = _groups[_selectedIndex];
    final colors = FTheme.of(context).colors;

    return Scaffold(
      backgroundColor: colors.background,
      body: Row(
        children: [
          // 220px fixed width, matching Medusa's real sidebar (packages/admin/dashboard/src/
          // components/layout/shell/shell.tsx's DesktopSidebarContainer: `w-[220px]`). Plain
          // icon+label row nav items, not NavigationRail — Material's NavigationRail only lays
          // out icon-above-label (or icon-only), never icon-beside-label like Medusa's actual
          // sidebar items, so no NavigationRail configuration gets that shape; a simple
          // ListView of tappable rows matches the real thing directly instead of fighting a
          // widget built for a narrower icon-rail shape. VerticalDivider still provides
          // Medusa's `border-e` end-side border.
          SizedBox(
            width: 220,
            child: ColoredBox(
              color: colors.background,
              child: ListView(
                padding: const EdgeInsets.all(8),
                children: [
                  for (var index = 0; index < _groups.length; index++)
                    _NavRow(
                      icon: _groups[index].root.icon ?? Icons.circle_outlined,
                      label: _groups[index].root.label,
                      selected: index == _selectedIndex,
                      onTap: () => setState(() => _selectedIndex = index),
                    ),
                ],
              ),
            ),
          ),
          VerticalDivider(width: 1, color: colors.border),
          Expanded(
            child: selected.children.isEmpty
                ? selected.root.builder(context)
                : _TabbedGroup(group: selected),
          ),
        ],
      ),
    );
  }
}

/// A single sidebar nav entry — icon beside label, selected state shown via
/// background fill + `colors.primary` text/icon, matching Medusa's own
/// `nav-item.tsx` row shape (not NavigationRail's icon-above-label layout).
class _NavRow extends StatelessWidget {
  const _NavRow({
    required this.icon,
    required this.label,
    required this.selected,
    required this.onTap,
  });

  final IconData icon;
  final String label;
  final bool selected;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Material(
        color: selected ? colors.secondary : Colors.transparent,
        borderRadius: BorderRadius.circular(6),
        child: InkWell(
          borderRadius: BorderRadius.circular(6),
          onTap: onTap,
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 8),
            child: Row(
              children: [
                Icon(icon, size: 18, color: selected ? colors.primary : colors.mutedForeground),
                const SizedBox(width: 10),
                Expanded(
                  child: Text(
                    label,
                    style: TextStyle(
                      fontSize: 13,
                      fontWeight: FontWeight.w500,
                      color: selected ? colors.primary : colors.foreground,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class _TabbedGroup extends StatefulWidget {
  const _TabbedGroup({required this.group});

  final _NavGroup group;

  @override
  State<_TabbedGroup> createState() => _TabbedGroupState();
}

class _TabbedGroupState extends State<_TabbedGroup> {
  int _selected = 0;

  @override
  Widget build(BuildContext context) {
    final group = widget.group;
    // FTabs owns both the tab strip and the selected tab's content area (mirrors Material's
    // TabBar+TabBarView combo in one widget) — each FTabEntry's child is that tab's actual page,
    // not a placeholder, so there's no separate content area to render below it.
    return FTabs(
      control: FTabControl.lifted(
        index: _selected,
        onChange: (index) => setState(() => _selected = index),
      ),
      children: [
        for (final child in group.children)
          FTabEntry(label: Text(child.label), child: Builder(builder: child.builder)),
      ],
    );
  }
}

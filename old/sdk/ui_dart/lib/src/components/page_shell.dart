import 'package:flutter/material.dart' show Scaffold;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Structure pulled from Medusa admin dashboard's real shell/page-template source
// (medusajs/medusa, packages/admin/dashboard/src/components/layout/shell/shell.tsx +
// .../pages/single-column-page/single-column-layout-component.tsx) — the topbar's
// breadcrumb+actions row, bottom border, 12px padding, and the content "Gutter"'s
// centered max-width-1600px/12px-padding/8px-gap shape all mirror that file's literal
// Tailwind classes (`p-3` = 12px, `gap-y-2` = 8px, `max-w-[1600px]`, `border-b`).

/// Page-level chrome every plugin page renders inside instead of hand-rolling its own
/// `Scaffold(appBar: AppBar(...), body: ...)` — Medusa's `Shell` + `SingleColumnPage`
/// combined (this app has no two-column/list-plus-preview pages, so only the single-
/// column template is ported). Still backed by a real [Scaffold] internally (not
/// removed) — plenty of existing pages call `ScaffoldMessenger.of(context)` for error
/// snackbars, which needs a `Scaffold` ancestor to find.
class CPageShell extends StatelessWidget {
  const CPageShell({super.key, required this.topbar, required this.child});

  /// Typically a [CPageTopbar].
  final Widget topbar;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Scaffold(
      backgroundColor: colors.background,
      body: SafeArea(
        child: Center(
          child: ConstrainedBox(
            constraints: const BoxConstraints(maxWidth: 1600),
            child: Padding(
              padding: const EdgeInsets.all(12),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  topbar,
                  const SizedBox(height: 8),
                  Expanded(child: child),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// The topbar's breadcrumb trail + right-aligned actions row — Medusa's `Shell`'s own
/// `Topbar`/`Breadcrumbs`. [breadcrumbs] has no navigation/routing behind it (this app
/// drills down via plain `Navigator.push`, not a route hierarchy a real breadcrumb trail
/// could derive itself from — see `app/lib/main.dart`'s own note on why) — callers pass
/// the trail as plain labels (e.g. `['Tasks', taskName]`), rendered as static text with
/// a chevron separator, not tappable links.
class CPageTopbar extends StatelessWidget {
  const CPageTopbar({super.key, required this.breadcrumbs, this.actions = const []});

  final List<String> breadcrumbs;
  final List<Widget> actions;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(border: Border(bottom: BorderSide(color: colors.border))),
      child: Row(
        children: [
          Expanded(
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                for (var index = 0; index < breadcrumbs.length; index++) ...[
                  if (index > 0)
                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 8),
                      child: Icon(FLucideIcons.chevronRight, size: 12, color: colors.mutedForeground),
                    ),
                  Text(
                    breadcrumbs[index],
                    style: TextStyle(
                      fontSize: 13,
                      fontWeight: FontWeight.w500,
                      color: index == breadcrumbs.length - 1 ? colors.foreground : colors.mutedForeground,
                    ),
                  ),
                ],
              ],
            ),
          ),
          if (actions.isNotEmpty)
            Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                for (var index = 0; index < actions.length; index++) ...[
                  if (index > 0) const SizedBox(width: 12),
                  actions[index],
                ],
              ],
            ),
        ],
      ),
    );
  }
}

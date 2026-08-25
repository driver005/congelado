import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';

import 'src/widgetbook/hero_widgetbook_app.dart';

void main() {
  runApp(const _WidgetbookHost());
}

/// HeroScope must sit ABOVE the chrome Navigator: overlay entries (tooltips,
/// dialogs) mount into the Navigator's Overlay, outside the use-case subtree
/// where the Theme addon installs its HeroScope. Without this outer scope,
/// `HeroTooltip` overlay content throws "No MixScope found".
class _WidgetbookHost extends StatelessWidget {
  const _WidgetbookHost();

  @override
  Widget build(BuildContext context) {
    return HeroScope(
      theme: MediaQuery.platformBrightnessOf(context) == Brightness.dark
          ? HeroTheme.dark
          : HeroTheme.light,
      child: const HeroWidgetbookApp(),
    );
  }
}

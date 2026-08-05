import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'registry.dart';
import 'shell/shell_scaffold.dart';

void main() {
  runApp(const CongeladoApp());
}

class CongeladoApp extends StatelessWidget {
  const CongeladoApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Congelado',
      theme: ThemeData(useMaterial3: true, colorSchemeSeed: Colors.indigo),
      darkTheme: ThemeData(
        useMaterial3: true,
        colorSchemeSeed: Colors.indigo,
        brightness: Brightness.dark,
      ),
      // FTheme wraps every route's content via MaterialApp's own `builder` hook (Forui's
      // documented integration point) rather than wrapping MaterialApp itself, so it stays in
      // scope for Navigator.push'd pages too (drill-down navigation inside a plugin's own
      // widget tree, e.g. tasks_list_page -> task_detail_page). Brightness follows the
      // platform/system setting, same signal MaterialApp's own theme/darkTheme pair already
      // reacts to, so Forui and Material widgets never disagree about light vs dark.
      builder: (context, child) => FTheme(
        data: MediaQuery.platformBrightnessOf(context) == Brightness.dark
            ? CongeladoTheme.dark
            : CongeladoTheme.light,
        child: child!,
      ),
      home: ShellScaffold(contributions: buildPluginContributions()),
    );
  }
}

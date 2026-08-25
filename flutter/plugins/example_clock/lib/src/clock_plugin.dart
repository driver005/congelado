import 'dart:async';

import 'package:congelado_plugin_sdk/congelado_plugin_sdk.dart';
import 'package:flutter/material.dart';

/// A [FlutterPlugin] that renders a live ticking clock.
class ClockPlugin extends FlutterPlugin {
  const ClockPlugin();

  @override
  String get id => 'example_clock';

  @override
  String get name => 'Live Clock';

  @override
  String? get author => 'Congelado Team';

  @override
  String? get description => 'A live ticking clock — demonstrates a code '
      'plugin (arbitrary widgets) alongside the spec plugins.';

  @override
  List<String> get tags => const ['clock', 'demo', 'code'];

  @override
  Widget build(BuildContext context, PluginSlot slot) {
    return const _ClockCard();
  }
}

class _ClockCard extends StatefulWidget {
  const _ClockCard();

  @override
  State<_ClockCard> createState() => _ClockCardState();
}

class _ClockCardState extends State<_ClockCard> {
  Timer? _timer;
  DateTime _now = DateTime.now();

  @override
  void initState() {
    super.initState();
    _timer = Timer.periodic(const Duration(seconds: 1), (_) {
      setState(() => _now = DateTime.now());
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final time = '${_now.hour.toString().padLeft(2, '0')}:'
        '${_now.minute.toString().padLeft(2, '0')}:'
        '${_now.second.toString().padLeft(2, '0')}';
    return Card(
      margin: const EdgeInsets.all(4),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(Icons.schedule, size: 20, color: Theme.of(context).colorScheme.primary),
            const SizedBox(width: 8),
            Text('Code plugin: $time',
                style: Theme.of(context).textTheme.titleMedium),
          ],
        ),
      ),
    );
  }
}

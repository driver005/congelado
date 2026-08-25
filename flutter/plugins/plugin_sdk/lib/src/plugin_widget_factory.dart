import 'package:flutter/material.dart';

import 'plugin_spec.dart';

/// Renders a [PluginSpec] widget tree into real widgets.
///
/// Supported node types (the declarative vocabulary of spec plugins):
///
/// | type | props |
/// | --- | --- |
/// | `text` | `text`, `style` (body/bodyLarge/title/headline), `color` |
/// | `button` | `label`, `action` (id passed to [onAction]) |
/// | `card` | `title`, `children` |
/// | `column` / `row` | `children`, `spacing` |
/// | `divider` | — |
/// | `badge` | `label`, `color` |
///
/// Unknown types render a small error placeholder instead of throwing, so one
/// bad spec cannot take down the host.
class PluginWidgetFactory {
  const PluginWidgetFactory({this.onAction});

  /// Invoked when a `button` node with an `action` prop is pressed.
  final ValueChanged<String>? onAction;

  Widget build(PluginNode node) => _buildNode(node);

  Widget _buildNode(PluginNode node) {
    switch (node.type) {
      case 'text':
        return _text(node);
      case 'button':
        return _button(node);
      case 'card':
        return _card(node);
      case 'column':
        return _column(node);
      case 'row':
        return _row(node);
      case 'divider':
        return const Divider(height: 16);
      case 'badge':
        return _badge(node);
      default:
        return Padding(
          padding: const EdgeInsets.all(4),
          child: Text(
            '[unknown widget "${node.type}"]',
            style: TextStyle(color: Colors.red.shade400, fontSize: 12),
          ),
        );
    }
  }

  Widget _text(PluginNode node) {
    final style = switch (node.propString('style')) {
      'bodyLarge' => ThemeData.light().textTheme.bodyLarge,
      'title' => ThemeData.light().textTheme.titleMedium,
      'headline' => ThemeData.light().textTheme.headlineSmall,
      _ => ThemeData.light().textTheme.bodyMedium,
    };
    final color = _color(node.propString('color'));
    return Text(
      node.propString('text') ?? '',
      style: style?.copyWith(color: color),
    );
  }

  Widget _button(PluginNode node) {
    final label = node.propString('label') ?? 'Button';
    final action = node.propString('action');
    return FilledButton(
      onPressed: action == null ? null : () => onAction?.call(action),
      child: Text(label),
    );
  }

  Widget _card(PluginNode node) {
    return Card(
      margin: const EdgeInsets.all(4),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisSize: MainAxisSize.min,
          children: [
            if (node.propString('title') != null)
              Text(
                node.propString('title')!,
                style: ThemeData.light().textTheme.titleMedium,
              ),
            ...node.childNodes.map(_buildNode),
          ],
        ),
      ),
    );
  }

  Widget _column(PluginNode node) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        for (final (i, child) in node.childNodes.indexed) ...[
          if (i > 0) SizedBox(height: node.propDouble('spacing') ?? 8),
          _buildNode(child),
        ],
      ],
    );
  }

  Widget _row(PluginNode node) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        for (final (i, child) in node.childNodes.indexed) ...[
          if (i > 0) SizedBox(width: node.propDouble('spacing') ?? 8),
          _buildNode(child),
        ],
      ],
    );
  }

  Widget _badge(PluginNode node) {
    final color = _color(node.propString('color'));
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
      decoration: BoxDecoration(
        color: (color ?? Colors.blue).withValues(alpha: 0.15),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Text(
        node.propString('label') ?? '',
        style: TextStyle(
          color: color ?? Colors.blue.shade700,
          fontSize: 12,
          fontWeight: FontWeight.w600,
        ),
      ),
    );
  }

  Color? _color(String? name) => switch (name) {
        'primary' => Colors.blue.shade700,
        'green' => Colors.green.shade700,
        'red' => Colors.red.shade700,
        'amber' => Colors.amber.shade800,
        'purple' => Colors.purple.shade700,
        _ => null,
      };
}

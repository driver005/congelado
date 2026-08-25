import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Feedback component use cases: Alert, AlertDialog, Toast, ProgressCircle,
/// Meter, EmptyState — mirroring the HeroUI storybook stories.
List<WidgetbookNode> feedbackUseCases() {
  return [
    WidgetbookComponent(
      name: 'Alert',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroAlert(
            title: 'Notice',
            description: 'This is a default alert message.',
          ),
        ),
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => Column(
            children: [
              for (final (i, color) in HeroAlertColor.values.indexed) ...[
                if (i > 0) const SizedBox(height: 12),
                HeroAlert(
                  color: color,
                  title: '${color.name} alert',
                  description: 'This alert uses the ${color.name} color.',
                ),
              ],
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With icon',
          builder: (context) => const HeroAlert(
            color: HeroAlertColor.danger,
            title: 'Something went wrong',
            description: 'Please try again in a few minutes.',
            icon: Icons.error_outline_rounded,
          ),
        ),
        WidgetbookUseCase(
          name: 'With title',
          builder: (context) => const HeroAlert(
            color: HeroAlertColor.success,
            title: 'All systems operational',
            icon: Icons.check_circle_outline_rounded,
          ),
        ),
        WidgetbookUseCase(
          name: 'With description only',
          builder: (context) => const HeroAlert(
            color: HeroAlertColor.warning,
            description: 'Your session will expire in 5 minutes.',
            icon: Icons.warning_amber_rounded,
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'AlertDialog',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Open alert dialog',
              onPressed: () => _showAlertDialog(context),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final size in HeroAlertDialogSize.values)
                HeroButton(
                  label: 'Open ${size.name}',
                  onPressed: () => _showAlertDialog(context, size: size),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With icon',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Open with icon',
              onPressed: () => _showAlertDialog(
                context,
                icon: Icons.delete_outline_rounded,
                color: HeroAlertDialogColor.danger,
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With close button',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Open with close button',
              onPressed: () => _showAlertDialog(context, closable: true),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Toast',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Show toast',
              onPressed: () => _showToast(context),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final color in HeroToastColor.values)
                HeroButton(
                  label: color.name,
                  variant: HeroButtonVariant.tertiary,
                  onPressed: () => _showToast(context, color: color),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With description',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Show toast with description',
              onPressed: () => _showToast(
                context,
                title: 'Item added',
                description: 'The item has been added to your cart.',
                color: HeroToastColor.success,
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With action',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Show toast with action',
              onPressed: () => _showToast(
                context,
                title: 'Update available',
                color: HeroToastColor.accent,
                action: HeroButton(
                  label: 'Update',
                  size: HeroButtonSize.sm,
                  variant: HeroButtonVariant.primary,
                  onPressed: () {},
                ),
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Placements',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final placement in HeroToastPlacement.values)
                HeroButton(
                  label: placement.name,
                  variant: HeroButtonVariant.tertiary,
                  onPressed: () => _showToast(context, placement: placement),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Loading',
          builder: (context) => Center(
            child: HeroButton(
              label: 'Show loading toast',
              onPressed: () =>
                  _showToast(context, title: 'Uploading…', spinner: true),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ProgressCircle',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroProgressCircle(value: 0.75),
        ),
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 24,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final color in HeroProgressCircleColor.values)
                HeroProgressCircle(value: 0.75, color: color),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 24,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final size in HeroProgressCircleSize.values)
                HeroProgressCircle(value: 0.75, size: size),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Values',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 24,
            crossAxisAlignment: WrapCrossAlignment.center,
            children: [
              for (final value in [0.25, 0.5, 0.75, 1.0])
                HeroProgressCircle(value: value),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Indeterminate',
          builder: (context) => const HeroProgressCircle(),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) =>
              const HeroProgressCircle(value: 0.75, label: 'Downloads'),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Meter',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroMeter(value: 0.75),
        ),
        WidgetbookUseCase(
          name: 'Colors',
          builder: (context) => Column(
            children: [
              for (final (i, color) in HeroMeterColor.values.indexed) ...[
                if (i > 0) const SizedBox(height: 16),
                HeroMeter(value: 0.75, color: color, label: color.name),
              ],
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Column(
            children: [
              for (final (i, size) in HeroMeterSize.values.indexed) ...[
                if (i > 0) const SizedBox(height: 16),
                HeroMeter(value: 0.75, size: size, label: 'size ${size.name}'),
              ],
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Values',
          builder: (context) => Column(
            children: [
              for (final (i, value) in [0.25, 0.5, 0.75, 1.0].indexed) ...[
                if (i > 0) const SizedBox(height: 16),
                HeroMeter(value: value, label: '${(value * 100).round()}%'),
              ],
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With label',
          builder: (context) => const HeroMeter(
            value: 0.75,
            label: 'Storage',
            output: '75% used',
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) =>
              const HeroMeter(value: 0.75, label: 'Storage', disabled: true),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'EmptyState',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) =>
              const HeroEmptyState(message: 'No items to display'),
        ),
        WidgetbookUseCase(
          name: 'With icon',
          builder: (context) => const HeroEmptyState(
            message: 'No notifications',
            icon: Icons.notifications_none_rounded,
          ),
        ),
      ],
    ),
  ];
}

void _showAlertDialog(
  BuildContext context, {
  HeroAlertDialogSize size = HeroAlertDialogSize.md,
  IconData? icon,
  HeroAlertDialogColor color = HeroAlertDialogColor.default_,
  bool closable = false,
}) {
  showHeroAlertDialog<void>(
    context,
    title: 'Delete item',
    description:
        'Are you sure you want to delete this item? This action '
        'cannot be undone.',
    icon: icon,
    color: color,
    size: size,
    // The component closes the dialog itself (dialog-bound context); the
    // callback only controls whether the close trigger is rendered.
    onClose: closable ? () {} : null,
    actions: [
      Builder(
        builder: (dialogContext) => HeroButton(
          label: 'Cancel',
          variant: HeroButtonVariant.ghost,
          onPressed: () => Navigator.pop(dialogContext),
        ),
      ),
      Builder(
        builder: (dialogContext) => HeroButton(
          label: 'Delete',
          variant: color == HeroAlertDialogColor.default_
              ? HeroButtonVariant.primary
              : HeroButtonVariant.danger,
          onPressed: () => Navigator.pop(dialogContext),
        ),
      ),
    ],
  );
}

void _showToast(
  BuildContext context, {
  String? title,
  String? description,
  IconData? icon,
  bool spinner = false,
  HeroToastColor color = HeroToastColor.default_,
  HeroToastPlacement placement = HeroToastPlacement.bottom,
  Widget? action,
}) {
  showHeroToast(
    context,
    title: title ?? 'Toast title',
    description: description,
    icon: icon ?? Icons.info_outline_rounded,
    spinner: spinner,
    color: color,
    placement: placement,
    action: action,
    onClose: () {},
  );
}

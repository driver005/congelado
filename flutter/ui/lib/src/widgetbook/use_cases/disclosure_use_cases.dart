import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Disclosure & navigation use cases: Accordion, Disclosure,
/// DisclosureGroup, Drawer, Dropdown, Popover, Breadcrumbs, Pagination and
/// ButtonGroup — mirroring the HeroUI v3 storybook.
List<WidgetbookNode> disclosureUseCases() {
  return [
    WidgetbookComponent(
      name: 'Accordion',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroAccordion(
              items: [
                HeroAccordionItem(
                  title: 'What is HeroUI?',
                  child: Text(
                    'HeroUI is a modern React component library built on '
                    'react-aria with Tailwind CSS.',
                  ),
                ),
                HeroAccordionItem(
                  title: 'How to install it?',
                  child: Text('Run `npm install @heroui/react`.'),
                ),
                HeroAccordionItem(
                  title: 'Is it accessible?',
                  child: Text(
                    'Yes — every component follows WAI-ARIA patterns.',
                  ),
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Surface variant',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroAccordion(
              variant: HeroAccordionVariant.surface,
              defaultExpandedKeys: {'1'},
              items: [
                HeroAccordionItem(
                  value: '0',
                  title: 'Section one',
                  child: Text('Surface accordion content.'),
                ),
                HeroAccordionItem(
                  value: '1',
                  title: 'Section two',
                  child: Text('Expanded by default.'),
                ),
                HeroAccordionItem(
                  value: '2',
                  title: 'Section three',
                  child: Text('More surface content.'),
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'No separators',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroAccordion(
              hideSeparator: true,
              items: [
                HeroAccordionItem(
                  title: 'First item',
                  child: Text('Content without separators.'),
                ),
                HeroAccordionItem(
                  title: 'Second item',
                  child: Text('Content without separators.'),
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroAccordion(
              items: [
                HeroAccordionItem(
                  title: 'Enabled item',
                  child: Text('I can expand.'),
                ),
                HeroAccordionItem(
                  title: 'Disabled item',
                  disabled: true,
                  child: Text('I cannot.'),
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final multiple = context.knobs.boolean(
              label: 'Allow multiple',
              initialValue: true,
            );
            return StatefulBuilder(
              builder: (context, setState) {
                return SizedBox(
                  width: 420,
                  child: HeroAccordion(
                    allowsMultipleExpanded: multiple,
                    defaultExpandedKeys: const {'0'},
                    items: const [
                      HeroAccordionItem(
                        value: '0',
                        title: 'React',
                        child: Text('Component library.'),
                      ),
                      HeroAccordionItem(
                        value: '1',
                        title: 'Flutter',
                        child: Text('This mirror.'),
                      ),
                      HeroAccordionItem(
                        value: '2',
                        title: 'SwiftUI',
                        child: Text('Coming soon.'),
                      ),
                    ],
                  ),
                );
              },
            );
          },
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Disclosure',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosure(
              title: 'Show details',
              child: Text(
                'A disclosure reveals content on demand with a height '
                'animation and a rotating chevron.',
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Expanded by default',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosure(
              title: 'Installation steps',
              defaultExpanded: true,
              child: Text('1. Install 2. Configure 3. Ship.'),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosure(
              title: 'Locked disclosure',
              isDisabled: true,
              child: Text('Cannot be opened.'),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosure(
              title: 'Terms & conditions',
              child: Text(
                'By expanding this you accept the friendly terms of this '
                'widgetbook example.',
              ),
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'DisclosureGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Multiple open',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosureGroup(
              children: [
                HeroDisclosure(
                  title: 'First',
                  child: Text('First disclosure body.'),
                ),
                HeroDisclosure(
                  title: 'Second',
                  child: Text('Second disclosure body.'),
                ),
                HeroDisclosure(
                  title: 'Third',
                  child: Text('Third disclosure body.'),
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Single open',
          builder: (context) => const SizedBox(
            width: 420,
            child: HeroDisclosureGroup(
              allowsMultipleExpanded: false,
              initialExpandedIndices: {1},
              children: [
                HeroDisclosure(
                  title: 'First',
                  child: Text('First disclosure body.'),
                ),
                HeroDisclosure(
                  title: 'Second',
                  child: Text('Second disclosure body.'),
                ),
                HeroDisclosure(
                  title: 'Third',
                  child: Text('Third disclosure body.'),
                ),
              ],
            ),
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Drawer',
      useCases: [
        WidgetbookUseCase(
          name: 'Placements',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final placement in HeroDrawerPlacement.values)
                HeroButton(
                  label: placement.name,
                  variant: HeroButtonVariant.secondary,
                  onPressed: () => showHeroDrawer<void>(
                    context,
                    placement: placement,
                    title: 'Drawer',
                    description:
                        'Slides in from the ${placement.name} edge.',
                    actions: [
                      Builder(
                        builder: (dialogContext) => HeroButton(
                          label: 'Close',
                          onPressed: () => Navigator.of(dialogContext).pop(),
                        ),
                      ),
                    ],
                    builder: (context) => const Text(
                      'Drawer body content goes here. It can hold forms, '
                      'navigation or any custom layout.',
                    ),
                  ),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Backdrops',
          builder: (context) => Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              for (final backdrop in HeroDrawerBackdrop.values)
                HeroButton(
                  label: backdrop.name,
                  variant: HeroButtonVariant.secondary,
                  onPressed: () => showHeroDrawer<void>(
                    context,
                    backdrop: backdrop,
                    placement: HeroDrawerPlacement.right,
                    title: 'Backdrop: ${backdrop.name}',
                    builder: (context) =>
                        const Text('Drawer with a custom backdrop.'),
                  ),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With handle',
          builder: (context) => HeroButton(
            label: 'Bottom sheet',
            variant: HeroButtonVariant.secondary,
            onPressed: () => showHeroDrawer<void>(
              context,
              placement: HeroDrawerPlacement.bottom,
              showHandle: true,
              title: 'Bottom sheet',
              builder: (context) =>
                  const Text('A bottom drawer with a drag handle.'),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final placement =
                context.knobs.object.dropdown<HeroDrawerPlacement>(
                  label: 'Placement',
                  options: HeroDrawerPlacement.values,
                );
            return HeroButton(
              label: 'Open drawer',
              onPressed: () => showHeroDrawer<void>(
                context,
                placement: placement,
                title: 'Settings',
                description: 'Slide in panel',
                actions: [
                  Builder(
                    builder: (dialogContext) => HeroButton(
                      label: 'Cancel',
                      variant: HeroButtonVariant.ghost,
                      onPressed: () => Navigator.of(dialogContext).pop(),
                    ),
                  ),
                  Builder(
                    builder: (dialogContext) => HeroButton(
                      label: 'Save',
                      onPressed: () => Navigator.of(dialogContext).pop(),
                    ),
                  ),
                ],
                builder: (context) => const Text(
                  'Drawer content — add forms or settings here.',
                ),
              ),
            );
          },
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Dropdown',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Center(
            child: HeroDropdown(
              trigger: (open) => HeroButton(
                label: 'Actions',
                variant: HeroButtonVariant.secondary,
                onPressed: open,
              ),
              items: const [
                HeroDropdownItem(label: 'New file'),
                HeroDropdownItem(label: 'Copy link'),
                HeroDropdownItem(label: 'Edit file'),
                HeroDropdownItem(label: 'Delete file', danger: true),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'With icons & description',
          builder: (context) => Center(
            child: HeroDropdown(
              trigger: (open) => HeroButton(
                label: 'Open menu',
                variant: HeroButtonVariant.secondary,
                onPressed: open,
              ),
              items: const [
                HeroDropdownItem(
                  label: 'Edit',
                  leadingIcon: Icons.edit_rounded,
                  description: 'Edit this file',
                ),
                HeroDropdownItem(
                  label: 'Duplicate',
                  leadingIcon: Icons.copy_rounded,
                  description: 'Create a copy',
                ),
                HeroDropdownItem(
                  label: 'Archive',
                  leadingIcon: Icons.archive_rounded,
                  description: 'Move to archive',
                ),
                HeroDropdownSeparator(),
                HeroDropdownItem(
                  label: 'Delete',
                  leadingIcon: Icons.delete_rounded,
                  danger: true,
                ),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'States',
          builder: (context) => Center(
            child: HeroDropdown(
              trigger: (open) => HeroButton(
                label: 'States',
                variant: HeroButtonVariant.secondary,
                onPressed: open,
              ),
              items: const [
                HeroDropdownItem(label: 'Enabled item', onPressed: _noop),
                HeroDropdownItem(label: 'Selected item', selected: true),
                HeroDropdownItem(label: 'Disabled item', enabled: false),
                HeroDropdownItem(label: 'Danger item', danger: true),
              ],
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Placements',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 24,
            children: [
              for (final placement in HeroPopoverPlacement.values)
                HeroDropdown(
                  trigger: (open) => HeroButton(
                    label: placement.name,
                    variant: HeroButtonVariant.ghost,
                    onPressed: open,
                  ),
                  placement: placement,
                  items: const [
                    HeroDropdownItem(label: 'Item one'),
                    HeroDropdownItem(label: 'Item two'),
                  ],
                ),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Popover',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => Center(
            child: HeroPopover(
              trigger: (open) => HeroButton(
                label: 'Open popover',
                variant: HeroButtonVariant.secondary,
                onPressed: open,
              ),
              content: const HeroPopoverDialog(
                title: 'Popover title',
                child: Text(
                  'This is the popover content. It can hold any widget.',
                ),
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Placements',
          builder: (context) => Wrap(
            spacing: 24,
            runSpacing: 24,
            alignment: WrapAlignment.center,
            children: [
              for (final placement in HeroPopoverPlacement.values)
                HeroPopover(
                  trigger: (open) => HeroButton(
                    label: placement.name,
                    variant: HeroButtonVariant.ghost,
                    onPressed: open,
                  ),
                  placement: placement,
                  content: HeroPopoverDialog(
                    title: placement.name,
                    child: const Text('Popover content body.'),
                  ),
                ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'No arrow',
          builder: (context) => Center(
            child: HeroPopover(
              showArrow: false,
              trigger: (open) => HeroButton(
                label: 'No arrow',
                variant: HeroButtonVariant.secondary,
                onPressed: open,
              ),
              content: const HeroPopoverDialog(
                title: 'Clean popover',
                child: Text('Without the arrow pointing at the trigger.'),
              ),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final placement =
                context.knobs.object.dropdown<HeroPopoverPlacement>(
                  label: 'Placement',
                  options: HeroPopoverPlacement.values,
                );
            final showArrow = context.knobs.boolean(
              label: 'Show arrow',
              initialValue: true,
            );
            return Center(
              child: HeroPopover(
                placement: placement,
                showArrow: showArrow,
                trigger: (open) => HeroButton(
                  label: 'Open popover',
                  onPressed: open,
                ),
                content: HeroPopoverDialog(
                  title: 'Interactive popover',
                  child: Text(
                    'Placement: $placement\nArrow: ${showArrow ? 'yes' : 'no'}',
                  ),
                ),
              ),
            );
          },
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Breadcrumbs',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroBreadcrumbs(
            items: [
              HeroBreadcrumbItem(label: 'Home', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Components', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Breadcrumbs'),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With icons',
          builder: (context) => const HeroBreadcrumbs(
            items: [
              HeroBreadcrumbItem(
                label: 'Home',
                icon: Icons.home_rounded,
                onPressed: _noop,
              ),
              HeroBreadcrumbItem(
                label: 'Library',
                icon: Icons.folder_rounded,
                onPressed: _noop,
              ),
              HeroBreadcrumbItem(
                label: 'Components',
                icon: Icons.widgets_rounded,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Many levels',
          builder: (context) => const HeroBreadcrumbs(
            items: [
              HeroBreadcrumbItem(label: 'Root', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Org', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Team', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Project', onPressed: _noop),
              HeroBreadcrumbItem(label: 'Settings'),
            ],
          ),
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Pagination',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroPagination(
            page: 5,
            totalPages: 12,
            onPageChanged: _noopValue,
          ),
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: (context) => Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              for (final size in HeroPaginationSize.values) ...[
                HeroPagination(
                  page: 4,
                  totalPages: 10,
                  size: size,
                  onPageChanged: _noopValue,
                ),
                const SizedBox(height: 16),
              ],
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With summary',
          builder: (context) => const HeroPagination(
            page: 2,
            totalPages: 8,
            onPageChanged: _noopValue,
            summary: Text(
              'Showing 1 to 10 of 100 results',
              style: TextStyle(fontSize: 14),
            ),
          ),
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: (context) => const HeroPagination(
            page: 1,
            totalPages: 5,
            disabled: true,
            onPageChanged: _noopValue,
          ),
        ),
        WidgetbookUseCase(
          name: 'Interactive',
          builder: (context) {
            final total = context.knobs.int.slider(
              label: 'Total pages',
              min: 3,
              max: 20,
              initialValue: 10,
            );
            final loop = context.knobs.boolean(label: 'Loop');
            return StatefulBuilder(
              builder: (context, setState) {
                final page = _page < 1 ? 1 : (_page > total ? total : _page);
                return HeroPagination(
                  page: page,
                  totalPages: total,
                  loop: loop,
                  onPageChanged: (p) => setState(() => _page = p),
                );
              },
            );
          },
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ButtonGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: (context) => const HeroButtonGroup(
            children: [
              HeroButton(
                label: 'Cut',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Copy',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Paste',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'With separators',
          builder: (context) => const HeroButtonGroup(
            showSeparators: true,
            children: [
              HeroButton(
                label: 'Day',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Week',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Month',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Year',
                variant: HeroButtonVariant.secondary,
                onPressed: _noop,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Vertical',
          builder: (context) => const HeroButtonGroup(
            orientation: HeroButtonGroupOrientation.vertical,
            showSeparators: true,
            children: [
              HeroButton(
                label: 'Save',
                variant: HeroButtonVariant.secondary,
                leadingIcon: Icons.save_rounded,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Discard',
                variant: HeroButtonVariant.secondary,
                leadingIcon: Icons.delete_rounded,
                onPressed: _noop,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Full width',
          builder: (context) => const HeroButtonGroup(
            fullWidth: true,
            children: [
              HeroButton(
                label: 'All',
                variant: HeroButtonVariant.tertiary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Unread',
                variant: HeroButtonVariant.tertiary,
                onPressed: _noop,
              ),
              HeroButton(
                label: 'Starred',
                variant: HeroButtonVariant.tertiary,
                onPressed: _noop,
              ),
            ],
          ),
        ),
        WidgetbookUseCase(
          name: 'Single',
          builder: (context) => const HeroButtonGroup(
            children: [
              HeroButton(
                label: 'Solo',
                variant: HeroButtonVariant.primary,
                onPressed: _noop,
              ),
            ],
          ),
        ),
      ],
    ),
  ];
}

int _page = 1;

void _noop() {}

void _noopValue(int _) {}

import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

/// Content/typographic component use cases: Label, Description, Header,
/// Link, Kbd, Form, Fieldset, FieldError, ErrorMessage, Tag, TagGroup,
/// Typography, Surface, ScrollShadow, CloseButton, Toolbar — mirroring the
/// HeroUI storybook stories.
List<WidgetbookNode> contentUseCases() {
  return [
    WidgetbookComponent(
      name: 'Label',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _labelBuilder,
        ),
        WidgetbookUseCase(
          name: 'Required',
          builder: _labelRequiredBuilder,
        ),
        WidgetbookUseCase(
          name: 'Invalid',
          builder: _labelInvalidBuilder,
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: _labelDisabledBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Description',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _descriptionBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Header',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _headerBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Link',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _linkBuilder,
        ),
        WidgetbookUseCase(
          name: 'With icon',
          builder: _linkIconBuilder,
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: _linkDisabledBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Kbd',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _kbdBuilder,
        ),
        WidgetbookUseCase(
          name: 'Light',
          builder: _kbdLightBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Form',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _formBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Fieldset',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _fieldsetBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'FieldError',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _fieldErrorBuilder,
        ),
        WidgetbookUseCase(
          name: 'Hidden',
          builder: _fieldErrorHiddenBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ErrorMessage',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _errorMessageBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Tag',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _tagBuilder,
        ),
        WidgetbookUseCase(
          name: 'Sizes',
          builder: _tagSizesBuilder,
        ),
        WidgetbookUseCase(
          name: 'Variants',
          builder: _tagVariantsBuilder,
        ),
        WidgetbookUseCase(
          name: 'Selected',
          builder: _tagSelectedBuilder,
        ),
        WidgetbookUseCase(
          name: 'With remove',
          builder: _tagRemoveBuilder,
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: _tagDisabledBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'TagGroup',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _tagGroupBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Typography',
      useCases: [
        WidgetbookUseCase(
          name: 'Headings',
          builder: _typoHeadingsBuilder,
        ),
        WidgetbookUseCase(
          name: 'Body',
          builder: _typoBodyBuilder,
        ),
        WidgetbookUseCase(
          name: 'Code',
          builder: _typoCodeBuilder,
        ),
        WidgetbookUseCase(
          name: 'Muted',
          builder: _typoMutedBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Surface',
      useCases: [
        WidgetbookUseCase(
          name: 'Variants',
          builder: _surfaceBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'ScrollShadow',
      useCases: [
        WidgetbookUseCase(
          name: 'Vertical',
          builder: _scrollShadowBuilder,
        ),
        WidgetbookUseCase(
          name: 'Horizontal',
          builder: _scrollShadowHBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'CloseButton',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _closeButtonBuilder,
        ),
        WidgetbookUseCase(
          name: 'Disabled',
          builder: _closeButtonDisabledBuilder,
        ),
      ],
    ),
    WidgetbookComponent(
      name: 'Toolbar',
      useCases: [
        WidgetbookUseCase(
          name: 'Default',
          builder: _toolbarBuilder,
        ),
        WidgetbookUseCase(
          name: 'Attached',
          builder: _toolbarAttachedBuilder,
        ),
        WidgetbookUseCase(
          name: 'Vertical',
          builder: _toolbarVerticalBuilder,
        ),
      ],
    ),
  ];
}

Widget _labelBuilder(BuildContext context) => const HeroLabel(text: 'Email address');

Widget _labelRequiredBuilder(BuildContext context) =>
    const HeroLabel(text: 'Email address', required: true);

Widget _labelInvalidBuilder(BuildContext context) =>
    const HeroLabel(text: 'Email address', invalid: true);

Widget _labelDisabledBuilder(BuildContext context) =>
    const HeroLabel(text: 'Email address', disabled: true);

Widget _descriptionBuilder(BuildContext context) =>
    const HeroDescription('We’ll never share your email with anyone else.');

Widget _headerBuilder(BuildContext context) => const HeroHeader('Account settings');

Widget _linkBuilder(BuildContext context) => HeroLink(
      label: 'Forgot password?',
      onPressed: () {},
    );

Widget _linkIconBuilder(BuildContext context) => HeroLink(
      label: 'Learn more',
      icon: const Icon(Icons.arrow_outward_rounded, size: 16),
      onPressed: () {},
    );

Widget _linkDisabledBuilder(BuildContext context) =>
    const HeroLink(label: 'Forgot password?', disabled: true);

Widget _kbdBuilder(BuildContext context) => const HeroKbd('⌘ K');

Widget _kbdLightBuilder(BuildContext context) => const HeroKbd('⌘ K', light: true);

Widget _formBuilder(BuildContext context) => SizedBox(
      width: 360,
      child: HeroForm(
        children: [
          const HeroFieldset(
            children: [
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  HeroLabel(text: 'Full name'),
                  SizedBox(height: 8),
                  HeroInput(hintText: 'Ada Lovelace'),
                ],
              ),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  HeroLabel(text: 'Email'),
                  SizedBox(height: 8),
                  HeroInput(hintText: 'ada@example.com'),
                ],
              ),
            ],
          ),
          HeroButton(label: 'Save', variant: HeroButtonVariant.primary, onPressed: () {}),
        ],
      ),
    );

Widget _fieldsetBuilder(BuildContext context) => const SizedBox(
      width: 360,
      child: HeroFieldset(
        children: [
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              HeroLabel(text: 'Username'),
              SizedBox(height: 8),
              HeroInput(hintText: 'ada'),
            ],
          ),
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              HeroLabel(text: 'Password'),
              SizedBox(height: 8),
              HeroInput(hintText: '••••••••'),
            ],
          ),
        ],
      ),
    );

Widget _fieldErrorBuilder(BuildContext context) =>
    const HeroFieldError('This field is required.');

Widget _fieldErrorHiddenBuilder(BuildContext context) =>
    const HeroFieldError('', visible: false);

Widget _errorMessageBuilder(BuildContext context) =>
    const HeroErrorMessage('Something went wrong. Please try again.');

Widget _tagBuilder(BuildContext context) => HeroTag(
      label: 'React',
      onPressed: () {},
    );

Widget _tagSizesBuilder(BuildContext context) => Wrap(
      spacing: 8,
      runSpacing: 8,
      crossAxisAlignment: WrapCrossAlignment.center,
      children: [
        for (final size in HeroTagSize.values)
          HeroTag(label: size.name, size: size, onPressed: () {}),
      ],
    );

Widget _tagVariantsBuilder(BuildContext context) => Wrap(
      spacing: 8,
      runSpacing: 8,
      children: [
        for (final variant in HeroTagVariant.values)
          HeroTag(label: variant.name, variant: variant, onPressed: () {}),
      ],
    );

Widget _tagSelectedBuilder(BuildContext context) => Wrap(
      spacing: 8,
      children: [
        HeroTag(label: 'React', selected: true, onPressed: () {}),
        HeroTag(label: 'Vue', onPressed: () {}),
      ],
    );

Widget _tagRemoveBuilder(BuildContext context) => HeroTag(
      label: 'React',
      onPressed: () {},
      onRemove: () {},
    );

Widget _tagDisabledBuilder(BuildContext context) =>
    const HeroTag(label: 'React', disabled: true);

Widget _tagGroupBuilder(BuildContext context) => HeroTagGroup(
      label: const HeroLabel(text: 'Framework'),
      description: 'Choose frameworks to include.',
      children: [
        for (final (label, selected) in [
          ('React', true),
          ('Vue', false),
          ('Svelte', false),
          ('Solid', false),
        ])
          HeroTag(label: label, selected: selected, onPressed: () {}),
      ],
    );

Widget _typoHeadingsBuilder(BuildContext context) => const Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        HeroTypography('Display heading', variant: HeroTypographyVariant.h1),
        HeroTypography('Heading one', variant: HeroTypographyVariant.h2),
        HeroTypography('Heading two', variant: HeroTypographyVariant.h3),
        HeroTypography('Heading three', variant: HeroTypographyVariant.h4),
        HeroTypography('Heading four', variant: HeroTypographyVariant.h5),
        HeroTypography('Heading five', variant: HeroTypographyVariant.h6),
      ],
    );

Widget _typoBodyBuilder(BuildContext context) => const Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        HeroTypography('Body — the quick brown fox jumps over the lazy dog.',
            variant: HeroTypographyVariant.body),
        SizedBox(height: 8),
        HeroTypography('Body small — the quick brown fox jumps over the lazy dog.',
            variant: HeroTypographyVariant.bodySm),
        SizedBox(height: 8),
        HeroTypography('Body extra small — the quick brown fox.',
            variant: HeroTypographyVariant.bodyXs),
      ],
    );

Widget _typoCodeBuilder(BuildContext context) => const Wrap(
      crossAxisAlignment: WrapCrossAlignment.center,
      spacing: 8,
      runSpacing: 8,
      children: [
        HeroTypography('npm install @heroui/react', variant: HeroTypographyVariant.code),
        HeroTypography('flutter pub get', variant: HeroTypographyVariant.code),
      ],
    );

Widget _typoMutedBuilder(BuildContext context) => const Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        HeroTypography('Muted heading', variant: HeroTypographyVariant.h4,
            color: HeroTypographyColor.muted),
        SizedBox(height: 8),
        HeroTypography('Muted body text for secondary information.',
            variant: HeroTypographyVariant.bodySm, color: HeroTypographyColor.muted),
      ],
    );

Widget _surfaceBuilder(BuildContext context) => Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        for (final variant in HeroSurfaceVariant.values)
          Padding(
            padding: const EdgeInsets.only(bottom: 8),
            child: HeroSurface(
              variant: variant,
              padding: const EdgeInsets.all(16),
              child: Text('Surface ${variant.name}'),
            ),
          ),
      ],
    );

Widget _scrollShadowBuilder(BuildContext context) => SizedBox(
      height: 160,
      child: HeroScrollShadow(
        child: Column(
          children: [
            for (var i = 0; i < 20; i++)
              ListTile(
                dense: true,
                leading: Icon(Icons.chevron_right_rounded, color: HeroTokens.colorMuted.resolve(context)),
                title: Text('Item $i'),
              ),
          ],
        ),
      ),
    );

Widget _scrollShadowHBuilder(BuildContext context) => SizedBox(
      height: 64,
      child: HeroScrollShadow(
        horizontal: true,
        vertical: false,
        child: Row(
          children: [
            for (var i = 0; i < 30; i++)
              Padding(
                padding: const EdgeInsets.all(4),
                child: HeroChip(label: 'Item $i'),
              ),
          ],
        ),
      ),
    );

Widget _closeButtonBuilder(BuildContext context) => const HeroCloseButton(onPressed: _noop);

Widget _closeButtonDisabledBuilder(BuildContext context) => const HeroCloseButton(disabled: true);

Widget _toolbarBuilder(BuildContext context) => HeroToolbar(
      children: [
        HeroButton(label: 'Bold', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_bold_rounded, onPressed: () {}),
        HeroButton(label: 'Italic', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_italic_rounded, onPressed: () {}),
        HeroButton(label: 'Underline', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_underlined_rounded, onPressed: () {}),
      ],
    );

Widget _toolbarAttachedBuilder(BuildContext context) => HeroToolbar(
      attached: true,
      children: [
        HeroButton(label: 'Bold', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_bold_rounded, onPressed: () {}),
        HeroButton(label: 'Italic', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_italic_rounded, onPressed: () {}),
        HeroButton(label: 'Underline', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_underlined_rounded, onPressed: () {}),
      ],
    );

Widget _toolbarVerticalBuilder(BuildContext context) => HeroToolbar(
      vertical: true,
      children: [
        HeroButton(label: 'Bold', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_bold_rounded, onPressed: () {}),
        HeroButton(label: 'Italic', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_italic_rounded, onPressed: () {}),
        HeroButton(label: 'Underline', variant: HeroButtonVariant.ghost, leadingIcon: Icons.format_underlined_rounded, onPressed: () {}),
      ],
    );

void _noop() {}

import 'package:flutter/material.dart';

/// A HeroUI v3 form (form.css) — a semantic container for a group of fields.
///
/// HeroUI v3 ships no visual styles for `<form>` (the spec is absent from
/// @heroui/styles); this is a thin layout wrapper that stacks fields
/// vertically with the standard fieldset gap, matching how HeroUI docs
/// compose forms.
class HeroForm extends StatelessWidget {
  const HeroForm({
    super.key,
    required this.children,
    this.gap,
  });

  final List<Widget> children;
  final double? gap;

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        for (var i = 0; i < children.length; i++) ...[
          if (i > 0) SizedBox(height: gap ?? 24),
          children[i],
        ],
      ],
    );
  }
}

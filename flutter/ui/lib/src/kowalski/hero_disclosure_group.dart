import 'package:flutter/material.dart';

import 'hero_disclosure.dart';

/// A HeroUI v3 disclosure group (disclosure-group.css `.disclosure-group`).
///
/// A `w-full` container that coordinates several [HeroDisclosure] children,
/// optionally keeping only one open at a time (react-aria DisclosureGroup
/// `allowsMultipleExpanded`). Each child is rebuilt in controlled mode with
/// the group's shared expansion state.
///
/// ```dart
/// HeroDisclosureGroup(
///   children: [
///     HeroDisclosure(title: 'First', child: const Text('...')),
///     HeroDisclosure(title: 'Second', child: const Text('...')),
///   ],
/// )
/// ```
class HeroDisclosureGroup extends StatefulWidget {
  const HeroDisclosureGroup({
    super.key,
    required this.children,
    this.allowsMultipleExpanded = true,
    this.initialExpandedIndices = const {},
  });

  final List<HeroDisclosure> children;

  /// When false, opening one disclosure collapses the others.
  final bool allowsMultipleExpanded;

  /// Indices expanded on first build.
  final Set<int> initialExpandedIndices;

  @override
  State<HeroDisclosureGroup> createState() => _HeroDisclosureGroupState();
}

class _HeroDisclosureGroupState extends State<HeroDisclosureGroup> {
  late final Set<int> _expanded;

  @override
  void initState() {
    super.initState();
    _expanded = {...widget.initialExpandedIndices};
  }

  void _toggle(int index) {
    setState(() {
      if (_expanded.contains(index)) {
        _expanded.remove(index);
      } else {
        if (!widget.allowsMultipleExpanded) {
          _expanded.clear();
        }
        _expanded.add(index);
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        for (var i = 0; i < widget.children.length; i++)
          // Drive each child in controlled mode so the group owns the
          // shared expansion set.
          HeroDisclosure(
            key: widget.children[i].key,
            title: widget.children[i].title,
            isExpanded: _expanded.contains(i),
            onExpandedChange: (_) => _toggle(i),
            isDisabled: widget.children[i].isDisabled,
            child: widget.children[i].child,
          ),
      ],
    );
  }
}

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 pagination sizes (pagination.css `.pagination--sm/md/lg`).
enum HeroPaginationSize { sm, md, lg }

/// Metrics for one [HeroPaginationSize] (desktop `md:` values kick in at the
/// 768px breakpoint).
class _HeroPaginationMetrics {
  const _HeroPaginationMetrics({
    required this.pageSize,
    required this.mdPageSize,
    required this.fontSize,
    required this.navPaddingX,
    required this.pressScale,
  });

  final double pageSize;
  final double mdPageSize;
  final double fontSize;
  final double navPaddingX;
  final double pressScale;

  static const _HeroPaginationMetrics sm = _HeroPaginationMetrics(
    pageSize: 32,
    mdPageSize: 28,
    fontSize: 12,
    navPaddingX: 8,
    pressScale: 0.98,
  );

  static const _HeroPaginationMetrics md = _HeroPaginationMetrics(
    pageSize: 36,
    mdPageSize: 32,
    fontSize: 14,
    navPaddingX: 10,
    pressScale: 0.97,
  );

  static const _HeroPaginationMetrics lg = _HeroPaginationMetrics(
    pageSize: 40,
    mdPageSize: 36,
    fontSize: 16,
    navPaddingX: 12,
    pressScale: 0.96,
  );
}

const Map<HeroPaginationSize, _HeroPaginationMetrics> _heroPaginationMetrics =
    {
  HeroPaginationSize.sm: _HeroPaginationMetrics.sm,
  HeroPaginationSize.md: _HeroPaginationMetrics.md,
  HeroPaginationSize.lg: _HeroPaginationMetrics.lg,
};

/// One rendered pagination slot: a page number or an ellipsis.
sealed class _HeroPaginationSlot {
  const _HeroPaginationSlot();
}

class _HeroPageSlot extends _HeroPaginationSlot {
  const _HeroPageSlot(this.page);
  final int page;
}

class _HeroEllipsisSlot extends _HeroPaginationSlot {
  const _HeroEllipsisSlot();
}

/// A HeroUI v3 pagination (pagination.css) — previous/next navigation,
/// page buttons with an active fill, and ellipses between page groups.
///
/// ```dart
/// HeroPagination(
///   page: page,
///   totalPages: 12,
///   onPageChanged: (p) => setState(() => page = p),
/// )
/// ```
class HeroPagination extends StatelessWidget {
  const HeroPagination({
    super.key,
    required this.page,
    required this.totalPages,
    required this.onPageChanged,
    this.size = HeroPaginationSize.md,
    this.siblings = 1,
    this.boundaries = 1,
    this.loop = false,
    this.disabled = false,
    this.summary,
  });

  /// The current page (1-based).
  final int page;

  /// The total number of pages.
  final int totalPages;

  final ValueChanged<int> onPageChanged;

  final HeroPaginationSize size;

  /// Pages shown around the current page.
  final int siblings;

  /// Pages shown at the start/end.
  final int boundaries;

  /// When true, prev from page 1 goes to the last page and vice versa.
  final bool loop;

  final bool disabled;

  /// Optional summary slot (`.pagination__summary` — `text-sm text-muted`,
  /// `gap-2`).
  final Widget? summary;

  @override
  Widget build(BuildContext context) {
    final metrics = _heroPaginationMetrics[size]!;
    final isDesktop = MediaQuery.sizeOf(context).width >= 768;
    final fontSize = metrics.fontSize;
    final isWide = MediaQuery.sizeOf(context).width >= 640;
    final buttonSize = isDesktop ? metrics.mdPageSize : metrics.pageSize;

    final slots = _heroPaginationSlots(
      page: page,
      totalPages: totalPages,
      siblings: siblings,
      boundaries: boundaries,
    );

    // `.pagination__content` — `flex items-center gap-1 self-start
    // sm:self-center`.
    final content = Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        _HeroNavButton(
          icon: Icons.chevron_left,
          size: buttonSize,
          paddingX: metrics.navPaddingX,
          fontSize: fontSize,
          enabled: !disabled && (loop || page > 1),
          onTap: () => onPageChanged(page > 1 ? page - 1 : totalPages),
          label: 'Previous page',
        ),
        const SizedBox(width: 4), // gap-1
        for (final slot in slots) ...[
          switch (slot) {
            _HeroPageSlot() => _HeroPageButton(
                page: slot.page,
                active: slot.page == page,
                enabled: !disabled,
                size: buttonSize,
                fontSize: fontSize,
                pressScale: metrics.pressScale,
                onTap: () => onPageChanged(slot.page),
              ),
            _HeroEllipsisSlot() => _HeroEllipsis(
                size: buttonSize,
                fontSize: fontSize,
              ),
          },
          const SizedBox(width: 4), // gap-1
        ],
        _HeroNavButton(
          icon: Icons.chevron_right,
          size: buttonSize,
          paddingX: metrics.navPaddingX,
          fontSize: fontSize,
          enabled: !disabled && (loop || page < totalPages),
          onTap: () => onPageChanged(page < totalPages ? page + 1 : 1),
          label: 'Next page',
        ),
      ],
    );

    // `.pagination` — `flex w-full flex-col items-center justify-between
    // gap-4 sm:flex-row`; summary and content `self-start sm:self-center`.
    if (isWide) {
      return Row(
        mainAxisAlignment: summary != null
            ? MainAxisAlignment.spaceBetween
            : MainAxisAlignment.start,
        children: [
          if (summary != null) summary!,
          content,
        ],
      );
    }
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (summary != null) ...[
          summary!,
          const SizedBox(height: 16), // gap-4
        ],
        content,
      ],
    );
  }
}

/// Computes the visible page set with ellipses (bounded window).
List<_HeroPaginationSlot> _heroPaginationSlots({
  required int page,
  required int totalPages,
  required int siblings,
  required int boundaries,
}) {
  final result = <_HeroPaginationSlot>[];
  final seen = <int>{};

  void add(int p) {
    if (p < 1 || p > totalPages || !seen.add(p)) return;
    if (result.isNotEmpty && p - (result.last as _HeroPageSlot).page > 1) {
      result.add(const _HeroEllipsisSlot());
    }
    result.add(_HeroPageSlot(p));
  }

  for (var p = 1; p <= boundaries; p++) {
    add(p);
  }
  for (var p = page - siblings; p <= page + siblings; p++) {
    add(p);
  }
  for (var p = totalPages - boundaries + 1; p <= totalPages; p++) {
    add(p);
  }

  if (result.isNotEmpty && result.last is _HeroEllipsisSlot) {
    result.removeLast();
  }
  return result;
}

class _HeroPageButton extends StatefulWidget {
  const _HeroPageButton({
    required this.page,
    required this.active,
    required this.enabled,
    required this.size,
    required this.fontSize,
    required this.pressScale,
    required this.onTap,
  });

  final int page;
  final bool active;
  final bool enabled;
  final double size;
  final double fontSize;
  final double pressScale;
  final VoidCallback onTap;

  @override
  State<_HeroPageButton> createState() => _HeroPageButtonState();
}

class _HeroPageButtonState extends State<_HeroPageButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final size = widget.size;

    // `.pagination__link` — ghost button: transparent bg, `--default-hover`
    // on hover/press; `[data-active]` swaps to `bg-default` with
    // `--default-foreground` text.
    // Hover/press always paint `--pagination-link-bg-hover`; the active
    // page swaps the resting fill to `bg-default` (`[data-active]`).
    final background = _pressed || _hovered
        ? HeroTokens.colorDefaultHover.resolve(context)
        : widget.active
            ? HeroTokens.colorDefault.resolve(context)
            : HeroTokens.colorTransparent.resolve(context);

    return Opacity(
      opacity: widget.enabled
          ? 1.0
          : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: HeroFocusRing(
        radius: HeroTokens.radius3xl.resolve(context).x,
        builder: (context, node, focused) => MouseRegion(
          cursor:
              widget.enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
          onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
          onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
          child: Focus(
            focusNode: node,
            canRequestFocus: widget.enabled,
            onKeyEvent: widget.enabled
                ? (node, event) {
                    if (event is KeyDownEvent &&
                        (event.logicalKey == LogicalKeyboardKey.enter ||
                            event.logicalKey == LogicalKeyboardKey.space)) {
                      widget.onTap();
                      return KeyEventResult.handled;
                    }
                    return KeyEventResult.ignored;
                  }
                : null,
            child: GestureDetector(
              behavior: HitTestBehavior.opaque,
              onTap: widget.enabled ? widget.onTap : null,
              onTapDown:
                  widget.enabled ? (_) => setState(() => _pressed = true) : null,
              onTapUp:
                  widget.enabled ? (_) => setState(() => _pressed = false) : null,
              onTapCancel:
                  widget.enabled ? () => setState(() => _pressed = false) : null,
              child: AnimatedScale(
                // `&:active` — scale(0.97/0.98/0.96) over 250ms ease-smooth.
                scale: _pressed ? widget.pressScale : 1,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
                ),
                curve: heroEaseSmooth,
                child: AnimatedContainer(
                  // bg transition 100ms `--ease-out`.
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 100),
                  ),
                  curve: heroEaseOut,
                  width: size,
                  height: size,
                  alignment: Alignment.center,
                  decoration: BoxDecoration(
                    color: background,
                    borderRadius: BorderRadius.circular(
                      HeroTokens.radius3xl.resolve(context).x,
                    ),
                  ),
                  child: Text(
                    '${widget.page}',
                    style: TextStyle(
                      fontSize: widget.fontSize,
                      fontWeight: HeroTokens.weightMedium.resolve(context),
                      color: HeroTokens.colorDefaultForeground.resolve(context),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class _HeroEllipsis extends StatelessWidget {
  const _HeroEllipsis({
    required this.size,
    required this.fontSize,
  });

  final double size;
  final double fontSize;

  @override
  Widget build(BuildContext context) {
    // `.pagination__ellipsis` — `inline-flex size-9 items-center
    // justify-center text-sm text-muted select-none md:size-8`.
    return Container(
      width: size,
      height: size,
      alignment: Alignment.center,
      child: Text(
        '…',
        style: TextStyle(
          fontSize: fontSize,
          color: HeroTokens.colorMuted.resolve(context),
        ),
      ),
    );
  }
}

class _HeroNavButton extends StatefulWidget {
  const _HeroNavButton({
    required this.icon,
    required this.size,
    required this.paddingX,
    required this.fontSize,
    required this.enabled,
    required this.onTap,
    required this.label,
  });

  final IconData icon;
  final double size;
  final double paddingX;
  final double fontSize;
  final bool enabled;
  final VoidCallback onTap;
  final String label;

  @override
  State<_HeroNavButton> createState() => _HeroNavButtonState();
}

class _HeroNavButtonState extends State<_HeroNavButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final size = widget.size;
    final background = _pressed || _hovered
        ? HeroTokens.colorDefaultHover.resolve(context)
        : HeroTokens.colorTransparent.resolve(context);

    return Opacity(
      opacity: widget.enabled
          ? 1.0
          : HeroTokens.doubleDisabledOpacity.resolve(context),
      child: HeroFocusRing(
        radius: HeroTokens.radius3xl.resolve(context).x,
        builder: (context, node, focused) => MouseRegion(
          cursor:
              widget.enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
          onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
          onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
          child: Focus(
            focusNode: node,
            canRequestFocus: widget.enabled,
            child: GestureDetector(
              behavior: HitTestBehavior.opaque,
              onTap: widget.enabled ? widget.onTap : null,
              onTapDown:
                  widget.enabled ? (_) => setState(() => _pressed = true) : null,
              onTapUp:
                  widget.enabled ? (_) => setState(() => _pressed = false) : null,
              onTapCancel:
                  widget.enabled ? () => setState(() => _pressed = false) : null,
              child: AnimatedScale(
                scale: _pressed ? 0.97 : 1,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
                ),
                curve: heroEaseSmooth,
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 100),
                  ),
                  curve: heroEaseOut,
                  height: size,
                  // `.pagination__link--nav` — `w-auto gap-1.5 px-2.5`.
                  padding: EdgeInsets.symmetric(horizontal: widget.paddingX),
                  alignment: Alignment.center,
                  decoration: BoxDecoration(
                    color: background,
                    borderRadius: BorderRadius.circular(
                      HeroTokens.radius3xl.resolve(context).x,
                    ),
                  ),
                  child: Semantics(
                    label: widget.label,
                    button: true,
                    child: Icon(
                      widget.icon,
                      size: widget.fontSize + 4,
                      color: HeroTokens.colorDefaultForeground.resolve(context),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

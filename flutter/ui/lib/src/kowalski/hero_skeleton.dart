import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 skeleton animations (skeleton.css `.skeleton--*`).
enum HeroSkeletonAnimation {
  /// Sweeping highlight (`--skeleton-animation: shimmer`, 2s linear).
  shimmer,

  /// Gentle pulse (`animate-pulse`).
  pulse,

  /// Static placeholder.
  none,
}

/// A HeroUI v3 skeleton placeholder (skeleton.css).
///
/// `.skeleton` — `rounded-sm` (4), `bg-surface-tertiary/70`. Remix has no
/// skeleton component, so this is a hand-written widget (no recipe): the
/// container color comes from the resolved token, and shimmer/pulse are driven
/// by a repeating controller that honors reduced motion.
class HeroSkeleton extends StatefulWidget {
  const HeroSkeleton({
    super.key,
    this.width,
    this.height = 20,
    this.borderRadius,
    this.animation = HeroSkeletonAnimation.shimmer,
  });

  /// Width; null fills the parent (skeleton.css default is w-full).
  final double? width;

  /// Height (`.skeleton` has no fixed height in the CSS; callers size it).
  final double height;

  /// Corner radius; defaults to `rounded-sm` (4).
  final BorderRadius? borderRadius;

  final HeroSkeletonAnimation animation;

  @override
  State<HeroSkeleton> createState() => _HeroSkeletonState();
}

class _HeroSkeletonState extends State<HeroSkeleton>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller = AnimationController(
    vsync: this,
    duration: const Duration(milliseconds: heroSkeletonAnimationMs),
  );

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    // MediaQuery (reduced-motion) lookups are not allowed in initState.
    _maybeStart();
  }

  @override
  void didUpdateWidget(HeroSkeleton oldWidget) {
    super.didUpdateWidget(oldWidget);
    _maybeStart();
  }

  void _maybeStart() {
    _controller.stop();
    final animates = widget.animation != HeroSkeletonAnimation.none &&
        !HeroMotion.reducedMotionOf(context);
    if (animates) {
      _controller.repeat();
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final radius =
        widget.borderRadius ??
        BorderRadius.all(HeroTokens.radiusSkeletonRadius.resolve(context));
    final background =
        HeroTokens.colorSurfaceTertiary.resolve(context).withValues(
              alpha: HeroTokens.doubleSkeletonOpacity.resolve(context),
            );

    // The static placeholder. It is passed to AnimatedBuilder via its `child`
    // parameter so the animation builder references only that parameter — it
    // never closes over a captured local widget (a captured-and-reassigned
    // local is what produced the earlier infinite tree / StackOverflow).
    final base = Container(
      width: widget.width,
      height: widget.height,
      decoration: BoxDecoration(color: background, borderRadius: radius),
    );

    switch (widget.animation) {
      case HeroSkeletonAnimation.shimmer:
        // Shimmer sweep: a white 50%-alpha gradient translating across the
        // placeholder (skeleton.css `.skeleton--shimmer`). Blend-mode overlay
        // is approximated with a plain translucent sweep (see worksheet).
        //
        // The overlay Stack is wrapped in an explicitly sized SizedBox: parents
        // such as a Wrap inside a scrollable give unbounded height, and
        // StackFit.expand would then demand an infinite height.
        return SizedBox(
          width: widget.width,
          height: widget.height,
          child: ClipRRect(
            borderRadius: radius,
            child: AnimatedBuilder(
              animation: _controller,
              child: base,
              builder: (context, child) {
                // Sweep uses the LAID-OUT width — a `widget.width ?? ∞`
                // offset leaves the sweep at ±∞ for width-null skeletons,
                // so the shimmer never enters the viewport.
                return LayoutBuilder(
                  builder: (context, constraints) {
                    final w = constraints.maxWidth;
                    final t = _controller.value * 3.0 - 1.0; // -1 -> 2
                    return Stack(
                      fit: StackFit.expand,
                      children: [
                        child!,
                        Transform.translate(
                          offset: Offset(w * t, 0),
                          child: FractionallySizedBox(
                            widthFactor: 0.5,
                            child: DecoratedBox(
                              decoration: BoxDecoration(
                                gradient: LinearGradient(
                                  colors: [
                                    HeroTokens.colorTransparent.resolve(context),
                                    // Visible over the surface-tertiary base.
                                    HeroTokens.colorAccentForeground.resolve(context)
                                        .withValues(alpha: 0.5),
                                    HeroTokens.colorTransparent.resolve(context),
                                  ],
                                ),
                              ),
                            ),
                          ),
                        ),
                      ],
                    );
                  },
                );
              },
            ),
          ),
        );
      case HeroSkeletonAnimation.pulse:
        return SizedBox(
          width: widget.width,
          height: widget.height,
          child: AnimatedBuilder(
            animation: _controller,
            child: base,
            builder: (context, child) => Opacity(
              opacity: 0.55 + 0.3 * _controller.value,
              child: child,
            ),
          ),
        );
      case HeroSkeletonAnimation.none:
        return base;
    }
  }
}

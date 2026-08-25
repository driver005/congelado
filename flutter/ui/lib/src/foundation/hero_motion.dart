import 'package:flutter/widgets.dart';

import '../tokens/hero_tokens.dart';

/// Motion helpers for the HeroUI design system.
///
/// HeroUI v3 moves every transition to CSS and honors
/// `prefers-reduced-motion` (`motion-reduce:transition-none`). This is the
/// Flutter analog: route all durations through [durationOf] so reduced-motion
/// users get instant transitions, matching the source's intent.
abstract final class HeroMotion {
  const HeroMotion._();

  /// Whether the platform requests reduced motion.
  static bool reducedMotionOf(BuildContext context) =>
      MediaQuery.maybeOf(context)?.disableAnimations ?? false;

  /// Returns [value] unless reduced motion is requested, in which case
  /// [Duration.zero].
  static Duration durationOf(BuildContext context, Duration value) =>
      reducedMotionOf(context) ? Duration.zero : value;

  /// HeroUI v3 easing curves (shared-theme.css `--ease-*`).
  static Curve get smooth => heroEaseSmooth;
  static Curve get outFluid => heroEaseOutFluid;
  static Curve get out => heroEaseOut;
  static Curve get inOut => heroEaseInOut;
  static Curve get linear => heroEaseLinear;
  static Curve get outQuart => heroEaseOutQuart;
}

/// `--ease-out-quart` (shared-theme.css) — used for transform transitions
/// (close-button, drawer, dropdown, alert-dialog).
const Cubic heroEaseOutQuart = Cubic(0.165, 0.84, 0.44, 1.0);

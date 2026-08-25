import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:mix/mix.dart';
import 'package:widgetbook/widgetbook.dart';

/// Foundation use cases: the generated token surface rendered visually.
List<WidgetbookNode> foundationUseCases() {
  return [
    WidgetbookComponent(name: 'Colors', useCases: [
      WidgetbookUseCase(name: 'Semantic roles', builder: (context) => const _ColorGrid()),
    ]),
    WidgetbookComponent(name: 'Typography', useCases: [
      WidgetbookUseCase(name: 'Type scale', builder: (context) => const _TypeScale()),
    ]),
    WidgetbookComponent(name: 'Radius', useCases: [
      WidgetbookUseCase(name: 'Radius scale', builder: (context) => const _RadiusScale()),
    ]),
    WidgetbookComponent(name: 'Shadows', useCases: [
      WidgetbookUseCase(name: 'Elevation tokens', builder: (context) => const _ShadowTiles()),
    ]),
    WidgetbookComponent(name: 'Spacing', useCases: [
      WidgetbookUseCase(name: 'Spacing scale', builder: (context) => const _SpacingScale()),
    ]),
  ];
}

/// The generated spacing scale (4px base, Tailwind v4).
class _SpacingScale extends StatelessWidget {
  const _SpacingScale();

  static const List<(String, SpaceToken)> _spaces = [
    ('0.5 — 2', HeroTokens.space05),
    ('1 — 4', HeroTokens.space1),
    ('1.5 — 6', HeroTokens.space15),
    ('2 — 8', HeroTokens.space2),
    ('3 — 12', HeroTokens.space3),
    ('4 — 16', HeroTokens.space4),
    ('5 — 20', HeroTokens.space5),
    ('6 — 24', HeroTokens.space6),
    ('8 — 32', HeroTokens.space8),
    ('10 — 40', HeroTokens.space10),
  ];

  @override
  Widget build(BuildContext context) {
    final foreground = HeroTokens.colorForeground.resolve(context);
    final accent = HeroTokens.colorAccent.resolve(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        for (final (label, token) in _spaces)
          Padding(
            padding: const EdgeInsets.symmetric(vertical: 4),
            child: Row(
              children: [
                SizedBox(
                  width: 90,
                  child: Text(
                    label,
                    style: TextStyle(fontSize: 12, color: foreground),
                  ),
                ),
                Container(
                  width: token.resolve(context),
                  height: 12,
                  decoration: BoxDecoration(
                    color: accent,
                    borderRadius: BorderRadius.circular(2),
                  ),
                ),
              ],
            ),
          ),
      ],
    );
  }
}

/// The HeroUI v3 semantic color roles (variables.css `--color-*`).
class _ColorGrid extends StatelessWidget {
  const _ColorGrid();

  static const List<(String, ColorToken)> _roles = [
    ('background', HeroTokens.colorBackground),
    ('foreground', HeroTokens.colorForeground),
    ('surface', HeroTokens.colorSurface),
    ('surface-secondary', HeroTokens.colorSurfaceSecondary),
    ('surface-tertiary', HeroTokens.colorSurfaceTertiary),
    ('overlay', HeroTokens.colorOverlay),
    ('overlay-foreground', HeroTokens.colorOverlayForeground),
    ('muted', HeroTokens.colorMuted),
    ('default', HeroTokens.colorDefault),
    ('default-foreground', HeroTokens.colorDefaultForeground),
    ('accent', HeroTokens.colorAccent),
    ('accent-foreground', HeroTokens.colorAccentForeground),
    ('accent-hover', HeroTokens.colorAccentHover),
    ('accent-soft', HeroTokens.colorAccentSoft),
    ('accent-soft-foreground', HeroTokens.colorAccentSoftForeground),
    ('success', HeroTokens.colorSuccess),
    ('success-soft', HeroTokens.colorSuccessSoft),
    ('warning', HeroTokens.colorWarning),
    ('warning-soft', HeroTokens.colorWarningSoft),
    ('danger', HeroTokens.colorDanger),
    ('danger-soft', HeroTokens.colorDangerSoft),
    ('danger-soft-foreground', HeroTokens.colorDangerSoftForeground),
    ('border', HeroTokens.colorBorder),
    ('separator', HeroTokens.colorSeparator),
    ('field', HeroTokens.colorField),
    ('field-placeholder', HeroTokens.colorFieldPlaceholder),
  ];

  @override
  Widget build(BuildContext context) {
    return Wrap(
      spacing: 8,
      runSpacing: 8,
      children: [
        for (final (label, token) in _roles) _tile(context, label, token),
      ],
    );
  }

  Widget _tile(BuildContext context, String label, ColorToken token) {
    final color = token.resolve(context);
    final foreground =
        color.computeLuminance() > 0.5
        ? HeroTokens.colorForeground.resolve(context)
        : HeroTokens.colorAccentForeground.resolve(context);
    return SizedBox(
      width: 170,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            height: 44,
            width: double.infinity,
            decoration: BoxDecoration(
              color: color,
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: HeroTokens.colorBorder.resolve(context)),
            ),
            alignment: Alignment.center,
            child: Text(
              'Aa',
              style: TextStyle(
                color: foreground,
                fontWeight: FontWeight.w600,
              ),
            ),
          ),
          const SizedBox(height: 4),
          Text(
            label,
            style: TextStyle(
              fontSize: 12,
              color: HeroTokens.colorForeground.resolve(context),
            ),
          ),
          Text(
            '#${color.toARGB32().toRadixString(16).padLeft(8, '0').substring(2).toUpperCase()}',
            style: TextStyle(
              fontSize: 11,
              color: HeroTokens.colorMuted.resolve(context),
            ),
          ),
        ],
      ),
    );
  }
}

/// The generated type scale (Tailwind defaults used by the component CSS).
class _TypeScale extends StatelessWidget {
  const _TypeScale();

  static const List<(String, TextStyleToken)> _types = [
    ('xs — 12/16', HeroTokens.typeXs),
    ('sm — 14/20', HeroTokens.typeSm),
    ('base — 16/24', HeroTokens.typeBase),
    ('lg — 18/28', HeroTokens.typeLg),
    ('xl — 20/28', HeroTokens.typeXl),
    ('2xl — 24/32', HeroTokens.type2xl),
    ('3xl — 30/36', HeroTokens.type3xl),
    ('4xl — 36/40', HeroTokens.type4xl),
  ];

  @override
  Widget build(BuildContext context) {
    final foreground = HeroTokens.colorForeground.resolve(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        for (final (label, token) in _types)
          Padding(
            padding: const EdgeInsets.symmetric(vertical: 8),
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.baseline,
              textBaseline: TextBaseline.alphabetic,
              children: [
                SizedBox(
                  width: 120,
                  child: Text(
                    label,
                    style: TextStyle(
                      fontSize: 12,
                      color: HeroTokens.colorMuted.resolve(context),
                    ),
                  ),
                ),
                Text(
                  'The quick brown fox',
                  style: token.resolve(context).copyWith(color: foreground),
                ),
              ],
            ),
          ),
      ],
    );
  }
}

/// The generated radius scale (--radius base 0.5rem = 8px).
class _RadiusScale extends StatelessWidget {
  const _RadiusScale();

  static const List<(String, RadiusToken)> _radii = [
    ('xs — 2', HeroTokens.radiusXs),
    ('sm — 4', HeroTokens.radiusSm),
    ('md — 6', HeroTokens.radiusMd),
    ('lg — 8', HeroTokens.radiusLg),
    ('xl — 12', HeroTokens.radiusXl),
    ('2xl — 16', HeroTokens.radius2xl),
    ('3xl — 24', HeroTokens.radius3xl),
    ('4xl — 32', HeroTokens.radius4xl),
    ('field — 12', HeroTokens.radiusField),
  ];

  @override
  Widget build(BuildContext context) {
    final surface = HeroTokens.colorSurface.resolve(context);
    final border = HeroTokens.colorBorder.resolve(context);
    final foreground = HeroTokens.colorForeground.resolve(context);
    return Wrap(
      spacing: 16,
      runSpacing: 16,
      children: [
        for (final (label, token) in _radii)
          SizedBox(
            width: 110,
            child: Column(
              children: [
                Container(
                  width: 72,
                  height: 72,
                  decoration: BoxDecoration(
                    color: surface,
                    borderRadius: BorderRadius.circular(token.resolve(context).x),
                    border: Border.all(color: border),
                  ),
                ),
                const SizedBox(height: 6),
                Text(
                  label,
                  style: TextStyle(fontSize: 12, color: foreground),
                ),
              ],
            ),
          ),
      ],
    );
  }
}

/// The generated shadow tokens (surface / overlay / field), per theme.
class _ShadowTiles extends StatelessWidget {
  const _ShadowTiles();

  static const List<(String, BoxShadowToken)> _tokens = [
    ('surface', HeroTokens.shadowSurface),
    ('overlay', HeroTokens.shadowOverlay),
    ('field', HeroTokens.shadowField),
  ];

  @override
  Widget build(BuildContext context) {
    final surface = HeroTokens.colorSurface.resolve(context);
    final foreground = HeroTokens.colorForeground.resolve(context);
    return Wrap(
      spacing: 24,
      runSpacing: 24,
      children: [
        for (final (label, token) in _tokens)
          SizedBox(
            width: 200,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Container(
                  width: 200,
                  height: 110,
                  decoration: BoxDecoration(
                    color: surface,
                    borderRadius: BorderRadius.circular(12),
                    boxShadow: token.resolve(context),
                  ),
                ),
                const SizedBox(height: 6),
                Text(
                  label,
                  style: TextStyle(fontSize: 12, color: foreground),
                ),
              ],
            ),
          ),
      ],
    );
  }
}

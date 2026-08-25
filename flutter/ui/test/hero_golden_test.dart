import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

/// Golden baseline: every Hero* component, light + dark, on a fixed surface.
///
/// Generate/refresh with:
/// ```bash
/// flutter test --update-goldens test/hero_golden_test.dart
/// ```
/// Token or geometry changes must show up here as a reviewable diff — this is
/// the "looks like HeroUI" gate (values derive from @heroui/styles@3.2.4).
void main() {
  for (final theme in HeroTheme.values) {
    final suffix = theme.name;

    Future<void> golden(WidgetTester tester, String name, Widget child,
        {double width = 320}) async {
      await tester.pumpWidget(
        HeroScope(
          theme: theme,
          child: MaterialApp(
            debugShowCheckedModeBanner: false,
            home: Scaffold(
              // Resolve the background inside the tree (a Builder below the
              // HeroScope), not from tester.element before the first pump.
              body: Builder(
                builder: (context) => ColoredBox(
                  color: HeroTokens.colorBackground.resolve(context),
                  child: Center(
                    child: RepaintBoundary(
                      child: SizedBox(width: width, child: child),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
      );
      // Deterministic frame for the repeating spinner/skeleton controllers.
      await tester.pump(const Duration(milliseconds: 375));
      await expectLater(
        find.byType(RepaintBoundary).last,
        matchesGoldenFile('goldens/${name}_$suffix.png'),
      );
    }

    testWidgets('button $suffix', (tester) async {
      await golden(
        tester,
        'button',
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: [
            HeroButton(label: 'Continue', onPressed: () {}),
            HeroButton(
              label: 'Secondary',
              variant: HeroButtonVariant.secondary,
              onPressed: () {},
            ),
            HeroButton(
              label: 'Danger',
              variant: HeroButtonVariant.danger,
              onPressed: () {},
            ),
            const HeroButton(label: 'Disabled'),
          ],
        ),
      );
    });

    testWidgets('card $suffix', (tester) async {
      await golden(
        tester,
        'card',
        const HeroCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              HeroCardTitle('Product'),
              HeroCardDescription('Details about this product.'),
              SizedBox(height: 12),
              HeroBadge(label: 'Healthy', color: HeroBadgeColor.success),
            ],
          ),
        ),
      );
    });

    testWidgets('input $suffix', (tester) async {
      await golden(
        tester,
        'input',
        Column(
          children: const [
            HeroInput(hintText: 'Enter something…'),
            SizedBox(height: 12),
            HeroInput(hintText: 'Error', error: true),
            SizedBox(height: 12),
            HeroInput(hintText: 'Disabled', enabled: false),
          ],
        ),
      );
    });

    testWidgets('chip badge $suffix', (tester) async {
      await golden(
        tester,
        'chip_badge',
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: const [
            HeroChip(label: 'Default'),
            HeroChip(label: 'Accent', color: HeroChipColor.accent),
            HeroChip(label: 'Danger', color: HeroChipColor.danger),
            HeroBadge(label: 'Default'),
            HeroBadge(
              label: 'Accent',
              color: HeroBadgeColor.accent,
              variant: HeroBadgeVariant.solid,
            ),
          ],
        ),
      );
    });

    testWidgets('tabs $suffix', (tester) async {
      await golden(
        tester,
        'tabs',
        // HeroTabs overflows the 320px preview (3 labels + indicator padding);
        // give it its natural width.
        HeroTabs(
          selectedTabId: 'a',
          tabs: const [
            HeroTab(id: 'a', label: 'Overview', child: Text('')),
            HeroTab(id: 'b', label: 'Activity', child: Text('')),
            HeroTab(id: 'c', label: 'Settings', child: Text('')),
          ],
        ),
        width: 520,
      );
    });

    testWidgets('controls $suffix', (tester) async {
      await golden(
        tester,
        'controls',
        Wrap(
          spacing: 24,
          runSpacing: 16,
          crossAxisAlignment: WrapCrossAlignment.center,
          children: [
            HeroSwitch(selected: true, onChanged: (_) {}),
            HeroCheckbox(selected: true, onChanged: (_) {}),
            HeroRadioGroup<String>(
              groupValue: 'a',
              onChanged: (_) {},
              child: const Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  HeroRadio(value: 'a'),
                  SizedBox(width: 8),
                  HeroRadio(value: 'b'),
                ],
              ),
            ),
          ],
        ),
      );
    });

    testWidgets('skeleton progress spinner $suffix', (tester) async {
      await golden(
        tester,
        'skeleton_progress_spinner',
        Column(
          children: [
            const SizedBox(
              width: 200,
              child: HeroSkeleton(height: 12, animation: HeroSkeletonAnimation.none),
            ),
            const SizedBox(height: 16),
            HeroProgress(value: 0.65),
            const SizedBox(height: 16),
            const Wrap(
              spacing: 16,
              children: [
                HeroSpinner(color: HeroSpinnerColor.accent),
                HeroSpinner(color: HeroSpinnerColor.danger),
              ],
            ),
          ],
        ),
      );
    });
  }
}

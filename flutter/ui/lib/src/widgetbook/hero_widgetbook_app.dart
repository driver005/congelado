import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:widgetbook/widgetbook.dart';

import 'use_cases/button_use_cases.dart';
import 'use_cases/card_use_cases.dart';
import 'use_cases/color_use_cases.dart';
import 'use_cases/content_use_cases.dart';
import 'use_cases/disclosure_use_cases.dart';
import 'use_cases/display_use_cases.dart';
import 'use_cases/date_use_cases.dart';
import 'use_cases/feedback_use_cases.dart';
import 'use_cases/plugins_use_cases.dart';
import 'use_cases/selection_use_cases.dart';
import 'use_cases/table_use_cases.dart';
import 'use_cases/form_use_cases.dart';
import 'use_cases/foundation_use_cases.dart';

/// The Widgetbook catalogue for the HeroUI v3 design-system mirror.
///
/// The **Theme addon drives `HeroScope`** (light/dark), so every use case
/// resolves tokens against the selected HeroUI theme. The chrome theme
/// (accent seed, same as the congelado shell app) is separate — it only
/// styles Widgetbook's own UI.
class HeroWidgetbookApp extends StatelessWidget {
  const HeroWidgetbookApp({super.key});

  static ThemeData _chromeTheme(BuildContext context, Brightness brightness) {
    final seed = HeroTokens.colorAccent.resolve(context);
    return ThemeData(
      useMaterial3: true,
      colorSchemeSeed: seed,
      brightness: brightness,
      fontFamily: 'Inter',
    );
  }

  @override
  Widget build(BuildContext context) {
    return Widgetbook.material(
      lightTheme: _chromeTheme(context, Brightness.light),
      darkTheme: _chromeTheme(context, Brightness.dark),
      themeMode: ThemeMode.system,
      addons: [
        // ThemeAddon is FIRST, so it is the outermost wrapper: HeroScope +
        // the HeroUI page background sit behind device frames too.
        ThemeAddon<HeroTheme>(
          themes: const [
            WidgetbookTheme(name: 'Light', data: HeroTheme.light),
            WidgetbookTheme(name: 'Dark', data: HeroTheme.dark),
          ],
          themeBuilder: (context, theme, child) => HeroScope(
            theme: theme,
            child: Builder(
              builder: (context) => ColoredBox(
                color: HeroTokens.colorBackground.resolve(context),
                child: child,
              ),
            ),
          ),
        ),
        DeviceFrameAddon(
          devices: [
            Devices.ios.iPhone13,
            Devices.ios.iPhone13Mini,
            Devices.ios.iPad,
            Devices.android.pixel4,
            Devices.android.samsungGalaxyS20,
          ],
        ),
        TextScaleAddon(),
        AlignmentAddon(),
        GridAddon(),
      ],
      directories: [
        WidgetbookFolder(name: 'Foundation', children: foundationUseCases()),
        WidgetbookFolder(
          name: 'Components',
          children: [
            ...buttonUseCases(),
            ...cardUseCases(),
            ...formUseCases(),
            ...disclosureUseCases(),
            ...displayUseCases(),
            ...colorUseCases(),
            ...contentUseCases(),
            ...tableUseCases(),
            ...selectionUseCases(),
            ...feedbackUseCases(),
            ...dateUseCases(),
          ],
        ),
        WidgetbookFolder(name: 'Plugins', children: pluginsUseCases()),
      ],
    );
  }
}

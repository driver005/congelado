import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:congelado_engine_ui/register.dart' as engine_ui;
import 'package:congelado_openapi_ui/register.dart' as openapi_ui;

/// Hand-maintained (never generated) merge of every installed plugin's UI
/// contributions. Onboarding a new plugin's UI: add its `path:` dependency
/// to pubspec.yaml, import its `register.dart` here, and append its
/// `contributions` list below. No C++ rebuild or engine restart involved —
/// purely a Flutter-side change.
List<PluginUiContribution> buildPluginContributions() {
  return [
    ...engine_ui.contributions,
    ...openapi_ui.contributions,
  ];
}

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';

import 'openapi_page.dart';

/// This plugin's nav contribution(s), imported and merged by
/// app/lib/registry.dart. Adding a page: append a [PluginUiContribution]
/// here — no other file in this package or the shell needs to change.
final List<PluginUiContribution> contributions = [
  const PluginUiContribution(
    id: 'openapi_generator.spec',
    label: 'API',
    icon: Icons.api_outlined,
    order: 40,
    builder: _buildOpenApiPage,
  ),
];

Widget _buildOpenApiPage(BuildContext context) => const OpenApiPage();

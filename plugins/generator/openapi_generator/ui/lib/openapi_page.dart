import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FItemGroup/FItem/FBadge/FCircularProgress were
// already correct as originally written (no fix needed here, unlike most other call sites this
// session) — see plugins/engine/ui/lib/*.dart's own notes for what those Forui fixes covered.

/// Shows the engine's live OpenAPI document — title/version from `info`,
/// then every path grouped with its declared HTTP methods, sourced from
/// `GET /openapi.json`'s standard OpenAPI `paths` shape (`{"/path":
/// {"get": {...}, "post": {...}}}`).
class OpenApiPage extends StatefulWidget {
  const OpenApiPage({super.key});

  @override
  State<OpenApiPage> createState() => _OpenApiPageState();
}

class _OpenApiPageState extends State<OpenApiPage> {
  final _api = OpenApiClient();
  late Future<Map<String, dynamic>> _spec = _api.fetchSpec();

  void _refresh() => setState(() => _spec = _api.fetchSpec());

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: const ['API'],
        actions: [
          IconButton(onPressed: _refresh, icon: const Icon(Icons.refresh)),
        ],
      ),
      child: FutureBuilder<Map<String, dynamic>>(
        future: _spec,
        builder: (context, snapshot) {
          if (snapshot.connectionState != ConnectionState.done) {
            return const Center(child: FCircularProgress());
          }
          if (snapshot.hasError) {
            return Center(
              child: Text('Failed to load OpenAPI document: ${snapshot.error}'),
            );
          }
          final spec = snapshot.data ?? const {};
          final info = spec['info'] as Map<String, dynamic>? ?? const {};
          final paths = spec['paths'] as Map<String, dynamic>? ?? const {};
          final sortedPaths = paths.keys.toList()..sort();

          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  '${info['title'] ?? 'API'} · v${info['version'] ?? '?'}',
                  style: Theme.of(context).textTheme.titleMedium,
                ),
                const SizedBox(height: 8),
                Text('${sortedPaths.length} path(s)'),
                const SizedBox(height: 16),
                Expanded(
                  child: ListView(
                    children: [
                      FItemGroup(
                        children: [
                          for (final path in sortedPaths)
                            FItem(
                              title: Text(path),
                              subtitle: Wrap(
                                spacing: 6,
                                children: [
                                  for (final method
                                      in (paths[path] as Map<String, dynamic>)
                                          .keys)
                                    FBadge(child: Text(method.toUpperCase())),
                                ],
                              ),
                            ),
                        ],
                      ),
                    ],
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}

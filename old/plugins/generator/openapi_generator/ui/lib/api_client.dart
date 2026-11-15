import 'dart:convert';

import 'package:http/http.dart' as http;

/// Minimal client for the engine's live-generated OpenAPI document —
/// `GET /openapi.json` (note: not under `/api/v1`, a top-level path — see
/// `plugins/openapi_generator/src/doc_generator.cppm`'s `m_serve_path`
/// default), regenerated fresh from the route/schema registries on every
/// hit by `IOpenApiGenerator::serve_document()`
/// (`sdk/heart/app.cppm`'s `ctx.get_router()->add_route(...)` wires it up
/// at server startup — no separate route to add for this UI).
class OpenApiClient {
  OpenApiClient({this.baseUrl = 'http://localhost:8080'});

  final String baseUrl;

  Future<Map<String, dynamic>> fetchSpec() async {
    final res = await http.get(Uri.parse('$baseUrl/openapi.json'));
    if (res.statusCode >= 400) {
      throw Exception(
        '${res.request?.method} ${res.request?.url} -> '
        '${res.statusCode}: ${res.body}',
      );
    }
    return jsonDecode(res.body) as Map<String, dynamic>;
  }
}

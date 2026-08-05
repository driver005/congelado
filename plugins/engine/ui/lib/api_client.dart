import 'dart:convert';

import 'package:http/http.dart' as http;

/// Minimal REST client for the engine plugin's `/api/v1/tasks`,
/// `/api/v1/workflows`, `/api/v1/metadata/*`, and `/api/v1/query`
/// endpoints — plain Dart HTTP, no code generation, no dependency on the
/// C++ backend beyond knowing these paths (documented in the engine's own
/// openapi.json).
class EngineApiClient {
  EngineApiClient({this.baseUrl = 'http://localhost:8080'});

  final String baseUrl;

  Future<List<Map<String, dynamic>>> listTasks() async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/metadata/tasks'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  Future<Map<String, dynamic>> getTask(String name) async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/tasks/$name'));
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> updateTask(
    String name,
    Map<String, dynamic> body,
  ) async {
    final res = await http.put(
      Uri.parse('$baseUrl/api/v1/tasks/$name'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<void> deleteTask(String name) async {
    final res = await http.delete(Uri.parse('$baseUrl/api/v1/tasks/$name'));
    _checkOk(res);
  }

  // ── Workflows ──────────────────────────────────────────────────────────

  Future<List<Map<String, dynamic>>> listWorkflows() async {
    final res = await http.get(
      Uri.parse('$baseUrl/api/v1/metadata/workflows'),
    );
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  Future<Map<String, dynamic>> getWorkflow(String name) async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/workflows/$name'));
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> updateWorkflow(
    String name,
    Map<String, dynamic> body,
  ) async {
    final res = await http.put(
      Uri.parse('$baseUrl/api/v1/workflows/$name'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<void> deleteWorkflow(String name) async {
    final res = await http.delete(
      Uri.parse('$baseUrl/api/v1/workflows/$name'),
    );
    _checkOk(res);
  }

  /// Starts a new execution of the `name` workflow definition, returning the
  /// created `WorkflowExecution` (which carries the new `exec_id`) — see
  /// `plugins/engine/src/core/handler/workflow.cppm`'s `start_execution`.
  Future<Map<String, dynamic>> startWorkflow(
    String name, {
    Map<String, String> variables = const {},
  }) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/workflows/$name/start'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode({'variables': variables}),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> getExecution(String execId) async {
    final res = await http.get(
      Uri.parse('$baseUrl/api/v1/workflows/exec/$execId'),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> terminateExecution(String execId) async {
    final res = await http.delete(
      Uri.parse('$baseUrl/api/v1/workflows/exec/$execId'),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  /// Creates a new workflow definition — `POST /api/v1/workflows`, see
  /// `plugins/engine/src/core/handler/workflow.cppm`'s `create_definition`.
  Future<Map<String, dynamic>> createWorkflow(Map<String, dynamic> body) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/workflows'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  /// Creates a new task definition — `POST /api/v1/tasks`, see
  /// `plugins/engine/src/core/handler/task.cppm`'s `create_definition`.
  Future<Map<String, dynamic>> createTask(Map<String, dynamic> body) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/tasks'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  // ── Execution lifecycle (beyond terminate) ──────────────────────────────
  //
  // pause/resume/retry/restart/rerun/signal all reply 200 with an EMPTY body on success
  // (`res.set_status(OK)`, no `set_body` — confirmed by reading every one of these handlers in
  // `plugins/engine/src/core/handler/workflow.cppm`) and a real error body on failure (409/404,
  // caught by `_checkOk`) — `_postNoBody` skips the `jsonDecode` the other methods on this class
  // do, since there is nothing to decode on the success path.

  Future<void> _postNoBody(String path, {Object? body}) async {
    final res = await http.post(
      Uri.parse('$baseUrl$path'),
      headers: body == null ? null : const {'content-type': 'application/json'},
      body: body == null ? null : jsonEncode(body),
    );
    _checkOk(res);
  }

  Future<void> pauseExecution(String execId) =>
      _postNoBody('/api/v1/workflows/exec/$execId/pause');
  Future<void> resumeExecution(String execId) =>
      _postNoBody('/api/v1/workflows/exec/$execId/resume');
  Future<void> retryExecution(String execId) =>
      _postNoBody('/api/v1/workflows/exec/$execId/retry');
  Future<void> restartExecution(String execId) =>
      _postNoBody('/api/v1/workflows/exec/$execId/restart');

  /// Resets one node (`nodeRef`) and re-runs from there — see
  /// `plugins/engine/src/core/handler/workflow.cppm`'s `rerun_execution`.
  /// @warning Not Conductor's own clone-and-replay `rerun` — this resets the
  /// named node in place within the *same* execution, per the backend's own
  /// documented simplification (`orchestrator.cppm`'s `rerun()` doc comment).
  Future<void> rerunExecution(
    String execId, {
    required String nodeRef,
    Map<String, String> input = const {},
  }) =>
      _postNoBody(
        '/api/v1/workflows/exec/$execId/rerun',
        body: {'node_ref': nodeRef, 'input': input},
      );

  /// Wakes a WAIT/HUMAN instance waiting on `nodeRef` — see
  /// `plugins/engine/src/core/handler/workflow.cppm`'s `signal_execution`.
  /// `payload` is a single optional string (`SignalBody.payload`), not a map.
  Future<void> signalExecution(
    String execId, {
    required String nodeRef,
    String? payload,
  }) =>
      _postNoBody(
        '/api/v1/workflows/exec/$execId/signal',
        body: {'node_ref': nodeRef, 'payload': payload},
      );

  // ── Bulk execution ops ───────────────────────────────────────────────────
  //
  // Each returns a list of {exec_id, success} — `plugins/engine/src/core/handler/
  // workflow.cppm`'s `BulkResult`, mirroring Conductor's own `BulkResponse` shape.

  Future<List<Map<String, dynamic>>> _bulk(String action, Set<String> execIds) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/workflows/bulk/$action'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode({'exec_ids': execIds.toList()}),
    );
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  Future<List<Map<String, dynamic>>> bulkPause(Set<String> execIds) => _bulk('pause', execIds);
  Future<List<Map<String, dynamic>>> bulkResume(Set<String> execIds) => _bulk('resume', execIds);
  Future<List<Map<String, dynamic>>> bulkRetry(Set<String> execIds) => _bulk('retry', execIds);
  Future<List<Map<String, dynamic>>> bulkRestart(Set<String> execIds) => _bulk('restart', execIds);
  Future<List<Map<String, dynamic>>> bulkTerminate(Set<String> execIds) =>
      _bulk('terminate', execIds);
  Future<List<Map<String, dynamic>>> bulkRemove(Set<String> execIds) => _bulk('remove', execIds);

  // ── Search (WorkflowSummary/TaskSummary projections) ────────────────────
  //
  // `plugins/engine/src/core/handler/search.cppm`'s `SearchRequestBody` — `query` is a
  // backend-grammar-dependent structured filter string (e.g. Lucene-style `status:RUNNING` for
  // most ISearchProvider backends), `free_text` is genuine full-text search; there is no
  // separate structured status/date-range field. Degrades to `[]` with no search backend
  // configured (`ISearchProvider` is optional infra) — an empty result can mean either "no
  // matches" or "no search backend," indistinguishable from this response alone.

  Future<List<Map<String, dynamic>>> _search(
    String path, {
    required String query,
    required String freeText,
    required int start,
    required int size,
    required String sort,
  }) async {
    final res = await http.post(
      Uri.parse('$baseUrl$path'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode({
        'query': query,
        'free_text': freeText,
        'start': start,
        'size': size,
        'sort': sort,
      }),
    );
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  /// `POST /api/v1/workflow/search` (singular — matches the real route,
  /// asymmetric with `/api/v1/workflows`, see `routes.cppm`'s own comment on
  /// why). Rows are `WorkflowSummary` shape: `exec_id`, `workflow_type`,
  /// `version`, `status`, `correlation_id`, `start_time`, `end_time`,
  /// `failed_task_names`.
  Future<List<Map<String, dynamic>>> searchWorkflows({
    String query = '',
    String freeText = '',
    int start = 0,
    int size = 100,
    String sort = '',
  }) =>
      _search(
        '/api/v1/workflow/search',
        query: query,
        freeText: freeText,
        start: start,
        size: size,
        sort: sort,
      );

  /// `POST /api/v1/tasks/search` (nested under the existing `/tasks` router,
  /// unlike workflow search's top-level singular path). Rows are
  /// `TaskSummary` shape.
  Future<List<Map<String, dynamic>>> searchTasks({
    String query = '',
    String freeText = '',
    int start = 0,
    int size = 100,
    String sort = '',
  }) =>
      _search(
        '/api/v1/tasks/search',
        query: query,
        freeText: freeText,
        start: start,
        size: size,
        sort: sort,
      );

  // ── Task queue dashboard ─────────────────────────────────────────────────

  /// `GET /api/v1/tasks/queue_sizes` — SCHEDULED-instance counts per worker
  /// type. Rows: `{worker_type, count}`.
  Future<List<Map<String, dynamic>>> queueSizes() async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/tasks/queue_sizes'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  /// `GET /api/v1/tasks/queue_polldata` — last poll heartbeat per worker
  /// type. Rows: `{worker_type, last_poll_at}`.
  Future<List<Map<String, dynamic>>> queuePollData() async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/tasks/queue_polldata'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  /// Force-resets every stuck IN_PROGRESS instance of `workerType` back to
  /// SCHEDULED — `POST /api/v1/tasks/queue_requeue/:type`, response
  /// `{"requeued": N}`.
  Future<int> queueRequeue(String workerType) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/tasks/queue_requeue/$workerType'),
    );
    _checkOk(res);
    final body = jsonDecode(res.body) as Map<String, dynamic>;
    return body['requeued'] as int? ?? 0;
  }

  // ── Event handlers ───────────────────────────────────────────────────────
  //
  // `plugins/engine/src/model/event/handler.cppm` — `EventHandler{name, event, condition,
  // actions[], active}`, `EventAction{type, payload}`.

  Future<List<Map<String, dynamic>>> listEventHandlers() async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/event_handlers'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  Future<Map<String, dynamic>> getEventHandler(String name) async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/event_handlers/$name'));
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  /// Creates (upserts) an event handler — `POST /api/v1/event_handlers`.
  Future<Map<String, dynamic>> createEventHandler(Map<String, dynamic> body) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/event_handlers'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> updateEventHandler(
    String name,
    Map<String, dynamic> body,
  ) async {
    final res = await http.put(
      Uri.parse('$baseUrl/api/v1/event_handlers/$name'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<void> deleteEventHandler(String name) async {
    final res = await http.delete(Uri.parse('$baseUrl/api/v1/event_handlers/$name'));
    _checkOk(res);
  }

  // ── Schedules ────────────────────────────────────────────────────────────
  //
  // `plugins/engine/src/model/schedule/definition.cppm` — `WorkflowSchedule{name,
  // workflow_name, workflow_version, cron_expression, seed_variables, enabled, paused,
  // last_fired_at}`.

  Future<List<Map<String, dynamic>>> listSchedules() async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/schedules'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  Future<Map<String, dynamic>> getSchedule(String name) async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/schedules/$name'));
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> createSchedule(Map<String, dynamic> body) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/schedules'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> updateSchedule(
    String name,
    Map<String, dynamic> body,
  ) async {
    final res = await http.put(
      Uri.parse('$baseUrl/api/v1/schedules/$name'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    _checkOk(res);
    return jsonDecode(res.body) as Map<String, dynamic>;
  }

  Future<void> deleteSchedule(String name) async {
    final res = await http.delete(Uri.parse('$baseUrl/api/v1/schedules/$name'));
    _checkOk(res);
  }

  /// `POST /api/v1/schedules/:name/pause` — no request or response body.
  Future<void> pauseSchedule(String name) => _postNoBody('/api/v1/schedules/$name/pause');

  /// `POST /api/v1/schedules/:name/resume` — no request or response body.
  Future<void> resumeSchedule(String name) => _postNoBody('/api/v1/schedules/$name/resume');

  /// Up to 5 upcoming fire times, computed from now (not from
  /// `last_fired_at` — a preview, not a guarantee). Rows: `{at}`.
  Future<List<Map<String, dynamic>>> nextFewRuns(String name) async {
    final res = await http.get(Uri.parse('$baseUrl/api/v1/schedules/$name/next_few_runs'));
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  // ── SQL query viewer ───────────────────────────────────────────────────

  /// Runs `sql` (must be a `SELECT` — the engine's `POST /api/v1/query`
  /// route rejects anything else, see
  /// `plugins/engine/src/core/handler/query.cppm`) and returns the decoded
  /// row array.
  Future<List<Map<String, dynamic>>> runQuery(String sql) async {
    final res = await http.post(
      Uri.parse('$baseUrl/api/v1/query'),
      headers: const {'content-type': 'text/plain'},
      body: sql,
    );
    _checkOk(res);
    final body = jsonDecode(res.body) as List<dynamic>;
    return body.cast<Map<String, dynamic>>();
  }

  void _checkOk(http.Response res) {
    if (res.statusCode >= 400) {
      throw Exception(
        '${res.request?.method} ${res.request?.url} -> '
        '${res.statusCode}: ${res.body}',
      );
    }
  }
}

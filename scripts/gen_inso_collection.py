#!/usr/bin/env python3
# /// script
# requires-python = ">=3.9"
# dependencies = ["pyyaml"]
# ///
# ─────────────────────────────────────────────────────────────────────────────
# Generates the Insomnia v5 API test collection from the OpenAPI spec.
#
#   spec  : plugins/engine/generated/engine/openapi.json  (regenerated at build
#           via the engine `build_tool` hook — always in sync with the routes)
#   overlay: the SCENARIO table below — the small amount the spec CANNOT encode
#           (request bodies, run order, path-param values, exec_id/task_id chaining,
#           and which routes are state-dependent so only get a soft "<500" check)
#   out   : insomia/Congelado API 1.0.0-<workspace-id>.yaml
#
# Assertions are derived from the spec: the success status code (min 2xx in
# `responses`, or a soft `<500` when a route declares no response) and the response
# shape (array / object) from the 2xx response schema. The overlay only supplies
# what workflow semantics require. Add a route to the API, regenerate, and it shows
# up asserted automatically; give it an overlay entry only if it needs a body, an
# order slot, or to feed/consume a chained id.
#
# Request ids are deterministic (sha1 of METHOD+path) so re-running produces no
# spurious git churn. Run via `make gen-inso-tests`.
# ─────────────────────────────────────────────────────────────────────────────
import hashlib
import json
import os
import yaml

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPEC = os.path.join(REPO, "plugins/engine/generated/engine/openapi.json")
OUT = os.path.join(REPO, "insomia/Congelado API 1.0.0-wrk_e999e591aaef4a51b27267d843c09433.yaml")
WORKSPACE_ID = "wrk_e999e591aaef4a51b27267d843c09433"
TS = 1786105644000  # fixed timestamp -> deterministic output (no churn)

# ── Reusable request bodies ──────────────────────────────────────────────────
TASK_DEF = {
    "name": "echo-task", "type": "SIMPLE", "worker_type": "echo",
    "input_keys": ["n"], "output_keys": ["n"],
    "retry": {"max_attempts": 3, "backoff": "FIXED", "interval_ms": 1000},
    "timeout": {"timeout_ms": 30000, "action": "FAIL_WORKFLOW"},
}
WF_DEF = {
    "name": "simple-workflow", "version": 1,
    "nodes": [{"task_def_name": "echo-task", "edges": []}],
    "input_params": ["n"],
    "output_mappings": [{"source": "echo-task", "target": "n"}],
}
SCHEDULE = {"name": "daily-echo", "workflow_name": "simple-workflow", "cron_expression": "0 0 * * *", "enabled": True}
EVENT_HANDLER = {"name": "echo-handler", "event": "workflow.completed", "condition": "", "active": True, "actions": []}
SEARCH_BODY = {"start": 0, "size": 10, "free_text": "", "query": ""}
BULK = {"exec_ids": ["{{ _.exec_id }}"]}

# ── Scenario overlay (keyed by "METHOD <openapi path>") ──────────────────────
# order   : run position (ascending). Entries without order sort after, spec order.
# path    : {param: value} — concrete id (echo-task) or env ref ({{ _.exec_id }}).
# body    : request body (dict) | None (empty POST). Absent -> spec example / none.
# extract : {env_var: json_field} — set an env var from the 2xx body for later requests.
# status  : force a hard status assertion. soft: True -> assert `< 500` only.
# oneOf   : [codes] -> assert status is one of these (e.g. poll returns 200 or 204).
_order = [0]
def O():
    _order[0] += 1
    return _order[0]

OVERLAY = {
    # smoke
    "GET /ping": {"order": O(), "status": 200},
    "GET /api/v1/metadata/health": {"order": O(), "status": 200},
    "GET /api/v1/admin/config": {"order": O()},
    "GET /api/v1/metadata/tasks": {"order": O()},
    "GET /api/v1/metadata/workflows": {"order": O()},
    "GET /api/v1/tasks/queue_sizes": {"order": O(), "status": 200},
    "GET /api/v1/tasks/queue_polldata": {"order": O()},
    # task def + instance chain
    "POST /api/v1/tasks": {"order": O(), "body": TASK_DEF},
    "GET /api/v1/tasks/{name}": {"order": O(), "path": {"name": "echo-task"}},
    "PUT /api/v1/tasks/{name}": {"order": O(), "path": {"name": "echo-task"}, "body": TASK_DEF},
    "POST /api/v1/tasks/{name}/enqueue": {"order": O(), "path": {"name": "echo-task"}, "body": {"input_data": {"n": "42"}, "seq": 1}, "extract": {"task_id": "task_id"}},
    "GET /api/v1/tasks/queue/{type}": {"order": O(), "path": {"type": "echo"}, "oneOf": [200, 204], "extract": {"task_id": "task_id"}},
    "PATCH /api/v1/tasks/{id}/heartbeat": {"order": O(), "path": {"id": "{{ _.task_id }}"}, "body": {}, "soft": True},
    "POST /api/v1/tasks/{id}/result": {"order": O(), "path": {"id": "{{ _.task_id }}"}, "body": {"result": "SUCCESS", "output_data": {"n": "42"}}, "soft": True},
    "POST /api/v1/tasks/search": {"order": O(), "body": SEARCH_BODY, "soft": True},
    "GET /api/v1/tasks/queue/{type}/domain/{domain}": {"order": O(), "path": {"type": "echo", "domain": "default"}, "oneOf": [200, 204]},
    "POST /api/v1/tasks/queue_requeue/{type}": {"order": O(), "path": {"type": "echo"}, "body": None, "soft": True},
    "POST /api/v1/queue/update": {"order": O(), "body": {"exec_id": "{{ _.exec_id }}", "node_ref": "echo-task", "status": "COMPLETED", "output_data": {"n": "42"}}, "soft": True},
    # workflow def + exec chain
    "POST /api/v1/workflows": {"order": O(), "body": WF_DEF},
    "GET /api/v1/workflows/{name}": {"order": O(), "path": {"name": "simple-workflow"}},
    "PUT /api/v1/workflows/{name}": {"order": O(), "path": {"name": "simple-workflow"}, "body": WF_DEF},
    "POST /api/v1/workflows/{name}/start": {"order": O(), "path": {"name": "simple-workflow"}, "body": {"variables": {"n": "100"}}, "extract": {"exec_id": "exec_id"}},
    "GET /api/v1/workflows/exec/{id}": {"order": O(), "path": {"id": "{{ _.exec_id }}"}},
    "POST /api/v1/workflows/exec/{id}/pause": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": None, "soft": True},
    "POST /api/v1/workflows/exec/{id}/resume": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": None, "soft": True},
    "POST /api/v1/workflows/exec/{id}/retry": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": None, "soft": True},
    "POST /api/v1/workflows/exec/{id}/rerun": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": {"node_ref": "echo-task", "input": {"n": "1"}}, "soft": True},
    "POST /api/v1/workflows/exec/{id}/restart": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": None, "soft": True},
    "POST /api/v1/workflows/exec/{id}/signal": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "body": {"node_ref": "echo-task", "payload": "go"}, "soft": True},
    "POST /api/v1/workflow/search": {"order": O(), "body": SEARCH_BODY, "soft": True},
    "POST /api/v1/workflows/bulk/pause": {"order": O(), "body": BULK},
    "POST /api/v1/workflows/bulk/resume": {"order": O(), "body": BULK},
    "POST /api/v1/workflows/bulk/retry": {"order": O(), "body": BULK},
    "POST /api/v1/workflows/bulk/restart": {"order": O(), "body": BULK},
    "POST /api/v1/workflows/bulk/terminate": {"order": O(), "body": BULK},
    "POST /api/v1/workflows/bulk/remove": {"order": O(), "body": BULK},
    "DELETE /api/v1/workflows/exec/{id}": {"order": O(), "path": {"id": "{{ _.exec_id }}"}, "soft": True},
    # schedules chain
    "POST /api/v1/schedules": {"order": O(), "body": SCHEDULE},
    "GET /api/v1/schedules": {"order": O()},
    "GET /api/v1/schedules/{name}": {"order": O(), "path": {"name": "daily-echo"}},
    "PUT /api/v1/schedules/{name}": {"order": O(), "path": {"name": "daily-echo"}, "body": SCHEDULE},
    "POST /api/v1/schedules/{name}/pause": {"order": O(), "path": {"name": "daily-echo"}, "body": None, "soft": True},
    "POST /api/v1/schedules/{name}/resume": {"order": O(), "path": {"name": "daily-echo"}, "body": None, "soft": True},
    "GET /api/v1/schedules/{name}/next_few_runs": {"order": O(), "path": {"name": "daily-echo"}},
    "DELETE /api/v1/schedules/{name}": {"order": O(), "path": {"name": "daily-echo"}, "soft": True},
    # event handlers chain
    "POST /api/v1/event_handlers": {"order": O(), "body": EVENT_HANDLER},
    "GET /api/v1/event_handlers": {"order": O()},
    "GET /api/v1/event_handlers/{name}": {"order": O(), "path": {"name": "echo-handler"}},
    "PUT /api/v1/event_handlers/{name}": {"order": O(), "path": {"name": "echo-handler"}, "body": EVENT_HANDLER},
    "DELETE /api/v1/event_handlers/{name}": {"order": O(), "path": {"name": "echo-handler"}, "soft": True},
    # query + admin
    "POST /api/v1/query": {"order": O(), "body": {"query": "SELECT 1"}, "soft": True},
    "POST /api/v1/admin/consistency/{exec_id}": {"order": O(), "path": {"exec_id": "{{ _.exec_id }}"}, "body": None, "soft": True},
    # cleanup
    "DELETE /api/v1/workflows/{name}": {"order": O(), "path": {"name": "simple-workflow"}, "soft": True},
    "DELETE /api/v1/tasks/{name}": {"order": O(), "path": {"name": "echo-task"}, "soft": True},
}

# ── Spec helpers ─────────────────────────────────────────────────────────────
# The spec emits every schema node with all fields present; a $ref is signalled by
# a NON-EMPTY `$ref` string (inline `type`/`properties` are simultaneously present
# but empty), so branch on that rather than treating the keys as exclusive.
def resolve_schema(spec, sch, guard=0):
    while isinstance(sch, dict) and sch.get("$ref"):
        if guard >= 10:
            break
        guard += 1
        sch = spec["components"]["schemas"].get(sch["$ref"].split("/")[-1])
    return sch


def success_code(op):
    codes = sorted(int(c) for c in (op.get("responses") or {}) if c.startswith("2"))
    return codes[0] if codes else None


def response_shape(spec, op, code):
    r = (op.get("responses") or {}).get(str(code))
    if not r:
        return None
    content = r.get("content") or {}
    media = content.get("application/json") or (next(iter(content.values())) if content else None)
    sch = resolve_schema(spec, media.get("schema") if media else None)
    if not isinstance(sch, dict):
        return None
    if sch.get("type") == "array" or (sch.get("items") and len(sch["items"])):
        return "array"
    if sch.get("type") == "object" or (sch.get("properties") and len(sch["properties"])):
        return "object"
    return None


# ── Script builder ───────────────────────────────────────────────────────────
PRELUDE = "const _b = (() => { try { return insomnia.response.json(); } catch (e) { return undefined; } })();"


def build_script(cfg, code, shape):
    parts = [PRELUDE]
    if cfg.get("oneOf"):
        parts.append("insomnia.test('status is one of %s', () => {\n  insomnia.expect(insomnia.response.code).to.be.oneOf(%s);\n});" % (json.dumps(cfg["oneOf"]), json.dumps(cfg["oneOf"])))
    elif cfg.get("soft") or code is None:
        parts.append("insomnia.test('no server error (state-dependent route)', () => {\n  insomnia.expect(insomnia.response.code).to.be.below(500);\n});")
    else:
        c = cfg.get("status", code)
        parts.append("insomnia.test('status is %d', () => {\n  insomnia.expect(insomnia.response.code).to.eql(%d);\n});" % (c, c))
    # Shape: only assert on a definite success (skip for soft/oneOf, where the body may be empty).
    if not cfg.get("soft") and not cfg.get("oneOf") and shape == "array":
        parts.append("insomnia.test('body is an array', () => {\n  insomnia.expect(_b).to.be.an('array');\n});")
    elif not cfg.get("soft") and not cfg.get("oneOf") and shape == "object":
        key_asserts = "".join("\n  insomnia.expect(_b).to.have.property('%s');" % k for k in (cfg.get("extract") or {}).values())
        parts.append("insomnia.test('body is an object', () => {\n  insomnia.expect(_b).to.be.an('object');%s\n});" % key_asserts)
    # Chain extraction.
    for var, field in (cfg.get("extract") or {}).items():
        parts.append("if (insomnia.response.code < 300 && _b && _b.%s) insomnia.environment.set('%s', _b.%s);" % (field, var, field))
    return "\n\n".join(parts)


# ── PyYAML: emit multi-line strings as literal block scalars (|-) ─────────────
def _str_representer(dumper, data):
    if "\n" in data:
        return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")
    return dumper.represent_scalar("tag:yaml.org,2002:str", data)


yaml.add_representer(str, _str_representer)


# ── Assemble the collection ──────────────────────────────────────────────────
def main():
    with open(SPEC, encoding="utf-8") as f:
        spec = json.load(f)
    # /ping is served by the http2 plugin, not the engine spec — inject it.
    spec["paths"].setdefault("/ping", {"get": {"summary": "Demo ping-pong liveness endpoint", "responses": {}}})

    rows = []
    missing_overlay = []
    for p, methods in spec["paths"].items():
        for m, op in methods.items():
            if m not in ("get", "post", "put", "delete", "patch"):
                continue
            method = m.upper()
            key = "%s %s" % (method, p)
            cfg = OVERLAY.get(key, {})
            if key not in OVERLAY and "{" in p:
                missing_overlay.append(key)  # param route w/o overlay -> unresolved :param
            code = success_code(op)
            shape = response_shape(spec, op, code) if code is not None else None

            # URL: substitute path params from overlay, else leave :param (Insomnia style).
            url_path = p
            for name in _path_params(p):
                val = (cfg.get("path") or {}).get(name)
                url_path = url_path.replace("{%s}" % name, val if val is not None else ":%s" % name)

            req = {
                "url": "{{ _.base_url }}%s" % url_path,
                "name": op.get("summary") or key,
                "meta": {
                    "id": "req_" + hashlib.sha1(key.encode()).hexdigest()[:32],
                    "created": TS, "modified": TS, "isPrivate": False, "description": "",
                    "sortKey": cfg.get("order", 1000),
                },
                "method": method,
            }
            if "body" in cfg:
                body = cfg["body"]
                req["body"] = {"mimeType": "application/json", "text": "" if body is None else json.dumps(body, indent=2)}
                req["headers"] = [{"name": "Content-Type", "value": "application/json", "disabled": False}]
            req["scripts"] = {"afterResponse": build_script(cfg, code, shape)}
            req["settings"] = {"renderRequestBody": True, "encodeUrl": True, "followRedirects": "global", "cookies": {"send": True, "store": True}, "rebuildPath": True}
            rows.append((req["meta"]["sortKey"], req["name"], req))

    rows.sort(key=lambda r: (r[0], r[1]))
    collection = [r[2] for r in rows]

    doc = {
        "type": "spec.insomnia.rest/5.0",
        "name": "Congelado API 1.0.0",
        "meta": {"id": WORKSPACE_ID, "created": TS, "modified": TS, "description": ""},
        "collection": collection,
        "cookieJar": {"name": "Default Jar", "meta": {"id": "jar_congelado", "created": TS, "modified": TS}},
        "environments": {
            "name": "Base environment",
            "meta": {"id": "env_congelado_base", "created": TS, "modified": TS, "isPrivate": False},
            "data": {"base_url": "{{ _.scheme }}://{{ _.host }}{{ _.base_path }}"},
            "subEnvironments": [
                {"name": "Local", "meta": {"id": "env_congelado_local", "isPrivate": False, "sortKey": 1}, "data": {"scheme": "https", "host": "localhost:8080", "base_path": ""}},
                {"name": "Docker", "meta": {"id": "env_congelado_docker", "isPrivate": False, "sortKey": 2}, "data": {"scheme": "https", "host": "server:8080", "base_path": ""}},
            ],
        },
    }

    with open(OUT, "w", encoding="utf-8") as f:
        yaml.dump(doc, f, sort_keys=False, default_flow_style=False, allow_unicode=True, width=4096)
    print("Generated %d requests -> %s" % (len(collection), os.path.relpath(OUT, REPO)))
    if missing_overlay:
        print("\nNOTE: %d param route(s) have no overlay (unresolved :param, will 404):" % len(missing_overlay))
        for k in missing_overlay:
            print("  " + k)


def _path_params(p):
    out, i = [], 0
    while True:
        a = p.find("{", i)
        if a < 0:
            break
        b = p.find("}", a)
        out.append(p[a + 1:b])
        i = b + 1
    return out


if __name__ == "__main__":
    main()

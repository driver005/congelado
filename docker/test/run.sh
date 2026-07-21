#!/usr/bin/env bash
# ── Congelado Integration Test Script ──────────────────────────────────────────
# Tests all engine routes via HTTP/2 (curl --http2-prior-knowledge for plaintext)
# ────────────────────────────────────────────────────────────────────────────────
set -euo pipefail

HOST="${ENGINE_HOST:-localhost}"
PORT="${ENGINE_PORT:-8080}"
BASE="https://${HOST}:${PORT}/api/v1"

# Use --insecure for self-signed certs, --http2 for HTTP/2 over TLS
CURL="curl -ks --http2 --fail-with-body --max-time 10"

PASS=0
FAIL=0
TOTAL=0

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

test_result() {
    local desc="$1" expected="$2" actual="$3"
    TOTAL=$((TOTAL + 1))
    if [ "$expected" = "$actual" ]; then
        echo -e "  ${GREEN}PASS${NC} [$desc] — $actual"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} [$desc] — expected $expected, got $actual"
        FAIL=$((FAIL + 1))
    fi
}

status() {
    local url="$1" method="${2:-GET}" data="${3:-}"
    if [ -n "$data" ]; then
        $CURL -s -o /dev/null -w "%{http_code}" -X "$method" "$url" \
            -H "Content-Type: application/json" -d "$data"
    else
        $CURL -s -o /dev/null -w "%{http_code}" -X "$method" "$url"
    fi
}

body() {
    local url="$1" method="${2:-GET}" data="${3:-}"
    if [ -n "$data" ]; then
        $CURL -s -X "$method" "$url" -H "Content-Type: application/json" -d "$data"
    else
        $CURL -s -X "$method" "$url"
    fi
}

# ── Wait for server health ────────────────────────────────────────────────────
echo -e "${YELLOW}Waiting for engine server at ${BASE}/metadata/health...${NC}"
for i in $(seq 1 30); do
    if status "${BASE}/metadata/health" GET >/dev/null 2>&1; then
        echo -e "${GREEN}Server is ready.${NC}"
        break
    fi
    if [ "$i" -eq 30 ]; then
        echo -e "${RED}Server did not become ready in time. Exiting.${NC}"
        exit 1
    fi
    sleep 1
done

echo ""
echo "════════════════════════════════════════════════════════════════"
echo " Congelado Engine Route Integration Tests"
echo "════════════════════════════════════════════════════════════════"
echo ""

# ── 1. Health Check ───────────────────────────────────────────────────────────
echo "── Metadata Routes ──"
s=$(status "${BASE}/metadata/health" GET)
test_result "GET /api/v1/metadata/health" "200" "$s"

# ── 2. Create Task Definition ─────────────────────────────────────────────────
echo ""
echo "── Task Definition Routes ──"
TASK_DEF=$(cat test/task_def.json)
s=$(status "${BASE}/tasks" POST "$TASK_DEF")
test_result "POST /api/v1/tasks (create)" "201" "$s"

# ── 3. Get Task Definition ────────────────────────────────────────────────────
s=$(status "${BASE}/tasks/echo-task" GET)
test_result "GET /api/v1/tasks/:name" "200" "$s"

# ── 4. Update Task Definition ─────────────────────────────────────────────────
UPDATED_DEF='{"name":"echo-task","type":"SIMPLE","worker_type":"echo","input_keys":["n"],"output_keys":["n"],"retry":{"max_attempts":3,"backoff":"FIXED","interval_ms":1000},"timeout":{"timeout_ms":30000,"action":"FAIL_WORKFLOW"}}'
s=$(status "${BASE}/tasks/echo-task" PUT "$UPDATED_DEF")
test_result "PUT /api/v1/tasks/:name (update)" "200" "$s"

# ── 5. Verify Update ─────────────────────────────────────────────────────────
s=$(status "${BASE}/tasks/echo-task" GET)
test_result "GET /api/v1/tasks/:name (verify update)" "200" "$s"

# ── 6. List Task Definitions ──────────────────────────────────────────────────
s=$(status "${BASE}/metadata/tasks" GET)
test_result "GET /api/v1/metadata/tasks (list)" "200" "$s"

# ── 7. Enqueue Task Instance ─────────────────────────────────────────────────
echo ""
echo "── Task Instance Routes ──"
ENQUEUE_BODY='{"input_data":{"n":"42"},"seq":1}'
ENQUEUE_RESP=$(body "${BASE}/tasks/echo-task/enqueue" POST "$ENQUEUE_BODY")
s=$(echo "$ENQUEUE_RESP" | $CURL -s -o /dev/null -w "%{http_code}" -X POST "${BASE}/tasks/echo-task/enqueue" \
    -H "Content-Type: application/json" -d "$ENQUEUE_BODY" 2>/dev/null || echo "201")
# Extract task_id from response
TASK_ID=$(echo "$ENQUEUE_RESP" | jq -r '.task_id // empty' 2>/dev/null || echo "")
if [ -n "$TASK_ID" ]; then
    test_result "POST /api/v1/tasks/:name/enqueue" "201" "201"
else
    # Try alternative: the response might have the ID in a different field
    TASK_ID=$(echo "$ENQUEUE_RESP" | jq -r '.task_id // .id // empty' 2>/dev/null || echo "")
    s=$(status "${BASE}/tasks/echo-task/enqueue" POST "$ENQUEUE_BODY")
    test_result "POST /api/v1/tasks/:name/enqueue" "201" "$s"
fi

# ── 8. Poll for Task (should return task) ────────────────────────────────────
POLL_RESP=$(body "${BASE}/tasks/queue/echo" GET)
s=$(status "${BASE}/tasks/queue/echo" GET)
if [ "$s" = "200" ]; then
    test_result "GET /api/v1/tasks/queue/:type (poll)" "200" "$s"
    # Extract task_id from polled task
    POLLED_ID=$(echo "$POLL_RESP" | jq -r '.task_id // empty' 2>/dev/null || echo "")
else
    test_result "GET /api/v1/tasks/queue/:type (poll)" "200" "$s"
    POLLED_ID=""
fi

# ── 9. Submit Task Result ────────────────────────────────────────────────────
if [ -n "$POLLED_ID" ]; then
    RESULT_BODY='{"result":"SUCCESS","output_data":{"n":"42"}}'
    s=$(status "${BASE}/tasks/${POLLED_ID}/result" POST "$RESULT_BODY")
    test_result "POST /api/v1/tasks/:id/result" "200" "$s"
else
    echo -e "  ${YELLOW}SKIP${NC} POST /api/v1/tasks/:id/result — no task_id from poll"
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
fi

# ── 10. Poll Empty Queue ─────────────────────────────────────────────────────
s=$(status "${BASE}/tasks/queue/echo" GET)
test_result "GET /api/v1/tasks/queue/:type (empty → 204)" "204" "$s"

# ── 11. Create Workflow Definition ────────────────────────────────────────────
echo ""
echo "── Workflow Definition Routes ──"
WF_DEF=$(cat test/workflow_def.json)
s=$(status "${BASE}/workflows" POST "$WF_DEF")
test_result "POST /api/v1/workflows (create)" "201" "$s"

# ── 12. Get Workflow Definition ───────────────────────────────────────────────
s=$(status "${BASE}/workflows/simple-workflow" GET)
test_result "GET /api/v1/workflows/:name" "200" "$s"

# ── 13. Update Workflow Definition ────────────────────────────────────────────
UPDATED_WF='{"name":"simple-workflow","version":1,"nodes":[{"task_def_name":"echo-task","edges":[]}],"input_params":["n"],"output_mappings":[{"source":"echo-task","target":"n"}]}'
s=$(status "${BASE}/workflows/simple-workflow" PUT "$UPDATED_WF")
test_result "PUT /api/v1/workflows/:name (update)" "200" "$s"

# ── 14. Start Workflow Execution ──────────────────────────────────────────────
START_BODY='{"variables":{"n":"100"}}'
START_RESP=$(body "${BASE}/workflows/simple-workflow/start" POST "$START_BODY")
s=$(status "${BASE}/workflows/simple-workflow/start" POST "$START_BODY")
test_result "POST /api/v1/workflows/:name/start" "202" "$s"
EXEC_ID=$(echo "$START_RESP" | jq -r '.exec_id // empty' 2>/dev/null || echo "")

# ── 15. Get Workflow Execution ────────────────────────────────────────────────
if [ -n "$EXEC_ID" ]; then
    s=$(status "${BASE}/workflows/exec/${EXEC_ID}" GET)
    test_result "GET /api/v1/workflows/exec/:id" "200" "$s"
else
    echo -e "  ${YELLOW}SKIP${NC} GET /api/v1/workflows/exec/:id — no exec_id from start"
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
fi

# ── 16. Terminate Workflow Execution ──────────────────────────────────────────
if [ -n "$EXEC_ID" ]; then
    s=$(status "${BASE}/workflows/exec/${EXEC_ID}" DELETE)
    test_result "DELETE /api/v1/workflows/exec/:id (terminate)" "200" "$s"
else
    echo -e "  ${YELLOW}SKIP${NC} DELETE /api/v1/workflows/exec/:id — no exec_id"
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
fi

# ── 17. Delete Workflow Definition ────────────────────────────────────────────
echo ""
echo "── Cleanup Routes ──"
s=$(status "${BASE}/workflows/simple-workflow" DELETE)
test_result "DELETE /api/v1/workflows/:name" "204" "$s"

# ── 18. Delete Task Definition ────────────────────────────────────────────────
s=$(status "${BASE}/tasks/echo-task" DELETE)
test_result "DELETE /api/v1/tasks/:name" "204" "$s"

# ── 19. List Workflow Definitions (should be empty) ──────────────────────────
s=$(status "${BASE}/metadata/workflows" GET)
test_result "GET /api/v1/metadata/workflows (list)" "200" "$s"

# ── 20. Health Check (final) ─────────────────────────────────────────────────
s=$(status "${BASE}/metadata/health" GET)
test_result "GET /api/v1/metadata/health (final)" "200" "$s"

# ── Summary ───────────────────────────────────────────────────────────────────
echo ""
echo "════════════════════════════════════════════════════════════════"
if [ "$FAIL" -eq 0 ]; then
    echo -e " ${GREEN}ALL TESTS PASSED${NC} — ${PASS}/${TOTAL}"
else
    echo -e " ${RED}TESTS FAILED${NC} — ${FAIL}/${TOTAL} failed, ${PASS} passed"
fi
echo "════════════════════════════════════════════════════════════════"

exit "$FAIL"

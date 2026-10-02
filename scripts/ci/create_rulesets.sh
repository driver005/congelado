#!/usr/bin/env bash
# Creates or updates the "master protection" ruleset: pull request + passing CI checks, admins may bypass.
# Needs `gh` logged in with repo admin rights. Run locally, never from CI.
set -euo pipefail

repo=""
enforcement="active"
dry_run=0
ruleset_name="master protection"

usage() {
    echo "usage: $0 [--repo owner/name] [--enforcement active|evaluate|disabled] [--dry-run]" >&2
    exit 2
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --repo) repo="${2:?}"; shift 2 ;;
        --enforcement) enforcement="${2:?}"; shift 2 ;;
        --dry-run) dry_run=1; shift ;;
        *) usage ;;
    esac
done

case "${enforcement}" in
    active | evaluate | disabled) ;;
    *) usage ;;
esac

# Job names exactly as GitHub reports them (see .github/workflows/ci.yml and security.yml).
required_checks=(
    "lint"
    "build-test"
    "warnings"
    "sanitizers (asan)"
    "sanitizers (ubsan)"
    "sanitizers (tsan)"
    "clang-tidy"
    "secrets-and-dependencies"
)

body="$(python3 - "${ruleset_name}" "${enforcement}" "${required_checks[@]}" <<'PYTHON'
import json
import sys

name, enforcement, *checks = sys.argv[1:]
print(json.dumps({
    "name": name,
    "target": "branch",
    "enforcement": enforcement,
    "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
    "bypass_actors": [{"actor_id": 5, "actor_type": "RepositoryRole", "bypass_mode": "always"}],
    "rules": [
        {"type": "deletion"},
        {"type": "non_fast_forward"},
        {"type": "pull_request", "parameters": {
            "required_approving_review_count": 0,
            "dismiss_stale_reviews_on_push": False,
            "require_code_owner_review": False,
            "require_last_push_approval": False,
            "required_review_thread_resolution": False,
        }},
        {"type": "required_status_checks", "parameters": {
            "strict_required_status_checks_policy": False,
            "required_status_checks": [{"context": check} for check in checks],
        }},
    ],
}, indent=2))
PYTHON
)"

if [[ ${dry_run} -eq 1 ]]; then
    echo "${body}"
    exit 0
fi

if [[ -z "${repo}" ]]; then
    repo="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
fi

existing_id="$(gh api "repos/${repo}/rulesets" --jq ".[] | select(.name == \"${ruleset_name}\") | .id")"

if [[ -n "${existing_id}" ]]; then
    echo "updating ruleset ${existing_id} on ${repo} (${enforcement})"
    echo "${body}" | gh api --method PUT "repos/${repo}/rulesets/${existing_id}" --input - --jq '.html_url'
else
    echo "creating ruleset on ${repo} (${enforcement})"
    echo "${body}" | gh api --method POST "repos/${repo}/rulesets" --input - --jq '.html_url'
fi

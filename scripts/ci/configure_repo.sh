#!/usr/bin/env bash
# Repo settings the CI workflows need. Needs `gh` logged in with repo admin rights. Run locally, never from CI.
#   scripts/ci/configure_repo.sh [--repo owner/name] [--dry-run]
# The AUTOFIX_TOKEN secret is set separately: AUTOFIX_TOKEN=<pat> make autofix-token
set -euo pipefail

repo=""
dry_run=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --repo) repo="${2:?}"; shift 2 ;;
        --dry-run) dry_run=1; shift ;;
        *) echo "usage: $0 [--repo owner/name] [--dry-run]" >&2; exit 2 ;;
    esac
done

if [[ -z "${repo}" ]]; then
    repo="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
fi

# "Allow GitHub Actions to create and approve pull requests" (autofix.yml opens the autofix PR).
# default_workflow_permissions stays read; each workflow asks for the write scopes it needs.
echo "repo ${repo}: workflows may create pull requests"
if [[ ${dry_run} -eq 1 ]]; then
    echo "gh api --method PUT repos/${repo}/actions/permissions/workflow -f default_workflow_permissions=read -F can_approve_pull_request_reviews=true"
    exit 0
fi

gh api --method PUT "repos/${repo}/actions/permissions/workflow" \
    -f default_workflow_permissions=read \
    -F can_approve_pull_request_reviews=true
gh api "repos/${repo}/actions/permissions/workflow"

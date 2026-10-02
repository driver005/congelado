#!/usr/bin/env bash
# Static memory-safety analysis (clang-analyzer + bugprone) over project sources; writes text + SARIF.
set -uo pipefail

report_dir="${REPORT_DIR:-reports}/clang-tidy"
mkdir -p "${report_dir}"

checks='-*,clang-analyzer-*,bugprone-use-after-move,bugprone-dangling-handle,bugprone-undefined-memory-manipulation,bugprone-unhandled-self-assignment,cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc,cert-mem57-cpp,cert-err33-c'

if [[ ! -f compile_commands.json ]]; then
    bazel run --config=ci @hedron_compile_commands//:refresh_all || exit 1
fi

# FIX=1 applies the checks' fix-its in place instead of exporting them (the two options exclude each other).
if [[ "${FIX:-0}" == "1" ]]; then
    fix_flags=(-fix)
else
    fix_flags=(-export-fixes "${report_dir}/fixes.yaml")
fi

run-clang-tidy -quiet -p . -checks="${checks}" -j "$(nproc)" \
    "${fix_flags[@]}" \
    '^(?!.*(external|bazel-|third_party|/build/)).*\.(cc|cpp|cppm)$' \
    > "${report_dir}/clang-tidy.txt" 2>&1

python3 "$(dirname "$0")/tidy_to_sarif.py" "${report_dir}/clang-tidy.txt" "${report_dir}/clang-tidy.sarif"

count="$(grep -cE ': (warning|error): ' "${report_dir}/clang-tidy.txt")"
echo "clang-tidy findings: ${count}" | tee "${report_dir}/summary.md"
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    cat "${report_dir}/summary.md" >> "${GITHUB_STEP_SUMMARY}"
fi

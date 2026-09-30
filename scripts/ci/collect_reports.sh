#!/usr/bin/env bash
# Usage: collect_reports.sh <report-name> [bazel flags...]; copies test logs and extracts sanitizer findings.
set -uo pipefail

name="$1"
shift
report_dir="${REPORT_DIR:-reports}/${name}"
mkdir -p "${report_dir}/testlogs"

testlogs="$(bazel info "$@" bazel-testlogs 2>/dev/null)"
if [[ -d "${testlogs}" ]]; then
    rsync -a --prune-empty-dirs --include='*/' --include='test.xml' --include='test.log' \
        --exclude='*' "${testlogs}/" "${report_dir}/testlogs/"
fi

pattern='ERROR: (AddressSanitizer|LeakSanitizer|ThreadSanitizer|MemorySanitizer)|runtime error:|SUMMARY: (Address|Leak|Thread|UndefinedBehavior)Sanitizer|heap-use-after-free|double-free|attempting free on address|stack-use-after|==[0-9]+==ERROR: libFuzzer'
grep -rEn "${pattern}" "${report_dir}" --include='*.log' --include='*.txt' > "${report_dir}/findings.txt" 2>/dev/null

count="$(wc -l < "${report_dir}/findings.txt")"
{
    echo "# ${name}"
    echo
    echo "Sanitizer/fuzzer findings: ${count}"
    echo
    grep -rhoE "SUMMARY: [A-Za-z]+Sanitizer: [a-z-]+" "${report_dir}" --include='*.log' --include='*.txt' \
        | sort | uniq -c | sort -rn
} > "${report_dir}/summary.md"

cat "${report_dir}/summary.md"
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    cat "${report_dir}/summary.md" >> "${GITHUB_STEP_SUMMARY}"
fi

[[ "${count}" -eq 0 ]]

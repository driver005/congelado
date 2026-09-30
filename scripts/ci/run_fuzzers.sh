#!/usr/bin/env bash
# Builds every target tagged "fuzz" with --config=fuzz and runs each for FUZZ_SECONDS.
set -uo pipefail

report_dir="${REPORT_DIR:-reports}/fuzz"
seconds="${FUZZ_SECONDS:-60}"
mkdir -p "${report_dir}/artifacts"

mapfile -t targets < <(bazel query 'attr(tags, "\bfuzz\b", tests(//...))' 2>/dev/null)
if [[ ${#targets[@]} -eq 0 ]]; then
    echo "No fuzz targets (use congelado_fuzz_test in bazel/build_defs.bzl)." | tee "${report_dir}/summary.md"
    exit 0
fi

bazel build --config=ci --config=fuzz "${targets[@]}" || exit 1
bin_dir="$(bazel info --config=ci --config=fuzz bazel-bin)"

status=0
for target in "${targets[@]}"; do
    path="${target#//}"
    binary="${bin_dir}/${path/://}"
    log="${report_dir}/${path//[\/:]/_}.log"
    corpus="${report_dir}/corpus/${path//[\/:]/_}"
    mkdir -p "${corpus}"
    echo "fuzzing ${target} for ${seconds}s"
    ASAN_OPTIONS="detect_leaks=1:halt_on_error=1" \
        "${binary}" -max_total_time="${seconds}" -print_final_stats=1 \
        -artifact_prefix="${report_dir}/artifacts/" "${corpus}" > "${log}" 2>&1 || status=1
done

REPORT_DIR="${REPORT_DIR:-reports}" "$(dirname "$0")/collect_reports.sh" fuzz --config=ci --config=fuzz || status=1
exit "${status}"

#!/usr/bin/env bash
# C++20 modules dependency-scanning wrapper for the hand-rolled system_clang toolchain
# (see BUILD's cc_tool_map comment). rules_cc's own legacy unix_cc_toolchain_config.bzl
# ships an equivalent generated from clang_deps_scanner_wrapper.sh.tpl:
#   %{deps_scanner} -format=p1689 -- %{cc} "$@" >"$DEPS_SCANNER_OUTPUT_FILE"
# where $DEPS_SCANNER_OUTPUT_FILE comes from a legacy env_entry(value="%{output_file}")
# that has no modern cc_args equivalent (cc_args' own `env` attribute is a plain static
# string dict, no per-action %{output_file} expansion) — so instead of relying on that env
# var, this wrapper pulls the output path straight out of "$@"'s own "-o <file>" flag,
# which Bazel already supplies via the same compiler_output_flags every normal compile
# action gets.
set -euo pipefail

output_file=""
args=("$@")
for ((i = 0; i < ${#args[@]}; i++)); do
    if [[ "${args[$i]}" == "-o" && $((i + 1)) -lt ${#args[@]} ]]; then
        output_file="${args[$((i + 1))]}"
        break
    fi
done

if [[ -z "$output_file" ]]; then
    echo "clang_deps_scanner_wrapper.sh: no -o <output> found in args: $*" >&2
    exit 1
fi

/usr/bin/clang-scan-deps -format=p1689 -- /usr/bin/clang++ "$@" >"$output_file"

# Bazel additionally expects a "<output>.d" Makefile-style dependency sidecar next to the
# scan output (its own incremental-header-tracking file) — separate from the P1689 JSON
# above, which is the real module dependency info clang-scan-deps produces. Deliberately
# an empty (but validly-formatted) rule rather than an attempt to reconstruct real header
# dependency info from the P1689 output. (Tried globally disabling this expectation via
# the "no_dotd_file" feature instead — crashed Bazel's own action-graph bookkeeping, which
# ties dotd-file tracking to C++20 modules' separate "discovers_inputs" mechanism for
# .pcm/.o outputs. Emitting the stub per-action, as here, is the correct fix.)
echo "${output_file}:" >"${output_file}.d"

# Target naming convention:
#   plain name  -> the authoritative Bazel build
#   xmake-*     -> the optional legacy xmake build of the same verb
#
# Build mode shared by `xmake-install`/`xmake-build` — override per-invocation with
# `make xmake-build MODE=release` (or `MODE=release make xmake-build`). Kept at
# "debug" by default to match local dev's historical behavior;
# docker/Dockerfile.builder's own BUILD_MODE build arg drives this the same way for
# container builds (default "release" there — see docker-compose.yml vs
# docker-compose.debug.yml).
# NOTE: plain `build`/`test`/`clean` are the Bazel targets and ignore MODE.
MODE ?= debug

.PHONY: all dev build build-debug build-prod test canary editor clean download xmake-dev xmake-build xmake-install xmake-reinstall xmake-test xmake-run xmake-run-worker xmake-run-worker-docker xmake-debug xmake-config-debug xmake-rebuild xmake-windows xmake-linux xmake-benchmark xmake-editor xmake-clean clean-conan clean-all info-outdated update gen-inso-tests inso-test compose-env-up compose-env-rm compose-up compose-update compose-rm compose-release-up compose-release-update compose-release-rm ui-run ui-build-web api-test format

# Default entry — Bazel is the authoritative build system.
all: dev

# ------------------------------ Bazel -----------------------------------------
# Authoritative build — see MODULE.bazel / bazel/ / the plan doc. C++26/named-modules
# flags live per-target now (copts/features defaults in the congelado_* wrapper
# macros, bazel/build_defs.bzl) — no --config needed, and XLA/abseil
# stay untouched.
#
# //... is safe: XLA is fetched via http_archive (third_party/xla/repo.bzl) so Bazel's package
# scanner doesn't see it in the workspace — XLA targets are only reachable via @xla//xla/... labels
# (used by //include/c and other root targets that depend on XLA).

dev: build-debug editor

#--keep_going
build:
	bazel build  //...

# bazel .bazelrc configs: --config=debug (-g, unstripped), --config=prod
# (-O3, stripped, compilation_mode=opt). Default `build` above is plain
# fastbuild — fast to compile, no optimization/debug info either way.
build-debug:
	bazel build --config=debug //...

build-prod:
	bazel build --config=prod //...

test:
	bazel test //...


format:
	find . \
		-not \( -path './build' -prune \) \
		-not \( -path './.xmake' -prune \) \
		-not \( -path './bazel-*' -prune \) \
		-not \( -path './.bzluser' -prune \) \
		-not \( -path './c' -prune \) \
		\( -name '*.cpp' -o -name '*.cppm' -o -name '*.h' -o -name '*.hpp' \) \
		| xargs -P $(shell nproc) clang-format -i

# Just the Phase 1 proof-of-concept targets, fast sanity check.
canary:
	bazel build //include/core/ffi:core_ffi //bazel/probes:gmf_probe

# Strict warnings check: -Wall -Werror on congelado code only; every dep is
# silenced via the external/.*@-w carve-out inside warnings.bazelrc.
warn-check:
	bazel build --keep_going --config=warnings //...

# Regenerates compile_commands.json from the Bazel side (bazel-compile-commands,
# standalone binary, not a Bazel dep — hedron_compile_commands doesn't support
# Bazel 9 yet, upstream #279). Writes the same path `make xmake-editor` does —
# last one run wins, they don't merge.
editor:
	bazel-compile-commands //...

clean:
	bazel clean

# ------------------------------- xmake ----------------------------------------
# The legacy build, kept under the xmake-* prefix. See MODE above.

xmake-dev: xmake-build xmake-editor

xmake-build:
	xmake f -c -y -m $(MODE)
	xmake build

xmake-install:
	xmake f -c -v -y -m $(MODE)

xmake-reinstall:
	xmake require --force

xmake-config-debug:
	xmake f -m debug --debugger=gdb

xmake-debug: xmake-config-debug
	xmake run -D -d congelado

xmake-run:
	xmake run

xmake-run-worker:
	xmake run congelado_worker config/worker.toml ./build/workers

xmake-run-worker-docker:
	xmake run congelado_worker config/docker/worker.toml ./build/workers

xmake-rebuild:
	xmake -r

xmake-windows:
	xmake f -p mingw --toolchain=mingw -c -v

xmake-linux:
	xmake f -p linux -c -v

xmake-benchmark:
	xmake run benchmark

xmake-editor:
	xmake project -k compile_commands

xmake-test:
	xmake test -v

# ---------------------------- System deps -------------------------------------

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

clean-conan:
	conan remove "*"

clean-all: clean-conan xmake-clean
	rm -rf build/ ~/.xmake/

# ------------------------------ UI (Flutter) ----------------------------------
# UI: the Flutter app (flutter/ui/) is a separate, independent project — no
# xmake target ever touches it. Run `make xmake-run` first so a live engine is
# reachable. flutter/ui is both the design-system package (congelado_hero_ui)
# and the runnable app: lib/main.dart launches the Widgetbook catalogue.
ui-run:
	cd flutter/ui && flutter pub get && flutter run -d linux

ui-build-web:
	cd flutter/ui && flutter pub get && flutter build web

ui-catalogue:
	cd flutter/ui && flutter pub get && flutter run -d chrome

# ------------------------- Insomnia API test suite ----------------------------
# Insomnia-scripted API test suite (after-response assertions on every request in the
# insomia/ collection). Requires the modern `inso` CLI (>= 10, the `core@` release binary
# from github.com/Kong/insomnia/releases — the npm `insomnia-inso` package is stuck at 3.x
# and lacks `run collection`). Runs against a server the caller already started (e.g. the
# docker-compose `test` stack, or `make compose-release-up`); it does NOT manage lifecycle.
# --disableCertValidation: server is TLS with a self-signed cert. No --bail: creates return
# 201/202/204, which --bail (abort-on-non-200) would wrongly treat as failure. --requestTimeout
# bounds the queue long-poll endpoints so an empty queue can't hang the run.
INSO_COLLECTION := insomia/Congelado API 1.0.0-wrk_e999e591aaef4a51b27267d843c09433.yaml
INSO_ENV ?= Local

# Regenerate the Insomnia test collection ($(INSO_COLLECTION)) from the OpenAPI spec
# (plugins/engine/generated/engine/openapi.json, itself regenerated at build). Run this after
# adding/removing/renaming routes: new routes appear asserted automatically (status + shape
# from the spec); only bodies/order/chaining live in the generator's scenario overlay.
# Prefers `uv run` (auto-installs the pyyaml dep via the script's PEP 723 header, no venv
# needed); falls back to a python3 that already has pyyaml — same resolution style as api-test.
gen-inso-tests:
	@if command -v uv >/dev/null 2>&1; then \
		uv run scripts/gen_inso_collection.py; \
	elif python3 -c 'import yaml' >/dev/null 2>&1; then \
		python3 scripts/gen_inso_collection.py; \
	else \
		echo "need 'uv' (https://docs.astral.sh/uv) or python3 with pyyaml (pip install pyyaml)"; exit 1; \
	fi

inso-test:
	@command -v inso >/dev/null 2>&1 || { echo "inso not found — install inso >= 10 from https://github.com/Kong/insomnia/releases (asset inso-linux-x64-<ver>.tar.xz); npm 'insomnia-inso' is too old"; exit 1; }
	inso run collection "Congelado API 1.0.0" \
		--workingDir "$(INSO_COLLECTION)" \
		--env "$(INSO_ENV)" \
		--disableCertValidation \
		--requestTimeout 15000 \
		--reporter spec \
		--ci

# --------------------------- Docker (podman compose) --------------------------

compose-env-up:
	podman compose -f docker/docker-compose.environment.yml up -d

compose-env-rm:
	podman compose -f docker/docker-compose.environment.yml down

compose-up:
	podman compose -f docker/docker-compose.debug.yml up -d

compose-update:
	podman compose -f docker/docker-compose.debug.yml build
	podman compose -f docker/docker-compose.debug.yml up -d

compose-rm:
	podman compose -f docker/docker-compose.debug.yml down

compose-release-up:
	podman compose -f docker/docker-compose.yml up -d

compose-release-update:
	podman compose -f docker/docker-compose.yml build
	podman compose -f docker/docker-compose.yml up -d

compose-release-rm:
	podman compose -f docker/docker-compose.yml down

# ----------------------------- Schemathesis -----------------------------------
# Fuzzes every route in the engine's own live-served spec (make compose-up /
# make xmake-run first — this target doesn't manage server lifecycle itself).
# Points straight at the live /openapi route rather than a committed file — the engine
# regenerates and serves that route from its own utils::openapi::Registry on every
# startup, so there's no file to keep in sync. Resolves whichever Python runner is
# actually on this machine at invoke-time, in priority order, instead of hardcoding
# one: uvx (ephemeral, no install) -> pipx run (same, no install) -> pip-installed
# schemathesis -> skip with a message if none are present. Confirm the TLS-skip flag
# name against the installed schemathesis version first (`<runner> schemathesis run
# --help`) — it has moved across major versions (--request-tls-verify=false is
# current; older releases used --tls-verify=false/-k).
api-test:
	@if command -v uvx >/dev/null 2>&1; then \
		uvx schemathesis run https://localhost:8080/openapi --checks all --request-tls-verify=false; \
	elif command -v pipx >/dev/null 2>&1; then \
		pipx run schemathesis run https://localhost:8080/openapi --checks all --request-tls-verify=false; \
	elif python3 -m pip show schemathesis >/dev/null 2>&1; then \
		python3 -m schemathesis run https://localhost:8080/openapi --checks all --request-tls-verify=false; \
	else \
		echo "schemathesis unavailable — install uv, pipx, or pip to run 'make api-test'"; \
	fi

MODE ?= debug

.PHONY: all dev build build-debug build-prod test canary editor clean download xmake-dev xmake-build xmake-install xmake-reinstall xmake-test xmake-run xmake-run-worker xmake-run-worker-docker xmake-debug xmake-config-debug xmake-rebuild xmake-windows xmake-linux xmake-benchmark xmake-editor xmake-clean clean-conan clean-all info-outdated update gen-inso-tests inso-test gen-cc-abi check-cc-abi compose-env-up compose-env-rm compose-up compose-update compose-rm compose-release-up compose-release-update compose-release-rm ui-run ui-build-web api-test format

all: dev


dev: build-debug editor

#--keep_going
build:
	bazel build  //...

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

canary:
	bazel build //include/core/ffi:core_ffi //bazel/probes:gmf_probe

warn-check:
	bazel build --keep_going --config=warnings //...

editor:
	bazel-compile-commands //...

clean:
	bazel clean

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

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

clean-conan:
	conan remove "*"

clean-all: clean-conan xmake-clean
	rm -rf build/ ~/.xmake/

ui-run:
	cd flutter/ui && flutter pub get && flutter run -d linux

ui-build-web:
	cd flutter/ui && flutter pub get && flutter build web

ui-catalogue:
	cd flutter/ui && flutter pub get && flutter run -d chrome

INSO_COLLECTION := insomia/Congelado API 1.0.0-wrk_e999e591aaef4a51b27267d843c09433.yaml
INSO_ENV ?= Local

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

gen-cc-abi:
	bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot --repo-root "$(CURDIR)"

check-cc-abi:
	bazel run //include/cc/abi_gen:cc_abi_gen -- check --pilot --repo-root "$(CURDIR)"

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

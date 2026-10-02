SHELL := /bin/bash
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
	$(SOURCE_FILES) | xargs -P $(shell nproc) clang-format -i

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

# ── CI (same targets run locally, in `make ci` container, and in .github/workflows) ──
CI_IMAGE ?= ghcr.io/driver005/congelado-ci:latest
CONTAINER ?= podman
REPORT_DIR ?= $(CURDIR)/reports
FUZZ_SECONDS ?= 60
CI_CACHE ?= congelado-ci-cache
CI_TARGET ?= ci-all
CI_EXTRA ?=
CI_RUN = $(CONTAINER) run --rm -v $(CURDIR):/workspace -v $(CI_CACHE):/root/.cache -w /workspace $(CI_EXTRA) $(CI_IMAGE)
SOURCE_FILES = find . \
		-not \( -path './build' -prune \) \
		-not \( -path './.xmake' -prune \) \
		-not \( -path './bazel-*' -prune \) \
		-not \( -path './.bzluser' -prune \) \
		-not \( -path './.cache' -prune \) \
		-not \( -path './external' -prune \) \
		-not \( -path './third_party' -prune \) \
		\( -name '*.cpp' -o -name '*.cc' -o -name '*.cppm' -o -name '*.h' -o -name '*.hpp' \)

.PHONY: ci ci-image ci-shell ci-all ci-format ci-abi ci-build ci-test ci-warnings ci-asan ci-ubsan ci-tsan ci-fuzz ci-tidy ci-secrets ci-deps ci-security ci-rulesets rulesets ci-fix-tidy ci-autofix repo-settings autofix-token github-setup

ci-image:
	$(CONTAINER) build -f docker/Dockerfile.ci -t $(CI_IMAGE) docker

ci:
	$(CI_RUN) make -k $(CI_TARGET) REPORT_DIR=/workspace/reports FUZZ_SECONDS=$(FUZZ_SECONDS)

ci-shell:
	$(CONTAINER) run --rm -it -v $(CURDIR):/workspace -v $(CI_CACHE):/root/.cache -w /workspace $(CI_IMAGE) bash

ci-all: ci-format ci-abi ci-build ci-test ci-warnings ci-asan ci-ubsan ci-tsan ci-fuzz ci-tidy ci-security

ci-format:
	@mkdir -p $(REPORT_DIR)/format
	$(SOURCE_FILES) | xargs -P $(shell nproc) clang-format --dry-run --Werror > $(REPORT_DIR)/format/clang-format.txt 2>&1; \
		test ! -s $(REPORT_DIR)/format/clang-format.txt || { head -50 $(REPORT_DIR)/format/clang-format.txt; exit 1; }

ci-abi:
	bazel run --config=ci //include/cc/abi_gen:cc_abi_gen -- check --pilot --repo-root "$(CURDIR)"

ci-build:
	bazel build --config=ci //...

ci-test:
	bazel test --config=ci //... ; status=$$?; REPORT_DIR=$(REPORT_DIR) scripts/ci/collect_reports.sh test --config=ci || status=1; exit $$status

ci-warnings:
	bazel build --config=ci --config=warnings //...

ci-asan ci-ubsan ci-tsan: ci-%:
	bazel test --config=ci --config=$* \
		--test_env=ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1:strict_init_order=1:halt_on_error=1 \
		--test_env=UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
		--test_env=TSAN_OPTIONS=halt_on_error=1:second_deadlock_stack=1 \
		//... ; status=$$?; REPORT_DIR=$(REPORT_DIR) scripts/ci/collect_reports.sh $* --config=ci --config=$* || status=1; exit $$status

ci-fuzz:
	REPORT_DIR=$(REPORT_DIR) FUZZ_SECONDS=$(FUZZ_SECONDS) scripts/ci/run_fuzzers.sh

ci-tidy:
	REPORT_DIR=$(REPORT_DIR) scripts/ci/clang_tidy.sh

ci-secrets:
	@mkdir -p $(REPORT_DIR)/security
	gitleaks git --redact --report-format sarif --report-path $(REPORT_DIR)/security/gitleaks.sarif .

ci-deps:
	@mkdir -p $(REPORT_DIR)/security
	osv-scanner scan source --recursive --format sarif --output $(REPORT_DIR)/security/osv.sarif . ; \
		osv-scanner scan source --recursive --format markdown --output $(REPORT_DIR)/security/osv.md . ; true

ci-security: ci-secrets ci-deps

# Prints the master ruleset JSON without calling GitHub.
ci-rulesets:
	scripts/ci/create_rulesets.sh --dry-run

# Creates/updates the master ruleset (needs `gh` with repo admin rights). RULESET_ENFORCEMENT=evaluate to try it first.
RULESET_ENFORCEMENT ?= active
rulesets:
	scripts/ci/create_rulesets.sh --enforcement $(RULESET_ENFORCEMENT)

# Applies clang-tidy fix-its in place (only checks that have fix-its change anything).
ci-fix-tidy:
	FIX=1 REPORT_DIR=$(REPORT_DIR) scripts/ci/clang_tidy.sh

# Linter fixes first, then the formatter, so the result is formatted; used by .github/workflows/autofix.yml.
ci-autofix: ci-fix-tidy format

# Lets workflows open pull requests (needs `gh` with repo admin rights).
repo-settings:
	scripts/ci/configure_repo.sh

# Stores a personal access token (contents + pull-requests write) as the AUTOFIX_TOKEN secret: AUTOFIX_TOKEN=<pat> make autofix-token
autofix-token:
	@test -n "$$AUTOFIX_TOKEN" || { echo 'set AUTOFIX_TOKEN=<pat>'; exit 1; }
	@printf '%s' "$$AUTOFIX_TOKEN" | gh secret set AUTOFIX_TOKEN

# One-time GitHub setup: AUTOFIX_TOKEN secret, workflow PR permission, master ruleset (needs `gh` with repo admin rights).
# AUTOFIX_TOKEN=<pat> make github-setup   (RULESET_ENFORCEMENT=evaluate to try the ruleset without blocking)
github-setup:
	$(MAKE) autofix-token
	$(MAKE) repo-settings
	$(MAKE) rulesets

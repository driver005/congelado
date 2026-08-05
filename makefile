# Build mode shared by `install`/`build` — override per-invocation with `make build MODE=release`
# (or `MODE=release make build`). Kept at "debug" by default to match local dev's historical
# behavior; docker/Dockerfile.builder's own BUILD_MODE build arg drives this the same way for
# container builds (default "release" there — see docker-compose.yml vs docker-compose.debug.yml).
MODE ?= debug

.PHONY: all download build clean info-outdated update editor dev test compose-env-up compose-env-rm compose-up compose-update compose-rm compose-release-up compose-release-update compose-release-rm ui-run ui-build-web api-test

all: dev

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

install:
	xmake f -c -v -y -m $(MODE)

dev: build editor

config-debug:
	xmake f -m debug --debugger=gdb

debug: config-debug 
	xmake run -D -d congelado

run:
	xmake run

run-worker:
	xmake run congelado_worker worker.toml ./build/workers

run-worker-docker:
	xmake run congelado_worker worker.toml ./build/workers

build:
	xmake build-all --mode=$(MODE)

rebuild:
	xmake require --force
	xmake -r

#Configuration

windows:
	xmake f -p mingw --toolchain=mingw -c -v

linux:
	xmake f -p linux -c -v

benchmark:
	xmake run benchmark

# UI: the Flutter shell (app/) is a separate, independent project — no xmake
# target ever touches it. Run `make run` first so a live engine is reachable.
ui-run:
	cd app && flutter pub get && flutter run -d linux

ui-build-web:
	cd app && flutter pub get && flutter build web

test:
	xmake test -v

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

editor:
	xmake project -k compile_commands

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

clean-conan:
	conan remove "*"

clean:
	xmake clean

clean-all: clean-conan
	rm -rf build/ ~/.xmake/

# Fuzzes every route in the engine's own live-served spec (make compose-up / make run first —
# this target doesn't manage server lifecycle itself). Points straight at the live /openapi
# route rather than a committed file — the engine regenerates and serves that route from its
# own utils::openapi::Registry on every startup, so there's no file to keep in sync. Resolves
# whichever Python runner is actually on this machine at invoke-time, in priority order,
# instead of hardcoding one: uvx (ephemeral, no install) -> pipx run (same, no install) ->
# pip-installed schemathesis -> skip with a message if none are present. Confirm the TLS-skip
# flag name against the installed schemathesis version first (`<runner> schemathesis run
# --help`) — it has moved across major versions (--request-tls-verify=false is current; older
# releases used --tls-verify=false/-k).
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

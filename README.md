<div align="center">

# congelado

**A C++26 modules-native workflow engine — HTTP/2 API, dlopen'd plugins, and a worker
fleet that executes tasks straight off the wire. No cap, no legacy header soup.**

[![license](https://img.shields.io/badge/license-OCNAL%20v1.0-blue)](LICENSE)
[![language](https://img.shields.io/badge/language-C%2B%2B26-00599C)](MODULE.bazel)
[![build](https://img.shields.io/badge/build-Bazel-43A047)](https://bazel.build)
[![modules](https://img.shields.io/badge/modules-native%20C%2B%2B%20modules-6f42c1)](bazel/)

</div>

---

## Table of Contents

- [What is this](#what-is-this)
- [Features](#features)
- [Requirements](#requirements)
- [The GCC module bug you WILL hit](#the-gcc-module-bug-you-will-hit)
- [Getting started](#getting-started)
- [UI](#ui)
- [Docker](#docker)
- [License](#license)

## What is this

congelado is an HTTP/2-native engine built entirely on real C++26 modules — no `#include`
soup pretending to be modern, actual `import`. The engine loads its capabilities (routing,
logging, storage, protocol handling) as dlopen'd plugin `.so`s at boot, and a separate
worker daemon does the same thing for task execution — you write a class with a
`get_worker_type()`/`execute_worker()`, drop it under `plugins/engine/worker/internal/`,
and the build system + runtime loader pick it up with zero daemon-side code changes. Both
loaders now also take a second, user-owned directory alongside the built-in one, so your
own custom workers/plugins don't have to live inside the build tree to get loaded — bet.

## Features

- **HTTP/2 API surface**, OpenAPI-documented, generated at boot (`plugins/http2`).
- **Plugin-loaded everything** — the engine itself is just a host; logging
  (`plugins/file_logger`), Postgres storage (`plugins/postgres`), and the protocol layer
  (`plugins/http2`) are all separately-built `.so`s wired in through one FFI ABI
  (`sdk/plugin`).
- **Worker fleet** — a standalone daemon (`congelado_worker`) polls the engine for queued
  tasks and dispatches them to dlopen'd task workers (`sdk/worker`, `CONGELADO_TASK` ABI).
  Ships with `echo` and `transform` as reference implementations.
- **Dual load-path everywhere** — both the worker and plugin loaders take an *internal*
  (build-managed, don't touch it) directory and an *external* (yours, point it wherever)
  directory in the same call, so custom workers/plugins never have to be smuggled into the
  build output to get picked up.
- **Zero-boilerplate OpenAPI client SDK generation** (`congelado_cli generate`) straight
  from any running server's live `/openapi` route — engine and worker both serve their own.
- **io_uring only** — no epoll/posix fallback path to maintain, one leverager, one way to
  do I/O, that's the motion.

## Requirements

This project is developed and validated against **Arch Linux** (rolling), on purpose —
Debian/Ubuntu's packaged GCC (13/14/15) each hit real, version-specific `import std`
module bugs (missing `<print>`, missing `std.cc` entirely, missing the std module export
for range-adaptor pipe operators). Arch's rolling clang+gcc combo sidesteps all of that by
construction. If you're not on Arch, a container is the path of least resistance — see
[Docker](#docker) below.

**Toolchain** (versions this repo is actually validated against):

| Tool | Version |
|---|---|
| Bazel | 9.x+ (via `bazelisk` — resolves the pin in `.bazelversion`) |
| xmake (legacy compatibility) | 3.0.9+ |
| Conan | 2.21.0+ |
| clang / clang++ | 22.1.8+ |
| LLD | 22.1.7+ |

**System packages** (`pacman -S --needed ...`), pulled straight from what the project's
own Docker builder installs (`docker/Dockerfile.server`) plus the bare-metal `makefile`
dependency target. `bazelisk` is the authoritative build's toolchain; `ccache` is required
by `.bazelrc`'s `CC`/`CXX` wiring (uncached compiles of the vendored LLVM/XLA dep graph are
brutally slow) — `mold` is an optional faster alternative to `lld` (`bazel build
--config=mold //...`):

```bash
sudo pacman -S --needed \
  bazelisk ccache mold \
  xmake conan curl wget git unzip base-devel readline cmake ninja pkgconf \
  liburing python python-pip clang lld libc++ libc++abi tk nasm \
  libfontenc libice libsm libxaw libxcomposite libxcursor libxdamage libxtst \
  libxinerama libxkbfile libxrandr libxres xcb-util-wm xcb-util-image \
  xcb-util-keysyms xcb-util-renderutil libxxf86vm libxv xcb-util xcb-util-cursor

# ccache's default 5G cache is too small for this dep graph — bump it once:
ccache --max-size=60G
```

The X11/xcb chain isn't decorative — cpython's Tk support pulls it in transitively at
build time, and the runtime image needs the matching libs too, straight up.

**Everything else is Conan's problem, not yours** — `xmake f -c` pulls every C++
dependency automatically the first time you configure:

asio · openssl · libnghttp2 · nghttp3 · simdjson · protobuf · catch2 · cli11 ·
backward-cpp · libffi · tomlplusplus · stduuid · cpython · lua · reflect-cpp · sqlgen ·
microsoft-gsl · range-v3

No manual vendoring, no submodules to forget to init — that's the whole point of the
Conan layer, don't fight it.

## The GCC module bug you WILL hit

Straight talk: on a fresh Arch install you will very likely hit this exact error the first
time you `xmake build`:

```
error: no such file or directory: '/usr/lib64/gcc/x86_64-pc-linux-gnu/include/c++/16/bits/std.cc'
```

This isn't a congelado bug and it isn't an xmake bug — it's an upstream GCC bug
([`libstdc++/119266`](https://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg854259.html))
in the build script (`contrib/relpath.sh`) that generates `libstdc++.modules.json`. That script computes
the relative path from where the manifest lands (`/usr/lib/gcc/<target>/<ver>/`) to where
the actual `bits/std.cc`/`bits/std.compat.cc` module sources live
(`/usr/include/c++/<ver>/bits/`) — and gets it one directory level too shallow on the
completely standard Linux layout where headers install separately from the
GCC-versioned lib dir. Gentoo and Fedora patch this at the package level; this Arch build
doesn't (yet), no cap.

**Already fixed for you in the Docker build** — see
[`docker/Dockerfile.server:28-38`](docker/Dockerfile.server) for the containerized version
of this exact fix. For bare metal, run the equivalent two-liner once:

```bash
sudo mkdir -p /usr/lib/gcc/x86_64-pc-linux-gnu/include
sudo ln -sfn /usr/include/c++ /usr/lib/gcc/x86_64-pc-linux-gnu/include/c++
```

That makes the manifest's (wrong) relative path resolve to where the headers actually are,
without touching any package-managed file directly. It won't survive a GCC major-version
bump (new `/17` dir down the line) — if it stops working after a `pacman -Syu`, just rerun
it. Not a footgun, just a one-liner you'll run again eventually.

## Getting started

```bash
# 1. Install system + toolchain deps, detect the Conan profile
make dependency

# 2. Build with the authoritative Bazel workflow (default: fast, unoptimized fastbuild)
bazel build //...

# ...or make build-debug / make build-prod for --config=debug (-g, gdb-ready)
# / --config=prod (-O3, stripped)

# 3. Run the C++ test suite
bazel test //...

# Generate compile_commands.json for your editor
make editor

# The legacy xmake workflow remains available when needed:
make xmake-install
make xmake-build

# Debug build + gdb
make xmake-debug

# Regenerate compile_commands.json for your editor
make xmake-editor

# Run the C++ test suite
make xmake-test

# Run the Insomnia-scripted API suite against a running server (needs the modern `inso`
# CLI, >= 10; see below). Start a server first (e.g. `make compose-release-up`).
make inso-test

# Schema-fuzz the live OpenAPI surface
make api-test
```

See the `makefile` itself for the full target list — `windows`/`linux` for platform
reconfiguration, `clean`/`clean-all` for teardown, `info-outdated` for Conan dependency
drift.

### API testing (Insomnia scripts)

HTTP-level API tests live in the checked-in Insomnia collection
`insomia/Congelado API 1.0.0-*.yaml`. Every request carries an **after-response** script
(`insomnia.test` / `insomnia.expect`, chai-style) asserting status code + response shape,
and the create→start→exec / enqueue→poll→result flows chain via
`insomnia.environment.set('exec_id'|'task_id', …)`. The collection defines two
sub-environments: `Local` (`https://localhost:8080`) and `Docker` (`https://server:8080`).

The collection is **generated** — don't hand-edit it. `make gen-inso-tests` rebuilds it from
the OpenAPI spec (`plugins/engine/generated/engine/openapi.json`, itself regenerated at
build) via `scripts/gen_inso_collection.py`. Status + shape assertions come from the spec;
request bodies, run order, and the id-chaining live in that script's scenario overlay. Add a
route → regenerate → it appears asserted automatically (soft `< 500` if the spec declares no
response); give it an overlay entry only if it needs a body, an order slot, or a chained id.
Runs under `uv run` (auto-installs its `pyyaml` dep) or any `python3` with `pyyaml`.

- **GUI:** open the collection in Insomnia and use the Collection Runner.
- **CLI/CI:** `make inso-test` (or the compose `test` service). The server is HTTP/2-only
  over TLS with a self-signed cert, so runs pass `--disableCertValidation`. The runner needs
  `inso` **>= 10** — download the `core@` release binary from
  <https://github.com/Kong/insomnia/releases> (asset `inso-linux-x64-<ver>.tar.xz`); the npm
  `insomnia-inso` package is stuck at 3.x and lacks `run collection`.
- **Containerized:** `podman compose -f docker/docker-compose.yml up --build test` runs the
  suite (via `docker/Dockerfile.test`, which installs `inso`) against the `server` service
  after its healthcheck — this replaces the former `docker/test/run.sh` curl harness.

> Note: the suite is meant to run against the compose **release** stack (with Postgres and
> the other backends wired). The local debug + AddressSanitizer `make xmake-run` server is too slow
> under HTTP/2 load for the full suite.

## UI

Plugins can optionally ship real UI — their own pages mounted into one shared sidebar/tabs
shell — as ordinary hand-written Flutter code, strictly independent of the C++ backend. Every
plugin's C++ sources live under `plugins/<name>/src/`; a plugin that wants UI adds a sibling
`plugins/<name>/ui/` Dart package (see `plugins/engine/ui/` as the reference example) that
talks to that plugin's own REST endpoints with plain `package:http` — no code generation, no
FFI, no engine involvement at all. `flutter/ui/` is the one Flutter app (see
`flutter/ui/README.md`): the `congelado_hero_ui` design-system package, also runnable as an
app — `lib/main.dart` launches its Widgetbook catalogue (web, desktop, mobile from a single
codebase).

```bash
# One-time: scaffold flutter/ui/'s platform runner folders (needs the Flutter SDK)
cd flutter/ui && flutter create --platforms=web,linux,windows,macos,android,ios --project-name congelado_hero_ui . && cd ..

# Run the Widgetbook catalogue (needs no engine — pure Flutter)
make ui-catalogue

# Build for web
make ui-build-web
```

Don't have the Flutter SDK on your host? `docker/Dockerfile.ui` builds the web target inside a
container that has it and serves it over nginx — wired up as the `ui` service in
`docker/docker-compose.yml`, available at http://localhost:8081 once it's up:

```bash
podman compose -f docker/docker-compose.yml up -d ui
```

## Docker

Don't want to touch your host toolchain at all? `docker/docker-compose.yml` wires up all
three pieces — engine server, worker, and an integration-test runner that waits on the
server's health check before running:

```bash
docker compose -f docker/docker-compose.yml up --build
```

Podman person? Same compose file, `podman compose` reads it straight:

```bash
podman compose -f docker/docker-compose.yml up --build
```

The Dockerfile itself already carries the GCC modules.json fix (see above) baked in — one
less thing to think about.

**Debug vs release:** `docker/docker-compose.yml` builds in release mode by default (the builder
image's own `BUILD_MODE` build arg defaults to `release`). `docker/docker-compose.debug.yml` is an
identical-topology twin that builds in debug mode instead — same services, same ports, only the
compile mode differs. `make compose-up`/`make compose-update`/`make compose-rm` point at the debug
file (fast dev iteration); `make compose-release-up`/`make compose-release-update`/`make
compose-release-rm` point at the release one. Prefer the raw `docker`/`podman compose` invocations
above? Just swap `-f docker/docker-compose.yml` for `-f docker/docker-compose.debug.yml` to get the
debug build instead.

## License

[Open Core Non-AI License (OCNAL) v1.0](LICENSE) — Copyright (c) 2026 Adrian Fernandez.

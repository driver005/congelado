# syntax=docker/dockerfile:1
# ── Congelado Builder ──────────────────────────────────────────────────────────
# Single-stage image whose only job is compiling the project once and handing the result to
# every other service (server/worker) via a shared named volume — see the `builder` service in
# docker-compose.yml (`depends_on: condition: service_completed_successfully` gates server/worker
# on this container finishing first). Previously Dockerfile.server and Dockerfile.worker each ran
# their own full "FROM archlinux AS builder" compile independently — three redundant from-scratch
# builds (this one plus whatever Dockerfile.test also did) for the exact same source tree.
# Base: Arch Linux — matches the toolchain (clang/gcc/liburing versions) this project is
# actually developed against. Debian/Ubuntu's packaged GCC (13/14/15) each hit real,
# version-specific "import std" module bugs (missing <print>, missing std.cc entirely,
# missing std module export for range-adaptor pipe operators); Arch's rolling-release
# clang+gcc avoids all of that by construction.
# ────────────────────────────────────────────────────────────────────────────────

FROM archlinux:latest AS builder

# Build mode for `make install`/`make build` below — "release" by default (matches
# docker-compose.yml; docker-compose.debug.yml overrides this to "debug" via --build-arg). Docker
# auto-exports a declared ARG as an env var to every subsequent RUN in this stage, so $BUILD_MODE
# is directly usable in the make invocations below with no extra ENV needed.
ARG BUILD_MODE=release

# Container runs as root with no USER directive; xmake refuses to run as root otherwise.
ENV XMAKE_ROOT=y
# Cap parallel compile jobs — 12 host cores against limited container RAM has been
# spiking swap hard enough to trigger silent kills during the openssl/cpython/bison
# Conan builds; capping keeps peak memory bounded.
ENV MAKEFLAGS=-j4

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm --needed \
       curl wget git unzip rsync base-devel readline cmake ninja pkgconf liburing \
       python python-pip clang lld libc++ libc++abi tk krb5 \
       libfontenc libice libsm libxaw libxcomposite libxcursor libxdamage libxtst \
       libxinerama libxkbfile libxrandr libxres xcb-util-wm xcb-util-image \
       xcb-util-keysyms xcb-util-renderutil libxxf86vm libxv xcb-util xcb-util-cursor
# krb5 matters here: conan's libpq recipe never sets an explicit gssapi option — PostgreSQL's
# own meson build defaults that feature to "auto", so GSSAPI support gets silently linked in
# whenever krb5 happens to be present wherever libpq gets compiled. Shared-library linking
# allows unresolved dynamic symbols by default on Linux, so without krb5 here the *link* step
# for libpostgres_plugin.so would silently produce a .so with an unresolved
# `GSS_C_NT_HOSTBASED_SERVICE`, deferred to a dlopen-time abort in whichever service loads it.

# Arch's gcc package installs libstdc++.modules.json under /usr/lib/gcc/<target>/<ver>/
# but ships the actual bits/std.cc (the C++23 std module source) under the unrelated
# /usr/include/c++/<ver>/ tree instead of "next to" the manifest as its own relative
# source-path (../include/c++/<ver>/bits/std.cc) assumes. Symlink it into place so
# clang's std-module lookup (which follows that relative path literally) finds it.
RUN gccver=$(gcc -dumpversion | cut -d. -f1) \
    && mkdir -p /usr/lib/gcc/x86_64-pc-linux-gnu/include/c++/${gccver}/bits \
    && ln -sf /usr/include/c++/${gccver}/bits/std.cc \
       /usr/lib/gcc/x86_64-pc-linux-gnu/include/c++/${gccver}/bits/std.cc \
    && ln -sf /usr/include/c++/${gccver}/bits/std.compat.cc \
       /usr/lib/gcc/x86_64-pc-linux-gnu/include/c++/${gccver}/bits/std.compat.cc

# xmake
RUN curl -fsSL https://xmake.io/shget.text | bash \
    && ln -sf /root/.local/bin/xmake /usr/local/bin/xmake

# Conan 2 — tried installing this via pacman first (matching local dev's own `yay -S xmake conan
# libc++`) since it'd mean one less package manager in the image, but Arch's official repos don't
# actually carry a `conan` package (pacman -S conan fails: "target not found") — confirmed by an
# actual failed build, not assumed. Back to pip. >=2.21.0 is this project's documented floor (see
# README.md): opentelemetry-cpp's transitive libcurl recipe needs it. Docker installs unpinned
# latest from PyPI already, so this floor is normally a no-op — pinning it explicitly just makes
# the requirement enforced, not implicit.
RUN pip3 install --no-cache-dir --break-system-packages 'conan>=2.21.0' \
    && conan profile detect --force

# Copy project
WORKDIR /app
COPY . .

# Configure xmake (fetches Conan deps too) — no --sdk override needed; Arch's
# clang/libc++/gcc live at the default paths xmake already expects, mirroring how the
# project builds on bare-metal Arch.
#
# --mount=type=cache,target=/root/.conan2 persists conan's package store across separate
# `docker build` runs, independent of Docker's normal layer cache — without it, editing any
# source file invalidates this RUN (COPY . . above changed) and conan re-downloads/recompiles
# every dependency (asio/openssl/opentelemetry-cpp/etc, the genuinely slow part) from scratch
# every time, even though none of the *dependencies* actually changed.
#
# --mount=type=cache,target=/root/.xmake caches xmake's own GLOBAL dir (~/.xmake: the
# xmake-repo package-index clone + toolchain-detection cache, 60MB+) the same way — safe to
# reuse across otherwise-independent builds since it's tool/repo-index state, not build
# output. This is NOT the same thing as this project's own per-project `.xmake/` config dirs
# (repo root's and plugins/'s), which stay uncached — nor is it the same as build/'s own
# incremental BMI cache, which IS now reused across builds too, but through a separate
# fingerprint-guarded restore/save around `make build` below rather than a plain mount here (see
# that RUN's own comment for why, and for the safety net guarding the real stale-incremental-BMI
# -cache Clang crash a naive reuse of this state caused once this session).
RUN --mount=type=cache,target=/root/.conan2 --mount=type=cache,target=/root/.xmake make install MODE="$BUILD_MODE"

# Build everything, in one pass. engine (plugins/engine/) is its own standalone xmake project
# now — its build.cc runs and writes the generated OpenAPI client SDK as part of that project's
# own, separate invocation, which "xmake build-all" (xmake/tasks/build_all.lua) always runs
# before the root project's own remaining targets (congelado_worker, the one consumer of that
# generated SDK). This used to need two full "make build" passes by hand (a genuine, confirmed
# xmake C++-modules limitation when codegen and consumer share one project/invocation — see
# xmake/core_layers.lua's own comment) — splitting engine out into its own project sidesteps it
# instead of working around it.
# build/ is cached across separate `podman/docker build` runs, but NOT via a plain
# --mount=type=cache directly on /app/build: content written under a cache mount never persists
# into the final image layer once the RUN that used it exits, so the CMD below (cp -a
# /app/build/. /output/) would find nothing there. Instead this RUN mounts the cache at
# /root/.cache/congelado-buildcache (a path that is NEVER part of the image) and rsyncs into/out
# of the real /app/build path inside this one RUN, before the mount tears down — the same
# "extract from a cache mount into a real path before it unmounts" idea the cpython-stdlib RUN
# below already relies on, just done in both directions (restore before building, save after).
#
# Safety net against a real stale-incremental-BMI-cache Clang crash hit once this session when
# build/ state got reused across what should have been independent builds: before trusting the
# cached build/ tree, this computes a fingerprint of everything that defines the module graph and
# toolchain — clang version, xmake version, and the contents of every xmake.lua in the repo
# (deliberately NOT any of the project's actual .cppm/.cc sources — source-only edits are exactly
# the case this cache exists to survive) — and compares it against a stamp left *inside* the
# cached build/ tree by whichever build last populated it. Mismatch (including "never built
# before") wipes the cached tree and starts clean instead of trusting possibly-incompatible BMI
# state; a match restores it and lets xmake's own normal incremental staleness check (source hash
# + compiler flags) take over from there. Note: because this Dockerfile does a fresh `pacman -Syu`
# every build (Arch is rolling-release), an upstream clang/xmake bump alone — with zero source
# changes — will also flip this fingerprint and force one full rebuild; that's intended, not a bug.
#
# sharing=locked serializes access to this cache mount so two builds that overlap can't write to
# it concurrently and corrupt it — a second, independent way to end up with a "stale/corrupt
# cache" class of failure. Requires buildah >=1.40 / podman >=5.5 for the `sharing=` option on
# --mount=type=cache; if an older podman/buildah is ever used, buildah fails the build with an
# explicit "invalid mount option" error (not a silent no-op) — drop `,sharing=locked` back to the
# default `sharing=shared` (or use `sharing=private` for always-fresh, no cross-build reuse but no
# corruption risk either) as a fallback.
RUN --mount=type=cache,target=/root/.conan2 \
    --mount=type=cache,target=/root/.xmake \
    --mount=type=cache,id=congelado-buildcache,target=/root/.cache/congelado-buildcache,sharing=locked \
    sh -c '\
    CACHE=/root/.cache/congelado-buildcache && \
    STAMP="$CACHE/build/.docker-buildcache-fingerprint" && \
    mkdir -p "$CACHE/build" && \
    fingerprint=$( { clang --version; xmake --version; find /app -name xmake.lua | sort | xargs cat; } | sha256sum | cut -d" " -f1 ) && \
    if [ -f "$STAMP" ] && [ "$(cat "$STAMP")" = "$fingerprint" ]; then \
        echo "builder: build/ cache fingerprint matches - reusing cached build/ tree" && \
        rsync -a --exclude=.docker-buildcache-fingerprint "$CACHE/build/" /app/build/ ; \
    else \
        echo "builder: build/ cache missing or stale (fingerprint mismatch) - starting from a clean build/" && \
        rm -rf "${CACHE:?}/build" && mkdir -p "$CACHE/build" ; \
    fi && \
    make build MODE="$BUILD_MODE" && \
    rsync -a --delete --exclude=.docker-buildcache-fingerprint /app/build/ "$CACHE/build/" && \
    echo "$fingerprint" > "$STAMP" \
    '

# python_bridge_plugin dynamically links libpython3.12.so (already resolved fine via its own
# add_syslinks + LD_LIBRARY_PATH), but embedding CPython also needs its actual *standard
# library* — the .py/.so files under conan's cpython package (encodings/, etc.) — on disk at
# runtime to even initialize the interpreter ("No module named 'encodings'" otherwise). Those
# files only exist inside this RUN's cache-mounted conan store, which never reaches the image
# or the shared volume — so copy the stdlib tree out to a real (non-cache-mounted) path here,
# same idea as syncing build/ itself below. PYTHONHOME gets pointed at this in
# Dockerfile.server/.worker (both scan build/plugins/ and can load python_bridge_plugin).
RUN --mount=type=cache,target=/root/.conan2 sh -c '\
    cpython_pkg=$(find /root/.conan2/p/b -maxdepth 1 -iname "cpyth*" | head -1) && \
    mkdir -p /app/build/python-stdlib/lib && \
    cp -a "$cpython_pkg/p/lib/python3.12" /app/build/python-stdlib/lib/python3.12 \
    '

# server/worker both derive their plugin directory as "<binary's own dir>/../../../plugins" —
# the exact relative offset xmake's own build/linux/x86_64/debug/<bin> layout produces. Sync the
# *entire* build/ tree, unflattened, into the shared volume so that offset math keeps landing on
# a real directory for every consumer, instead of each Dockerfile re-deriving its own (and
# previously inconsistent — see the docker-compose.yml comment on the old flattened layouts)
# subset of paths.
CMD ["sh", "-c", "mkdir -p /output && cp -a /app/build/. /output/ && echo 'builder: build/ synced to /output'"]

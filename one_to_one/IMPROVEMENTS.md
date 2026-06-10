# Improvement Ideas (Run 1 observations)

Items are tagged with the source file where the idea originates.
None of these were implemented — they are proposals for the Run 3 improvement pass.

---

- **utils/encode.cppm** — "Zero-copy encode output via caller-owned slice"
  C++ returned `std::string` (heap alloc per call). D port already writes into a
  caller-owned `char[]`. A follow-up improvement could add an `encode_into` variant
  that accepts a `BufferWriter` node directly and skips the intermediate buffer entirely.

- **utils/codec/atom.cppm** — "Replace class ranges with inline-struct ranges"
  `BigEndianView` and `VariantEndianView` are D classes (heap-allocated). Because they
  are tiny (≤ 3 words of state) and short-lived, converting them to structs (with the
  value-wrapper exemption) would eliminate `make!`/`dispose` round-trips on every encode.

- **utils/buffering/deleter.cppm** — "Replace std::function<> action with fn+ctx pair"
  The D port already uses `void function(void*) @nogc nothrow` + `void* ctx` to stay
  `@nogc`. Consider a typed mixin variant `TypedDeleter!T` that avoids the void* cast
  site at the call location for better type safety.

- **utils/buffering/reader.cppm** — "BufferReader.front() as named struct vs pair"
  C++ returns `std::pair<const std::byte*, std::size_t>`. D port uses a nested
  `FrontResult` struct. Consider promoting `FrontResult` to a module-level type shared
  with `BufferView` so callers can name the type uniformly.

- **utils/hashmap/swiss.cppm** — "Scalar fallback path for match_byte on non-x86"
  The C++ always uses `_mm_cmpeq_epi8`; the D port adds a scalar fallback behind
  `version(X86_64)`. The scalar path could be further accelerated with a SWAR
  (SIMD-within-a-register) 64-bit trick to handle 8 control bytes per iteration
  without any SIMD intrinsics.

- **utils/queue/queue.cppm** — "Thread management fields missing in D port"
  C++ `ConcurrentQueue` carries `m_threads_mu`, `m_producers` (jthreads), and
  `m_consumers`. The D port drops them with a TODO. For the improvement pass, wire in
  a `@nogc` thread pool (e.g. from `core.thread.osthread`) with a lightweight stop
  flag instead of `std::stop_token`.

- **utils/queue/pager.cppm** — "size() accessor missing on Pager"
  `ConcurrentQueue.size_approx()` calls `pager.size()` and `winner.size()`, but `Pager`
  never exposes a `size()` method in either the C++ or D source. Add a derived
  `size_t size() const` that returns `m_writer - m_reader` (approximate, relaxed load).

- **modules/asio.cppm** — "Replace Asio with native D async I/O"
  The C++ port depends on the header-only Asio library for its executor model, strands,
  and SSL streams. The D port stubs this module out. For Run 3, wire the leverage layer
  (io_uring/POSIX) and OpenSSL directly so the asio.d stub can be removed entirely.

- **modules/errno.cppm** — "Trim errno.d re-export list to actually-used codes"
  The current D port re-exports the full POSIX errno list for completeness. After Run 2
  identifies which codes are referenced by the codebase, trim the public import to only
  those symbols to reduce symbol pollution.

- **modules/openssl.cppm** — "Adopt deimos/openssl for full OpenSSL binding"
  The hand-written minimal binding in openssl.d covers only SSL_CTX and SSL_new.
  When deimos/openssl is added to dub.sdl, the `version(Have_deimos_openssl)` branch
  automatically expands to the full binding, removing the need for hand declarations.

- **shared/handler.cppm** — "this_handler TLS fields need explicit __gshared annotation"
  The D port declares `this_handler.current` and `current_id` as plain static fields,
  which gives per-thread storage (correct). In the improvement pass, annotate them with
  explicit `static` + a comment confirming TLS semantics match the C++ `thread_local`.

- **shared/flow.cppm** — "Callback aliases use raw fn+ctx pairs; consider typed closures"
  All five callback aliases (ReadCallback, SendCallback, etc.) are `void function(void*,
  ...)` pairs to stay @nogc. For the improvement pass, evaluate a thin `Closure!Fn`
  struct (fn pointer + opaque context word) so call sites get type-checked context
  instead of bare void*.

- **interfaces/io.cppm** — "Concept definitions replaced by comments only"
  The C++ file is purely concept declarations (no runtime code). The D port renders them
  as structured comments. For Run 3, evaluate whether D `template` constraints or a
  dedicated `Satisfies!(T, ...)` mixin can give compile-time enforcement without
  duplicating the entire constraint at every instantiation site.

- **interfaces/request.cppm + response.cppm** — "Builder chaining lost in CRTP port"
  C++ used deducing-this (`Self&&`) to return the derived type from `add_header`,
  `remove_header`, `with_status`, and `build`. D has no equivalent; the methods are
  dropped to abstract stubs. For Run 3, consider a mixin template `BuilderMixin`
  that concrete classes include to re-add the chaining pattern.

- **interfaces/logger.cppm** — "ILogger as extern(C++) interface"
  Marking ILogger `extern(C++)` gives ABI stability across plugin .so boundaries
  (matching the C++ intent). In Run 3 verify that LDC correctly mangles the vtable
  and that the D-side destructor slot aligns with the C++ virtual destructor.

- **interfaces/protocol.cppm** — "ReceiveDispatchFn / SendDispatchFn as fn+ctx structs"
  The C++ `std::function` wrappers are replaced with `struct { fn; ctx; }` pairs. For
  Run 3, replace them with the proposed `Closure!Fn` type from shared/flow once that
  improvement is implemented, to keep dispatch callback shapes consistent.

- **interfaces/protocol.cppm** — "IProtocol::get_server/get_client return null instead of throw"
  C++ default implementations throw `std::runtime_error`; the D port returns `null`
  (exception-free). Callers that previously relied on the throw for "not implemented"
  detection must now null-check. In Run 3, document or enforce this via an assert/abort
  in debug builds.

- **interfaces/codec/cache.cppm + db.cppm** — "ICacheCodec/IDbCodec as template classes, not interfaces"
  D interfaces cannot be parameterized; ICacheCodec!T and IDbCodec!T are D template
  classes. This loses the guarantee that they have no data members. In Run 3, add a
  `static assert(ICacheCodec!T.sizeof == __traits(classInstanceSize, Object))` style
  check, or restructure as abstract mixin templates.

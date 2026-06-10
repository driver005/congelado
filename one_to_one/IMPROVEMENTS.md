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

---

# Improvement Ideas (Run 1 — io/shared + io/codec/*)

Items from the write-only pass over Groups 1-5. None implemented.

- **io/codec/shared/huffman.cppm** — "Huffman TransTable as CTFE immutable"
  C++ left `inline static const TransTable<W> TABLE = build_table<W>()` with a
  `TODO: make constexpr`. D port uses `shared static this()` for runtime init, preserving
  that TODO. In Run 3, evaluate whether `build_table!W()` can be evaluated at compile time
  as a CTFE function and stored as `static immutable`. This would eliminate the runtime
  `shared static this()` and make the lookup table a true compile-time constant.

- **io/codec/shared/table.cppm** — "DynamicTable ring-buffer eviction via circular array"
  The C++ port used a `std::deque`; D port uses a dynamic array `m_deque[]` with linear
  eviction. For Run 3, replace with a ring-buffer (power-of-two capacity, head+tail
  indices) to make O(1) eviction without any heap reallocation.

- **io/codec/shared/table.cppm** — "StaticTable template param (non-type ref) not portable to D"
  C++ StaticTable is `template<const auto& Table>` (non-type parameter binding a static
  array). D has no direct equivalent. The D port uses `StaticTableBase` with a runtime
  `init(slice)` call. In Run 3, consider a template `StaticTableBase(alias Table)` that
  accepts an alias to the immutable array for compile-time resolution of the slice.

- **io/codec/hpack/table.cppm + io/codec/qpack/table.cppm** — "Static field tables as CTFE"
  Both HPackTable and QPackTable initialize `__gshared HeaderFieldStatic[N]` arrays in
  `shared static this()`. These entries are literal constants (all string and token values
  known at compile time). In Run 3, convert to `static immutable HeaderFieldStatic[N]`
  initialized via a CTFE lambda, eliminating the mutable shared statics entirely.

- **io/codec/hpack/hpack.cppm** — "HpackEncoder m_buf as mutable member note"
  C++ declared `mutable std::array<std::byte, 16384> m_buf` to permit buffer writes on
  a logically-const encoder. D has no `mutable`; the field is a plain member. In Run 3,
  audit all `const` call sites to verify no `m_buf` mutation is attempted through a
  const reference.

- **io/codec/qpack/qpack.cppm** — "Huffman encode/decode paths stubbed as TODO"
  The QPACK encoder/decoder string paths have `// TODO: huffman encode/decode` stubs
  matching the C++ TODO comments. In Run 3, wire the `Huffman!W.encode()` and
  `Huffman!W.decode()` calls from `io.codec.shared.huffman` to complete the
  Huffman-compressed string encoding path.

- **io/codec/quic/tls.cppm** — "ALPN select callback dropped"
  The C++ TlsContext wired an ALPN select callback (`SSL_CTX_set_alpn_select_cb`) using
  a lambda capture. D's `@nogc nothrow` function pointers cannot capture; the callback
  was dropped with a TODO. In Run 3, implement via a module-level C-ABI callback
  (`extern(C) int alpn_select_cb(...)`) that reads an immutable protocol list from a
  module-level constant, restoring ALPN negotiation without GC.

- **io/codec/quic/connection.cppm** — "StreamEntry[] linear scan → SwissHashMap"
  C++ `std::unordered_map<uint64_t, SSL*>` was ported to D `StreamEntry[]` dynamic array.
  Stream lookup (`write_stream`) is O(n). In Run 3, replace with
  `SwissHashMap!(ulong, SSL*)` from `util.hashmap.swiss` for O(1) average lookup,
  matching the original C++ intent.

- **io/codec/quic/connection.cppm** — "Stream read buffer stack-allocated, limits message size"
  `poll_streams` uses a `ubyte[65536]` stack buffer per stream per tick. QUIC streams can
  carry larger application messages. In Run 3, replace with a caller-supplied
  `BufferWriter` node or a module-level ring-buffer passed by reference, removing the 64
  KiB cap and the risk of stack overflow on deeply recursive poll paths.

- **io/shared/http/header.cppm** — "HeaderEntry tagged union vs std::variant"
  C++ used `std::variant<shared_ptr<HeaderField<true>>, shared_ptr<HeaderField<false>>>`.
  D port uses a manual tagged union `struct HeaderEntry { HeaderEntryKind kind; union { ... } }`.
  In Run 3, evaluate `std.variant.Algebraic` or a custom `SumType!T` for safer exhaustive
  matching, or add a helper `opDispatch`/`match` to make the tag-switch pattern less
  error-prone at call sites.

- **io/codec/shared/atom.cppm** — "Sentinel-return error convention needs documentation"
  `decode_int` and `decode_string` signal errors by returning `consumed == 0`. C++ threw
  exceptions. In Run 3, upgrade to `Result!(T, DecodeError)` from `util.result` so call
  sites cannot silently ignore decode failures.

---

# Improvement Ideas (Run 1 — io/error + io/base/leverage + io/base/socket)

Items from the write-only pass over the 12 files in this batch. None implemented.

- **io/error/http.cppm** — "Exception hierarchy → tagged-union Result type"
  All 10 exception classes (Http2Exception, StreamError, ConnectionError, DecodeError and
  its 7 sub-types, CompressionError) were collapsed to plain structs with a `DecodeErrorKind`
  tag enum. In Run 3, unify them under a `Result!(T, IoError)` where `IoError` is a sum type
  over all error variants, so call sites get exhaustive matching via `final switch`.

- **io/error/base.cppm** — "TlsError fixed-size message buffer"
  The C++ TlsError formatted dynamically into `std::string`. The D port uses a 512-byte
  stack buffer (ctx + ": " + ERR_error_string). In Run 3, evaluate whether 512 bytes is
  sufficient in practice, or introduce a caller-supplied buffer parameter to make the limit
  explicit.

- **io/base/leverage/uring.cppm** — "for_each_cqe macro expansion vs peek loop"
  The C++ `io_uring_for_each_cqe` macro uses a two-argument head/cqe loop for efficiency;
  the D port replaces it with a sequential `io_uring_peek_cqe` + `io_uring_cqe_seen` loop.
  In Run 3, expose a proper `io_uring_for_each_cqe` binding (possibly via ImportC shim) to
  restore the original batch-drain semantics without the extra `io_uring_cqe_seen` per entry.

- **io/base/leverage/uring.cppm** — "io_uring_cqe_get_res helper is not a real liburing API"
  The D port declares `io_uring_cqe_get_res` and `io_uring_cqe_get_user_data` as extern(C)
  helpers, but these don't exist in liburing — cqe->res is a direct field access. In Run 2,
  expose the `io_uring_cqe` struct layout in the c/uring.c ImportC shim and replace the
  helper calls with direct field access.

- **io/base/leverage/posix.cppm** — "sync_file_range flags field not set"
  The C++ sets `sqe->sync_range_flags = sync_range_flags` after `io_uring_prep_rw`. In the
  D port this is stubbed out with a comment because the `io_uring_sqe` struct layout is
  not exposed by the c/uring.c shim. In Run 2, expose the sqe field offsets in the shim.

- **io/base/leverage/types.cppm** — "completion_callback as fn+ctx pair vs typed closure"
  C++ used `std::move_only_function<void(int)>` (owning, non-copyable). D port uses a bare
  `fn + ctx` pair (copyable, non-owning). This means the ctx pointer must outlive every
  call. In Run 3, introduce a `Closure!(void, int)` type from `util.closure` that owns a
  malloc'd context block and frees it on last copy, matching the C++ ownership semantics.

- **io/base/leverage/win32.cppm** — "AcceptEx / ConnectEx extension function loading missing"
  `load_extension_functions()` is a stub — the WSAID_CONNECTEX / WSAID_ACCEPTEX GUIDs and
  WSAIoctl calls are commented out. In Run 2, wire the full Win32 extension-function loading
  so `accept()` and `connect()` actually use overlapped I/O rather than returning
  `ERROR_NOT_SUPPORTED`.

- **io/base/socket/socket.cppm** — "Endpoint address stored as char[256] limits long hostnames"
  C++ used `std::string` (unbounded). D port uses `char[256]` for @nogc compatibility.
  255-character hostnames are unusual but valid; in Run 3 consider raising to 1024 or
  accepting a caller-supplied buffer to handle edge cases.

- **io/base/socket/socket.cppm** — "async_connect / async_accept / async_send / async_receive stubs"
  All four async methods are stubbed with a PORT-NOTE comment. The C++ used
  `shared_ptr<function<void(int)>>` for recursive retry; @nogc D needs an explicit
  heap-allocated continuation struct. In Run 2, implement these using `make!/dispose` from
  `util.alloc` with a fixed retry-state struct per operation.

- **io/base/socket/socket.cppm** — "generate_certificate() dropped entirely"
  C++ called `std::system("openssl ...")` which is inherently allocating/blocking. The D
  @nogc port drops the function entirely. In Run 3, add a clear API note (or a separate
  non-@nogc helper in a utility module) to make certificate generation explicit.

- **io/base/socket/socket.cppm** — "ALPN wire-format buffer is fixed 512 bytes"
  C++ used `std::vector<unsigned char>` (unbounded). D port uses `ubyte[512]`. ALPN wire
  format for a typical h2+http/1.1 pair is ~16 bytes; 512 is safe but the overflow is
  currently a silent `error()` log. In Run 3, return a `Result` or assert the limit more
  visibly.

- **io/base/socket/posix.cppm** — "ioctlsocket shim is a stub"
  `ioctlsocket` always returns 0. The POSIX caller should call `ioctl` directly. In Run 2,
  remove the stub and update socket.d to call `ioctl(FIONREAD, ...)` directly in
  `get_pending_bytes()`.

- **io/base/socket/socket.cppm** — "join() multicast not implemented"
  The C++ multicast join used `std::to_string(port)` (allocating) and several
  `getaddrinfo` calls. The D port stubs it with `fatal()`. In Run 3, implement using
  a stack-allocated port string (already done for the main constructor) and the existing
  `getaddrinfo` pattern.

---

# Improvement Ideas (Run 1 — io/layer/http2 + io/layer/shared + congelado/io/service)

Items from the write-only pass. None implemented.

- **io/layer/http2/settings.cppm** — "ReadSettingsAdaptor/WriteSettingsAdaptor as range pipelines"
  C++ used C++26 `range_adaptor_closure` + `std::views::chunk` + `fold_left`. D port uses
  plain slice-reading/writing functions. In Run 3, wrap these with an optional range-adapter
  facade (using `InputRange` protocol) if call sites want pipeline composition.

- **io/layer/http2/frame.cppm** — "WriteFrameClosureAdapter multi-chunk split is O(n²)"
  The D `write_frame_builder` loops over chunks with `~=` (realloc per append). In Run 3,
  replace with a pre-sized BufferNode + a single pass using known total size to avoid
  repeated reallocations.

- **io/layer/http2/stream.cppm** — "ConnectionStream.copy_reader_bytes is O(n) per call"
  The helper drains the BufferReader byte-by-byte via `front()`/`consume()`. In Run 3,
  add a `BufferReader.read_into(ubyte[], size_t)` bulk helper that advances across nodes
  in a single pass.

- **io/layer/http2/stream.cppm** — "DataStream.handle_header HPACK decode stubbed"
  The C++ wired `Hpack<Protocol>.decode(view)` from io.codec.hpack. The D port has a
  TODO stub. In Run 3, connect `HpackEncoder.decode()` using the existing D HPACK tables
  and wire decoding errors back to `Http2ErrorCode.COMPRESSION_ERROR`.

- **io/layer/http2/session.cppm** — "m_streams linear scan → SwissHashMap"
  C++ used `std::map<uint32_t, unique_ptr<Stream<>>>` for O(log n) lookup. D port uses
  a `DataStream[]` with O(n) linear scan. In Run 3, replace with
  `SwissHashMap!(uint, DataStream*)` for O(1) average lookup.

- **io/layer/http2/session.cppm** — "BufferNode allocation uses GC slice conversion"
  Every `send_frame`/`send` call builds a `ubyte[]` then wraps it with `new BufferNode(wire)`.
  This doubles the allocation. In Run 3, write directly into a pre-sized `BufferNode` using
  `get_data()`/`expand_written()` instead of the intermediate slice.

- **io/layer/http2/handshake.cppm** — "Preface comparison limited to first contiguous chunk"
  The D `is_valid_preface` only compares against the first contiguous chunk from `front()`.
  If the 24-byte preface spans two BufferNode boundaries the comparison silently fails.
  In Run 3, implement a proper multi-node compare loop (or add `BufferReader.read_into`
  as noted above).

- **io/layer/http2/plugin.cppm** — "Server.build() and dispatch not wired"
  `core.server.RouteBuilder`, `RouteHandler`, and the method→enum mapping are all stubbed
  with TODOs. Wire these once `core/server` is ported in the Core batch.

- **io/layer/http2/req.cppm + res.cppm** — "Custom header SwissHashMap deferred"
  Both `insert_str()` methods are no-ops; custom headers are silently dropped. In Run 3,
  wire `SwissHashMap!(const(char)[], HeaderField*)` for custom header storage, matching
  the C++ behaviour.

- **io/layer/http2/req.cppm** — "find_header() is a stub"
  C++ called `tokenize(name)` → static slot or hashmap lookup. D port always returns `[]`.
  In Run 3, implement tokenize and route to `m_static_headers` or the custom map.

- **congelado/io/service.cppm** — "CRTP pattern not enforceable without templates"
  C++ CRTP used `static_cast<Derived*>(this)`. D uses `cast(Derived) this`, which is
  a runtime downcast. In Run 3, add `static assert(is(Derived : IoServiceBase!Derived))`
  to the class template body to restore compile-time CRTP verification.

- **congelado/io/service.cppm** — "OpenFlags operator| / operator& as module-level fns"
  C++ defined these as `constexpr` friend operators on the enum. D has no in-enum operators;
  the D port adds two `opOr`/`opAnd` module-level functions. In Run 3, use D's `opBinary`
  template to restore the natural `a | b` syntax for call sites.

---

# Improvement Ideas (Run 1 — core/contracts + core/config + core/logger + core/ffi + core/heart + core/server + core/manager + core/client)

Items from the write-only pass over the 26 core files. None implemented.

- **core/contracts/types.cppm** — "ContractState bitwise operators as opBinary template"
  The D port uses module-level `opNot`/`opOr`/`opAnd` free functions instead of operator
  overloads. In Run 3, add `ContractState.opBinary!"&"` etc. via an `opBinary` template on
  the enum to restore `a & b` infix syntax that the C++ `constexpr` operators provided.

- **core/contracts/contract.cppm** — "AutoEraseContract / AutoClearExecuteFlag as scope guards"
  The C++ RAII guard structs are reproduced with D `scope(exit)` blocks. In Run 3, factor
  these into named `AutoEraseContract` and `AutoClearExecuteFlag` structs (value types) so
  each guard is visible and testable independently, matching the C++ intent.

- **core/contracts/contract.cppm** — "ContractThreadPool thread management uses GC threads"
  `core.thread.osthread.Thread` is GC-allocated and not @nogc-safe. In Run 2, wrap the
  thread pool in a `version(D_BetterC)` guard or replace with a POSIX `pthread_create`
  binding to restore @nogc compatibility.

- **core/contracts/signal_tree.cppm** — "NodeRouter/NodeBranch children allocation via make!"
  The D port allocates `NodeBranch[8]` as eight individual `make!NodeBranch()` calls in
  `NodeRouter.this()`. In Run 3, use a `NodeBranch[8]` inline array (value-embed) inside
  `NodeRouter` to avoid 8 separate heap allocations per router node, matching the C++
  `[[no_unique_address]] vector<Node<false>> m_children` optimization intent.

- **core/config/loader.cppm** — "TOML/JSON parsing completely stubbed"
  The D port stubs `parse_toml` and `parse_json` as no-ops that return an empty Config.
  In Run 3, integrate a D TOML library (e.g. `toml-d` from DUB) and a JSON library
  (e.g. `std.json` or `stdx-allocator`-backed parser) to restore full config loading.

- **core/config/types.cppm** — "PluginConfig fields stored as const(char)[] slices, not owned strings"
  C++ used `std::string` (owning). D port uses `const(char)[]` borrowed views for @nogc.
  In Run 3, decide whether PluginConfig should own copies (via a bump allocator) or borrow
  from a config arena, and document the lifetime contract clearly.

- **core/logger/logger.cppm** — "Variadic format overloads use snprintf with no args passed"
  The template `debug_(name, fmt, args...)` calls `snprintf(buf, fmt.ptr)` but does NOT
  forward `args` to snprintf (C varargs are unsafe from D). In Run 2, replace with a
  D-native format path (e.g. `formattedWrite` into a FixedAppender) to actually expand
  format placeholders, or accept pre-formatted strings only and remove the template
  overloads entirely.

- **core/logger/registry.cppm** — "loggers __gshared dynamic array is not @nogc"
  `__gshared ILogger[] loggers` uses GC slice append (`~=`). In Run 2, switch to a
  fixed-size `ILogger[16]` array + length counter, or use `util.alloc` for @nogc growth.

- **core/ffi/bridge.cppm** — "libffi Closure replaced by plain C function pointer + void*"
  C++ used libffi closures for dynamic thunk generation. The D port replaces them with two
  static `extern(C)` functions (`log_thunk`, `schedule_thunk`). This is simpler and @nogc
  safe. No improvement needed — document the simplification in Run 2.

- **core/ffi/bridge.cppm** — "build_config_view iteration stubbed"
  `build_config_view` does not iterate the PluginConfig's SwissHashMap because the
  iteration API is not yet stable. In Run 2, wire `SwissHashMap.opApply` (or equivalent
  iteration method) to populate `m_cfg_keys` / `m_cfg_vals`.

- **core/ffi/bridge.cppm** — "on_released / on_error return null (weak-ptr semantics lost)"
  C++ `on_released()` captured `weak_from_this()` to guard against use-after-free.
  The D port returns `null` (no-op). In Run 3, implement a reference-counted guard or
  a boolean `m_alive` flag to restore the weak-pointer safety net.

- **core/heart/app.cppm** — "Kahn's dependency sort partially stubbed"
  Phase 3 (full Kahn's algorithm with adjacency list) is reduced to preserving insertion
  order because SwissHashMap iteration is not yet available. In Run 2, wire
  `SwissHashMap.opApply` and complete the topological sort including `load_before_types`
  ordering constraints.

- **core/heart/app.cppm** — "Plugin directory scan uses std::filesystem::exists stub"
  `access(2)` is called for file-existence checks (POSIX only). On Windows this path is
  unimplemented. In Run 2, add a `version(Windows)` branch using `GetFileAttributesA`.

- **core/server/builder.cppm** — "RouterContext.build() sort step is no-op"
  C++ used `std::ranges::sort` to hash-bucket-order routes within each group. The D port
  skips the sort under @nogc. In Run 3, implement a @nogc insertion sort (O(n²) for small
  N is acceptable here — route tables are rarely > 256 entries) to restore deterministic
  hash-probe ordering.

- **core/server/builder.cppm** — "RouterBuildingHelper[][] uses GC slices"
  The `table` dynamic array inside `build()` uses GC-backed `~=`. In Run 2, replace with a
  fixed-size `RouterBuildingHelper[256][16]` stack arena + row-length counters to eliminate
  all allocations in `build()`.

- **core/server/router.cppm** — "split_path callback form breaks at compile time with @nogc"
  The lambda passed to `split_path` captures `current`/`current_index` by reference. Under
  `-betterC` this may require an explicit delegate wrapper. In Run 2, refactor to an
  iterator-style loop (index-based) that avoids the delegate entirely.

- **core/client/client.cppm** — "ClientConcept constraint not enforceable without D concepts"
  C++ used a `ClientConcept` concept to constrain `ClientType`. D uses a `static if`
  template constraint. In Run 2, define a `ClientConcept!T` template boolean that checks
  for `on_send()` returning a callable, to restore compile-time enforcement.

- **core/client/client.cppm** — "IRequest!Protocol constructed directly (no factory)"
  C++ `m_base_request{}` default-constructed the request. The D port calls
  `new IRequest!Protocol()` directly. If `IRequest` is abstract this will fail to compile.
  In Run 2, replace with a protocol-specific concrete request type or a factory function.

---

# Improvement Ideas (Run 1 — engine + worker)

Items from the write-only pass over 14 engine/worker files. None implemented.

- **engine/context.cppm** — "EngineContext pointer-to-connector ABI mismatch"
  C++ Connector is a value member. D port uses `ref Connector get_connector()` returning
  a ref to the embedded member. If Connector is a D class (reference type) the ref is
  redundant. In Run 2, decide class vs struct for Connector and remove the ref if it
  becomes a class.

- **engine/handler/metadata.cppm + task.cppm + workflow.cppm** — "std::reference_wrapper → raw pointer"
  C++ used `std::reference_wrapper<EngineContext> m_ctx` (rebindable reference). D port
  uses `EngineContext* m_ctx`. Both have the same null-risk; the D raw pointer is simpler.
  In Run 2, consider using `ref` parameters where re-binding is not needed to avoid the
  raw pointer entirely.

- **engine/handler/task.cppm** — "flatten_body uses GC ~= append"
  `flatten_body` builds a `ubyte[]` with GC-backed `~=`. In Run 2, replace with a
  fixed-size stack buffer (e.g. `ubyte[65536]`) or a `BufferWriter` node from `util.alloc`
  to eliminate GC pressure on the hot request path.

- **engine/handler/task.cppm + workflow.cppm** — "last_slash / last_slash_before helpers duplicated"
  Both files define the same two private helper functions. In Run 3, factor them into a
  shared `engine.util` module (or `shared.util`) to eliminate the duplication.

- **engine/handler/workflow.cppm** — "WorkflowExecution allocated with make! but never disposed"
  `start_execution` allocates via `make!WorkflowExecution()` but the pointer is captured
  in the insert callback; no `dispose` call exists. In Run 2, decide ownership: either
  move into the callback (store by value) or track disposal explicitly.

- **engine/routes.cppm** — "Handler lifetime not enforced"
  C++ used `std::shared_ptr` to guarantee handler lifetime matched route lifetime. D port
  uses raw pointers with a comment. In Run 2, wire an explicit arena or tie handler lifetime
  to a scope that outlives the router, and add a static assert or documentation contract.

- **worker/config.cppm** — "from_file() is a stub; TOML parsing missing"
  The full C++ TOML parsing (engine_url, worker_id, concurrency, tasks array) is dropped.
  In Run 2, integrate a D TOML library (e.g. `toml-d` from DUB) or the hand-rolled subset
  from `core.config.loader` once that stub is filled in.

- **worker/config.cppm** — "TaskConfig / WorkerConfig fields stored as borrowed const(char)[]"
  C++ used owning `std::string`. D port uses borrowed slices for @nogc. In Run 3, decide
  ownership: either copy into a bump allocator arena or document that config slices must
  outlive all WorkerConfig uses.

- **worker/task_worker.cppm** — "TaskInput get overloads lack double parsing"
  `get_double` stubs the float parser and always returns none. In Run 2, implement a
  minimal @nogc `strtod`-equivalent (or bind libc's `strtod` via extern(C)) to restore
  full numeric TaskInput support.

- **worker/task_worker.cppm** — "ITaskWorker on_released / on_error return fn pointers, not closures"
  C++ returned `std::function<void()>` (owning closure). D port returns bare
  `void function() @nogc nothrow` (no capture). Any ITaskWorker implementation that
  needs captured state must use a global/TLS variable. In Run 3, introduce a `Closure!Fn`
  struct (from `util.closure`) to restore owning-capture semantics without GC.

- **worker/context.cppm** — "run_task swallows errors silently"
  C++ called `on_error(std::current_exception())` on any exception. D is nothrow — the
  port has no exception path, so `on_error` is never invoked. In Run 2, define an explicit
  `ResultCode` return from `execute()` or an out-param error so failures are not silent.

- **worker/context.cppm** — "get_task_types fills caller-supplied slice; length unknown to callers"
  C++ returned a heap vector. D port uses a caller-supplied output slice, requiring callers
  to pre-size. In Run 3, expose a `size_t count_task_types() const` accessor so callers
  can stack-allocate the right size before calling `get_task_types`.

- **worker/context.cppm** — "m_workers uses GC ~= append"
  `add_task_worker` appends with `~=`. In Run 2, replace with a fixed-size
  `ITaskWorker[64]` array + length counter (typical deployments have few task types)
  to eliminate GC on the registration path.

- **worker/handler/execution.cppm + poll.cppm** — "call_engine bodies entirely commented out"
  All three execution routes and both poll routes are stubs — the call_engine integration
  was never wired in C++ either (all bodies are `//`-commented). In Run 2, unblock by
  wiring the EngineClient (once its replacement for the deleted engine_client.cppm is
  designed) and un-commenting the logic.

- **worker/handler/poll.cppm** — "build_submit_json returns empty slice"
  The JSON builder is a stub. In Run 2, implement @nogc JSON serialisation for
  `{result: "...", output_data: {...}}` using a stack buffer + a simple write loop,
  matching the C++ `std::format` output.

- **worker/handler/status.cppm** — "health_check body length assignment uses GC"
  `bytes.length = k_ok.length` triggers a GC allocation. In Run 2, replace with a
  `ubyte[32]` stack buffer and a manual copy loop (k_ok is only 16 bytes).

---

# Improvement Ideas (Run 1 — model + serde + connector + SDK + top-level)

Items from the write-only pass over the final 28 files. None implemented.

- **model/common/identifiers.cppm** — "generate_id() uses getrandom(2) directly"
  C++ used uuids::uuid_system_generator (stduuid library). D port calls getrandom(2)
  directly and sets RFC 4122 version/variant bits. In Run 2, verify this produces
  valid v4 UUIDs and add a unit test for the nil-detection path.

- **model/common/identifiers.cppm** — "Uuid as 16-byte struct vs deimos binding"
  stduuid is a C++ header library with no D equivalent. The Uuid struct is a plain
  16-byte value type. In Run 3, consider adding a to_string(Uuid) helper that formats
  the canonical xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx form using a stack char[36].

- **model/common/timestamps.cppm** — "ExecutionTimings uses Optional!long for timestamps"
  C++ used std::optional<std::chrono::system_clock::time_point>. D uses Optional!long
  (Unix epoch ms). In Run 2, verify that all callers consistently pass epoch-ms and
  document the epoch-ms contract at the module level.

- **model/task/definition.cppm + instance.cppm** — "Fixed-size arrays may be too small"
  TaskDef uses const(char)[][32] for input/output keys; TaskInstance uses KvEntry[64]
  for data maps. Real workflows may have more fields. In Run 3, replace with a
  bump-allocator backed slice or raise the caps to 128 with a compile-time assert.

- **model/workflow/exec.cppm** — "WorkflowExecution.m_task_instances fixed at 128"
  Complex workflows can have hundreds of task steps. In Run 3, raise the cap to 512
  or introduce a heap-allocated buffer via util.alloc for large workflow executions.

- **serde/core.cppm** — "Serializable!T specialization protocol not enforced"
  D has no partial specialization syntax like C++. The current approach requires each
  model class to manually define a Serializable!T block. In Run 3, add a compile-time
  check via static assert(is_serializable!T) at registration sites to catch missing
  specializations early.

- **serde/converter.cppm** — "FieldConverter machinery is almost entirely stubbed"
  The C++ FieldConverter<VT> did JSON/TOML encoding via rfl.hpp and simdjson.
  D port stubs all methods. In Run 2, wire mir-ion (or a minimal hand-rolled JSON
  encoder) for the concrete specializations: Uuid, long, const(char)[], bool, enums.

- **serde/json.cppm** — "Json.encode / decode entirely stubbed"
  All JSON encode/decode returns empty strings or error. In Run 2, integrate a
  @nogc-compatible JSON library (mir-ion recommended) and implement
  encode/decode over Serializable!T.fields() iteration.

- **serde/toml.cppm** — "Toml.encode / decode entirely stubbed"
  TOML encode/decode returns empty/error. In Run 2, integrate a D TOML library
  (toml-d from DUB or a hand-rolled subset matching toml++ semantics) and wire
  ModelToml.from_toml() as the primary config loading path.

- **serde/sql.cppm** — "All Sql.build_*_sql methods stubbed"
  C++ built SQL strings via std::format + FieldDesc iteration. D stubs all builders.
  In Run 2, implement using snprintf into stack buffers + Serializable!T.fields()
  iteration once field iteration is available.

- **serde/cache.cppm** — "Cache.pk_string / cache_key stubbed"
  Both methods return empty strings. In Run 2, wire Serializable!T.fields() iteration
  to find the primary_key=true field and format "table:pk_value" into a stack buffer.

- **connector/connector.cppm** — "Pending operation queue is never drained"
  The m_pending_buf circular buffer is allocated but on_execute() returns a null
  WorkerFunction stub. In Run 2, implement the queue draining loop in on_execute()
  using the existing PendingOp fn+ctx pattern.

- **connector/connector.cppm** — "Per-type local stores not implemented"
  C++ used unordered_map<type_index, any> to store per-type T→T maps. D port defers
  this to Run 2. The simplest @nogc approach is a fixed array of opaque store pointers
  with a runtime type tag (e.g. a const(char)[] type name string from T.stringof).

- **connector/local_cache.cppm** — "Linear scan O(n) lookup"
  LocalCache uses a StoreEntry[256] linear array. In Run 3, replace with
  SwissHashMap!(const(char)[], const(char)[]) for O(1) average get/set/remove.

- **sdk/worker/congelado_worker.cppm** — "TaskRegistry.get_all fills caller slice"
  C++ returned std::vector<ITask*>. D fills a caller-supplied ITask[] slice. In Run 3,
  expose a size_t count_tasks() accessor so callers can stack-allocate the right size.

- **sdk/plugin/congelado_plugin.cppm** — "Plugin.get_requires / get_load_before_types return const(char)[][]"
  C++ returned std::span<const std::string_view>. D returns const(const(char)[])[].
  The slice must remain valid for the plugin's lifetime. In Run 2, add documentation
  that implementations must return slices over static arrays (string literals only).

- **sdk/plugin/plugin.d** — "CongeladoPlugin mixin __gshared static caches"
  The s_cache arrays in congelado_requires / congelado_load_before_types use
  __gshared bool s_built flags. This is not thread-safe at load time. In Run 2,
  gate on a pthread_once or D's shared static this() to ensure single init.

- **defaults/plugins/http2/http2.cc** — "Http2Plugin imports reference unfinished modules"
  http2.d imports io.layer.http2, core_.server.builder, core_.contract.contract, etc.
  Several of these are themselves partially stubbed. In Run 2, audit which protocol
  paths are live and stub/comment the remainder to prevent link failures.

- **include/congelado.cppm** — "Umbrella module re-exports three sub-trees"
  congelado.d re-exports interfaces, shared_, and core_.ffi. In Run 2, verify that
  all three sub-modules compile clean before enabling the umbrella import at app sites.


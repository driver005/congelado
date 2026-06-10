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

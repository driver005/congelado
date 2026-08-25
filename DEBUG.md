# Debugging memory leaks

The debug build links AddressSanitizer + LeakSanitizer. A leak is reported at
process exit — so the process must **exit cleanly** for LSan to run (a hung
shutdown means LSan never fires). SIGINT/SIGTERM the app and let it tear down.

## 1. Read the LSan report

```
==NNN==ERROR: LeakSanitizer: detected memory leaks
Direct leak of 128 byte(s) in 2 object(s) allocated from:
    #0 operator new(...)
    #1 utils::buffering::BufferWriter::acquire() ./include/utils/buffering/writter.cppm:67
    #2 io::base::flow::sync::Receiver::arm_read() ./include/io/flow/receiver/sync.cppm:260
    ...
Indirect leak of 16384 byte(s) ...
SUMMARY: AddressSanitizer: 16512 byte(s) leaked in 4 allocation(s).
```

- **Direct leak** = the block is unreachable at exit (nothing points to it) and
  was never freed.
- **Indirect leak** = only reachable through another leaked block (e.g. a buffer
  owned by a leaked wrapper).
- The stack is the **allocation** site — *where it was `new`'d*, not where the
  missing free is. For a plain owner, that's enough. For **ref-counted** objects
  it is not: the allocation is fine; some `acquire()` just never got its
  matching `release()`. Go to step 2.

Symbolized frames need `llvm-symbolizer` on `PATH` (or set
`ASAN_SYMBOLIZER_PATH`), else frames show as raw addresses.

## 2. Ref-counted leaks: trace acquire/release

Our buffer chain (`utils::buffering`) uses intrusive refcounts — see
`include/utils/buffering/README.md` for the ownership map. When one of these
leaks, instrument the refcount to find the **unbalanced call site**.

Temporarily edit the refcount primitive (e.g. `BufferNode::acquire/release`,
`include/utils/buffering/node.cppm`) to print the object identity + a stack:

```cpp
// in the module; global fragment, before `export module ...;`
module;
#include <sanitizer/common_interface_defs.h>   // __sanitizer_print_stack_trace

// ...
void acquire() noexcept {
    auto count = m_refs.fetch_add(1, std::memory_order_relaxed) + 1;
    std::println(stderr, "ACQ {} ref={}", static_cast<const void *>(this), count);
    __sanitizer_print_stack_trace();
}
void release() noexcept {
    auto count = m_refs.fetch_sub(1, std::memory_order_acq_rel) - 1;
    std::println(stderr, "REL {} ref={}", static_cast<const void *>(this), count);
    __sanitizer_print_stack_trace();
    if (count == 0) { delete this; }
}
```

The object pointer is a stable per-object id for its lifetime. Rebuild fully
(`xmake -r` — a `module;` include change needs it) and capture stderr:

```bash
xmake -r && xmake run 2>/tmp/refs.log     # exercise the path, then Ctrl+C
```

## 3. Find the leaked object

Net every `ACQ` (+1) / `REL` (-1) per pointer; a nonzero net is a leak:

```bash
awk '/^ACQ /{c[$2]++} /^REL /{c[$2]--} END{for(k in c) if(c[k]) print "LEAKED",k,c[k]}' /tmp/refs.log
# LEAKED 0x7c41447ee5a0 net 1
```

`net 1` = one acquire with no release.

## 4. Find the missing-release call site

Dump that pointer's `ACQ`/`REL` frames and tally the **real caller** (skip the
sanitizer frame `#0` and the `acquire`/`release` frame itself). The caller whose
`ACQ` count exceeds its `REL` count is the bug:

```bash
p=0x7c41447ee5a0
awk -v P="$p" '
  $0 ~ ("^(ACQ|REL) " P " ref=") { kind=$1; want=1; next }
  want==1 && (/node\.cppm/ || /__sanitizer_print_stack_trace/) { next }   # skip self + sanitizer
  want==1 && /#[0-9]+ 0x/ {
    site=$0; sub(/^ *#[0-9]+ 0x[0-9a-f]+ in /,"",site); sub(/ \(BuildId.*/,"",site);
    c[kind" | "site]++; want=0
  }
  END { for (k in c) printf "%4d  %s\n", c[k], k }
' /tmp/refs.log | sort
```

Example output that pinned the real bug:

```
  97  ACQ | NodeView::acquire() view.cppm:88     <- called from BufferView::push_back
  96  REL | NodeView::release() view.cppm:93
```

96 balanced pairs (iteration) + **1 extra acquire** → `BufferView::push_back`
acquired a ref the ctor already held, and nothing released it. Fix: remove the
double-acquire (the ctor's stake is the chain's ref).

## 5. Clean up

Remove **all** instrumentation before committing: the `module;` sanitizer
include, the `ACQ`/`REL` prints, and any scratch `std::println`. Verify:

```bash
grep -rn "__sanitizer\|std::println(stderr, \"ACQ\|std::println(stderr, \"REL" include/
```

## Notes / gotchas

- LSan only runs on a clean exit. If shutdown hangs, fix that first (a stuck
  drain/join loop, a thread that never joins).
- A **direct** leak that is genuinely unreachable but not freed almost always
  means a refcount that never hit 0 — an unmatched `acquire()`. Reach for
  step 2 rather than staring at the allocation stack.
- Object pointers can be reused after free; for a short single-request run this
  is not a problem. For long runs, swap the pointer for a monotonic per-object
  id (a `static std::atomic<uint64_t>` counter set in the ctor).
- For non-refcounted leaks, the LSan allocation stack usually points straight at
  a missing `delete` / an owner that outlived its scope — no instrumentation
  needed.

```

See `include/utils/buffering/README.md` for the buffer subsystem's acquire /
release / new / delete ledger — the reference for what *should* balance.

# `utils::buffering` — ownership & reference-counting map

This subsystem moves bytes through chains of heap `BufferNode`s without copying. Two
*separate* ownership concerns run in parallel here — mixing them up is how the leak in this
folder happened, so they are documented explicitly.

## The two ownership axes

1. **`BufferNode` intrusive refcount (`m_refs`)** — governs the *byte buffer's* lifetime.
   Any holder that needs the bytes to stay alive takes one ref. When it hits zero the
   `BufferNode` does `delete this` (`node.cppm:196`). Shared between `NodeReader`s,
   `NodeView`s, and iterators.

2. **Wrapper object lifetime (`NodeReader` / `NodeView`)** — the per-chain-slot heap object
   that links a `BufferNode` into a chain. Owned by its chain, freed with an explicit
   `delete` on unlink. This is **not** refcounted — the chain is the sole owner.

A wrapper holds exactly **one** `BufferNode` ref for its whole life: acquired in its ctor,
released in its dtor. So deleting a wrapper (running its dtor) *is* how the chain drops its
stake in the bytes. Do **not** also call `release()` before deleting — that double-drops.

---

## `BufferNode::m_refs` — acquire / release sites

`NodeReader::acquire()/release()` and `NodeView::acquire()/release()` just forward to the
wrapped `BufferNode` (`reader.cppm:92/97`, `view.cppm:84/89`).

| # | Acquire site | File:line | Balanced by |
|---|---|---|---|
| A | `NodeReader` ctor (the chain's stake) | `reader.cppm:20` | `NodeReader` dtor `reader.cppm:26` (runs on `delete`) |
| B | writer slot, new-node branch | `writter.cppm:69` | `notify_read`/`release` (row R1/R2) |
| C | writer slot, reuse-tail branch | `writter.cppm:75` | `notify_read`/`release` (row R1/R2) |
| D | `BufferReader::Iterator` ctor / node-hop | `reader.cppm:158,262,304` | Iterator `++`/`+=`/dtor (row R3) |
| E | `BufferReader::Iterator` copy | `reader.cppm:178,194` | Iterator copy-assign / dtor |
| F | `BufferReader::peek()` (hands caller an extra ref) | `reader.cppm:426` | caller must `release()` |
| G | `NodeView` ctor (the view's stake) | `view.cppm:20` | `NodeView` dtor `view.cppm:26` (runs on `delete`) |
| H | `BufferView::push_back` / iterator | `view.cppm:408,174,190` | `NodeView` dtor / iterator |

| # | Release site | File:line | Balances |
|---|---|---|---|
| R1 | `BufferWriter::notify_read` | `writter.cppm:116` | B / C (bytes landed) |
| R2 | `BufferWriter::release(slot)` | `writter.cppm:139` | B / C (would-block / error, no bytes) |
| R3 | `BufferReader::Iterator` `++`/`+=`/copy-assign | `reader.cppm:265,307,201` | D / E |
| R4 | `NodeReader` dtor | `reader.cppm:26` | A — fires only via `delete` (below) |
| R5 | `NodeView` dtor | `view.cppm:26` | G — fires only via `delete` (below) |
| R6 | `BufferNode::release` → `delete this` at 0 | `node.cppm:196` | frees the buffer |

---

## Wrapper object lifetime — `new` / `delete` sites

| Wrapper | Allocated | Freed (unlink) |
|---|---|---|
| `NodeReader` | `BufferReader::push_back(BufferNode*)` `reader.cppm:542` | `consume` `reader.cppm:496`, `~BufferReader→release` `reader.cppm:617` |
| `NodeView` | `BufferView::push_back(BufferNode*,…)` `view.cppm:434` | `BufferView::release` `view.cppm:454` |

`BufferReader::push_back(NodeReader*)` does **not** acquire — a fresh `NodeReader` already
holds the chain's one ref via its ctor (row A). Bumping again would pin the `BufferNode`
forever after unlink (that was the leak).

---

## Net accounting per path (BufferNode refs → 0)

| Path | ctor (A) | writer slot (B/C) | notify_read (R1) | `delete` wrapper → dtor (R4) | net |
|---|---|---|---|---|---|
| **read** (writer `acquire` → `notify_read` → `consume`) | +1 | +1 | −1 | −1 | **0** |
| **send** (`BufferWriter::push` → `consume`) | +1 | — | — | −1 | **0** |
| **grow_view** (pure view; caller `consume`s after) | — | — | — | — | grow_view only adds NodeView refs (G/H); the following `consume` frees the reader's NodeReaders |

Iterators hold their own `BufferNode` ref (D/E ↔ R3), so bytes survive a `consume` that
deletes the wrapper — as long as no iterator outlives the `consume` of the node it points
*into* (single-consumer contract; parsers copy out or build a `BufferView`, then consume).

---

## Invariants

- One `BufferNode` ref per wrapper, tied to wrapper ctor/dtor. The wrapper's `delete` is the
  only thing that drops it — never pair `release()` with `delete` on the same wrapper.
- The chain owns its wrappers; unlink means `delete`, mirroring `BufferView::release`.
- Every `BufferWriter::acquire()` hands back a slot with **one** extra ref for the caller;
  pair it with exactly one `notify_read()` **or** `release()`.
- `consume(n)` with `n > size()` underflows `m_size` (unsigned) — do not over-consume
  (see the `@warning` on `BufferReader::consume`).

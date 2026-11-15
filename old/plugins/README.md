# Plugin Contract ownership

If your plugin's backend does blocking work — a network round-trip, disk I/O, anything that
isn't effectively instant — it owns its own `Contract`, not the host and not `connector::
Connector`. Register it yourself, in your own `on_load`:

```cpp
auto *contract_group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
auto *contract_registry = congelado::registry_ctx<core::contract::ContractRegistry>(host);
if (contract_group != nullptr && contract_registry != nullptr) {
    auto contract = create(*contract_group, core::contract::ContractState::IDLE);
    // wire your own wake-on-enqueue however your queue needs it, then:
    contract_registry->add(std::move(contract));
}
```

**Exception:** a backend whose lookup is O(1)/in-process, with nothing to wait on (e.g.
`cache/local` — a plain `unordered_map`), doesn't need one. There's nothing async to drain.

**Reference implementations:**
- `database/postgres` (`src/db_queue.cppm`) — owns the Contract draining `connector::Connector`'s
  own DB-op queue (`m_pending`), since it's the one real `IDatabase` backend.
- `cache/redis` (`bin/redis.cc`) — owns a private queue + Contract of its own, since hiredis
  round-trips are blocking network I/O; `connector::Connector` has no cache queue of its own to
  hand off, so redis drains its own.
- `cron/local` (`bin/local_cron.cc`) — the original pattern this follows, for its tick sweep.

**Gotcha:** `import connector;` crashes clang's modules in a plugin's entry TU (`bin/*.cc`, the
one with `CONGELADO_PLUGIN(...)`). If your Contract needs to touch `connector::Connector`
directly, put that code in its own `src/*.cppm` module and call into it from `bin/*.cc` instead —
see `database/postgres/src/db_queue.cppm`.

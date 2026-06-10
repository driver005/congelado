# Port Status: congelado C++ → D

## Summary

congelado is a C++26 module-based async HTTP/2 server framework. It is structured
as a deep module hierarchy: leaf utilities and stdlib wrappers at the bottom, IO
primitives (sockets, io_uring, TLS) in the middle, HTTP/2 codec and session layers
above that, and a core application heart (plugin manager, worker pool, engine) at
the top. The SDK exposes C headers for dynamically loaded worker and plugin shared
libraries. The port targets D (LDC compiler) with -betterC/@nogc enforcement where
feasible. Strategy is a strict one-to-one structural mapping — every .cppm module
becomes a .d module at the same logical path — followed by a compilation-clean pass,
then an improvement pass after the user reviews a proposed delta list. Total scope:
approximately 236 source units spanning C++26 named modules, two C SDK headers, one
.cc plugin implementation, and one src/ service file.

---

## Master Checklist

- [x] Run 0: D project skeleton under one_to_one/ builds an empty hello-target
- [x] Run 0: D_STYLE_GUIDE.md written, derived from .clang-format + observed C++ style
- [x] Run 0: Full file inventory enumerated, per-file checklist generated
- [ ] Run 1: All files converted (write-only pass, no verification)
- [ ] Run 2: Project compiles clean with LDC, -betterC/@nogc enforced
- [ ] Run 2: Syntax/semantics sanity pass complete (read-through, no build errors)
- [ ] Run 3: IMPROVEMENTS.md finalized and presented to user for selection
- [ ] Run 3: User-selected improvements implemented

---

## Per-File Conversion Checklist (Run 1)

### Leaf utilities
- [x] include/utils/consts.cppm               → one_to_one/src/utils/consts.d
- [x] include/utils/encode.cppm               → one_to_one/src/utils/encode.d
- [x] include/utils/helper.cppm               → one_to_one/src/utils/helper.d
- [x] include/utils/codec/atom.cppm           → one_to_one/src/utils/codec/atom.d
- [x] include/utils/codec/codec.cppm          → one_to_one/src/utils/codec/codec.d
- [x] include/utils/buffering/node.cppm       → one_to_one/src/utils/buffering/node.d
- [x] include/utils/buffering/base.cppm       → one_to_one/src/utils/buffering/base.d
- [x] include/utils/buffering/deleter.cppm    → one_to_one/src/utils/buffering/deleter.d
- [x] include/utils/buffering/reader.cppm     → one_to_one/src/utils/buffering/reader.d
- [x] include/utils/buffering/view.cppm       → one_to_one/src/utils/buffering/view.d
- [x] include/utils/buffering/writter.cppm    → one_to_one/src/utils/buffering/writter.d
- [x] include/utils/hashmap/base.cppm         → one_to_one/src/utils/hashmap/base.d
- [x] include/utils/hashmap/swiss.cppm        → one_to_one/src/utils/hashmap/swiss.d
- [x] include/utils/queue/node.cppm           → one_to_one/src/utils/queue/node.d
- [x] include/utils/queue/page.cppm           → one_to_one/src/utils/queue/page.d
- [x] include/utils/queue/pager.cppm          → one_to_one/src/utils/queue/pager.d
- [x] include/utils/queue/atomic_list.cppm    → one_to_one/src/utils/queue/atomic_list.d
- [x] include/utils/queue/queue.cppm          → one_to_one/src/utils/queue/queue.d

### Modules (C stdlib wrappers)
- [x] include/modules/asio.cppm               → one_to_one/src/modules/asio.d
- [x] include/modules/errno.cppm              → one_to_one/src/modules/errno.d
- [x] include/modules/fcntl.cppm              → one_to_one/src/modules/fcntl.d
- [x] include/modules/net.cppm                → one_to_one/src/modules/net.d
- [x] include/modules/netdb.cppm              → one_to_one/src/modules/netdb.d
- [x] include/modules/openssl.cppm            → one_to_one/src/modules/openssl.d
- [x] include/modules/socket.cppm             → one_to_one/src/modules/socket.d
- [x] include/modules/unistd.cppm             → one_to_one/src/modules/unistd.d
- [x] include/modules/winsock2.cppm           → one_to_one/src/modules/winsock2.d

### Shared umbrella modules
- [x] include/shared/types.cppm               → one_to_one/src/shared/types.d
- [x] include/shared/logger.cppm              → one_to_one/src/shared/logger.d
- [x] include/shared/socket.cppm              → one_to_one/src/shared/socket.d
- [x] include/shared/leverage.cppm            → one_to_one/src/shared/leverage.d
- [x] include/shared/transport.cppm           → one_to_one/src/shared/transport.d
- [x] include/shared/flow.cppm                → one_to_one/src/shared/flow.d
- [x] include/shared/handler.cppm             → one_to_one/src/shared/handler.d
- [x] include/shared/shared.cppm              → one_to_one/src/shared/shared.d

### Interfaces
- [x] include/interfaces/status.cppm          → one_to_one/src/interfaces/status.d
- [x] include/interfaces/logger.cppm          → one_to_one/src/interfaces/logger.d
- [x] include/interfaces/io.cppm              → one_to_one/src/interfaces/io.d
- [x] include/interfaces/request.cppm         → one_to_one/src/interfaces/request.d
- [x] include/interfaces/response.cppm        → one_to_one/src/interfaces/response.d
- [x] include/interfaces/cache.cppm           → one_to_one/src/interfaces/cache.d
- [x] include/interfaces/database.cppm        → one_to_one/src/interfaces/database.d
- [x] include/interfaces/client.cppm          → one_to_one/src/interfaces/client.d
- [x] include/interfaces/protocol.cppm        → one_to_one/src/interfaces/protocol.d
- [x] include/interfaces/codec/cache.cppm     → one_to_one/src/interfaces/codec/cache.d
- [x] include/interfaces/codec/db.cppm        → one_to_one/src/interfaces/codec/db.d
- [x] include/interfaces/interfaces.cppm      → one_to_one/src/interfaces/interfaces.d

### IO shared + codec shared
- [x] include/io/shared/consts.cppm           → one_to_one/src/io/shared/consts.d
- [x] include/io/shared/types.cppm            → one_to_one/src/io/shared/types.d
- [x] include/io/shared/shared.cppm           → one_to_one/src/io/shared/shared.d
- [x] include/io/shared/http/types.cppm       → one_to_one/src/io/shared/http/types.d
- [x] include/io/shared/http/header.cppm      → one_to_one/src/io/shared/http/header.d
- [x] include/io/shared/http/http.cppm        → one_to_one/src/io/shared/http/http.d
- [x] include/io/codec/shared/consts.cppm     → one_to_one/src/io/codec/shared/consts.d
- [x] include/io/codec/shared/types.cppm      → one_to_one/src/io/codec/shared/types.d
- [x] include/io/codec/shared/atom.cppm       → one_to_one/src/io/codec/shared/atom.d
- [x] include/io/codec/shared/huffman.cppm    → one_to_one/src/io/codec/shared/huffman.d
- [x] include/io/codec/shared/lowlevel.cppm   → one_to_one/src/io/codec/shared/lowlevel.d
- [x] include/io/codec/shared/table.cppm      → one_to_one/src/io/codec/shared/table.d
- [x] include/io/codec/shared/shared.cppm     → one_to_one/src/io/codec/shared/shared.d

### IO codec: HPACK
- [x] include/io/codec/hpack/consts.cppm      → one_to_one/src/io/codec/hpack/consts.d
- [x] include/io/codec/hpack/types.cppm       → one_to_one/src/io/codec/hpack/types.d
- [x] include/io/codec/hpack/table.cppm       → one_to_one/src/io/codec/hpack/table.d
- [x] include/io/codec/hpack/hpack.cppm       → one_to_one/src/io/codec/hpack/hpack.d

### IO codec: QPACK
- [x] include/io/codec/qpack/consts.cppm      → one_to_one/src/io/codec/qpack/consts.d
- [x] include/io/codec/qpack/types.cppm       → one_to_one/src/io/codec/qpack/types.d
- [x] include/io/codec/qpack/table.cppm       → one_to_one/src/io/codec/qpack/table.d
- [x] include/io/codec/qpack/qpack.cppm       → one_to_one/src/io/codec/qpack/qpack.d

### IO codec: QUIC
- [x] include/io/codec/quic/types.cppm        → one_to_one/src/io/codec/quic/types.d
- [x] include/io/codec/quic/crypto.cppm       → one_to_one/src/io/codec/quic/crypto.d
- [x] include/io/codec/quic/tls.cppm          → one_to_one/src/io/codec/quic/tls.d
- [x] include/io/codec/quic/connection.cppm   → one_to_one/src/io/codec/quic/connection.d
- [x] include/io/codec/quic/quic.cppm         → one_to_one/src/io/codec/quic/quic.d

### IO errors
- [x] include/io/error/base.cppm              → one_to_one/src/io/error/base.d
- [x] include/io/error/http.cppm              → one_to_one/src/io/error/http.d
- [x] include/io/error/error.cppm             → one_to_one/src/io/error/error.d

### IO base: leverage (io_uring / posix / win32)
- [x] include/io/base/leverage/types.cppm     → one_to_one/src/io/base/leverage/types.d
- [x] include/io/base/leverage/posix.cppm     → one_to_one/src/io/base/leverage/posix.d
- [x] include/io/base/leverage/uring.cppm     → one_to_one/src/io/base/leverage/uring.d
- [x] include/io/base/leverage/win32.cppm     → one_to_one/src/io/base/leverage/win32.d
- [x] include/io/base/leverage/base.cppm      → one_to_one/src/io/base/leverage/base.d

### IO base: socket
- [x] include/io/base/socket/consts.cppm      → one_to_one/src/io/base/socket/consts.d
- [x] include/io/base/socket/posix.cppm       → one_to_one/src/io/base/socket/posix.d
- [x] include/io/base/socket/win32.cppm       → one_to_one/src/io/base/socket/win32.d
- [x] include/io/base/socket/socket.cppm      → one_to_one/src/io/base/socket/socket.d

### IO flow
- [x] include/io/flow/receiver/sync.cppm      → one_to_one/src/io/flow/receiver/sync.d
- [x] include/io/flow/receiver/async.cppm     → one_to_one/src/io/flow/receiver/async.d
- [x] include/io/flow/receiver/reveiver.cppm  → one_to_one/src/io/flow/receiver/reveiver.d
- [x] include/io/flow/sender/sync.cppm        → one_to_one/src/io/flow/sender/sync.d
- [x] include/io/flow/sender/async.cppm       → one_to_one/src/io/flow/sender/async.d
- [x] include/io/flow/sender/sender.cppm      → one_to_one/src/io/flow/sender/sender.d
- [x] include/io/flow/socket/sync.cppm        → one_to_one/src/io/flow/socket/sync.d
- [x] include/io/flow/socket/async.cppm       → one_to_one/src/io/flow/socket/async.d
- [x] include/io/flow/socket/socket.cppm      → one_to_one/src/io/flow/socket/socket.d
- [x] include/io/flow/flow.cppm               → one_to_one/src/io/flow/flow.d

### IO layer: shared
- [x] include/io/layer/shared/types.cppm      → one_to_one/src/io/layer/shared/types.d
- [x] include/io/layer/shared/ping.cppm       → one_to_one/src/io/layer/shared/ping.d
- [x] include/io/layer/shared/codec.cppm      → one_to_one/src/io/layer/shared/codec.d
- [x] include/io/layer/shared/shared.cppm     → one_to_one/src/io/layer/shared/shared.d

### IO layer: HTTP/2
- [x] include/io/layer/http2/consts.cppm      → one_to_one/src/io/layer/http2/consts.d
- [x] include/io/layer/http2/settings.cppm    → one_to_one/src/io/layer/http2/settings.d
- [x] include/io/layer/http2/frame.cppm       → one_to_one/src/io/layer/http2/frame.d
- [x] include/io/layer/http2/helper.cppm      → one_to_one/src/io/layer/http2/helper.d
- [x] include/io/layer/http2/stream.cppm      → one_to_one/src/io/layer/http2/stream.d
- [x] include/io/layer/http2/req.cppm         → one_to_one/src/io/layer/http2/req.d
- [x] include/io/layer/http2/res.cppm         → one_to_one/src/io/layer/http2/res.d
- [x] include/io/layer/http2/handshake.cppm   → one_to_one/src/io/layer/http2/handshake.d
- [x] include/io/layer/http2/flow.cppm        → one_to_one/src/io/layer/http2/flow.d
- [x] include/io/layer/http2/session.cppm     → one_to_one/src/io/layer/http2/session.d
- [x] include/io/layer/http2/plugin.cppm      → one_to_one/src/io/layer/http2/plugin.d
- [x] include/io/layer/http2/http2.cppm       → one_to_one/src/io/layer/http2/http2.d

### IO: src/congelado
- [x] src/congelado/io/service.cppm           → one_to_one/src/congelado/io/service.d

### Core
- [ ] include/core/contracts/types.cppm       → one_to_one/src/core/contracts/types.d
- [ ] include/core/contracts/consts.cppm      → one_to_one/src/core/contracts/consts.d
- [ ] include/core/contracts/contract.cppm    → one_to_one/src/core/contracts/contract.d
- [ ] include/core/contracts/signal_tree.cppm → one_to_one/src/core/contracts/signal_tree.d
- [ ] include/core/config/types.cppm          → one_to_one/src/core/config/types.d
- [ ] include/core/config/loader.cppm         → one_to_one/src/core/config/loader.d
- [ ] include/core/config/config.cppm         → one_to_one/src/core/config/config.d
- [ ] include/core/logger/logger.cppm         → one_to_one/src/core/logger/logger.d
- [ ] include/core/logger/registry.cppm       → one_to_one/src/core/logger/registry.d
- [ ] include/core/ffi/ffi.cppm               → one_to_one/src/core/ffi/ffi.d
- [ ] include/core/ffi/bridge.cppm            → one_to_one/src/core/ffi/bridge.d
- [ ] include/core/heart/context.cppm         → one_to_one/src/core/heart/context.d
- [ ] include/core/heart/heart.cppm           → one_to_one/src/core/heart/heart.d
- [ ] include/core/heart/app.cppm             → one_to_one/src/core/heart/app.d
- [ ] include/core/server/consts.cppm         → one_to_one/src/core/server/consts.d
- [ ] include/core/server/types.cppm          → one_to_one/src/core/server/types.d
- [ ] include/core/server/base.cppm           → one_to_one/src/core/server/base.d
- [ ] include/core/server/middleware.cppm     → one_to_one/src/core/server/middleware.d
- [ ] include/core/server/router.cppm         → one_to_one/src/core/server/router.d
- [ ] include/core/server/handler.cppm        → one_to_one/src/core/server/handler.d
- [ ] include/core/server/server.cppm         → one_to_one/src/core/server/server.d
- [ ] include/core/manager/plugin.cppm        → one_to_one/src/core/manager/plugin.d
- [ ] include/core/manager/handle.cppm        → one_to_one/src/core/manager/handle.d
- [ ] include/core/manager/loader.cppm        → one_to_one/src/core/manager/loader.d
- [ ] include/core/manager/handler.cppm       → one_to_one/src/core/manager/handler.d
- [ ] include/core/client/client.cppm         → one_to_one/src/core/client/client.d

### Engine + Worker
- [ ] include/engine/context.cppm             → one_to_one/src/engine/context.d
- [ ] include/engine/routes.cppm              → one_to_one/src/engine/routes.d
- [ ] include/engine/engine.cppm              → one_to_one/src/engine/engine.d
- [ ] include/engine/handler/context.cppm     → one_to_one/src/engine/handler/context.d
- [ ] include/engine/handler/metadata.cppm    → one_to_one/src/engine/handler/metadata.d
- [ ] include/engine/handler/task.cppm        → one_to_one/src/engine/handler/task.d
- [ ] include/engine/handler/workflow.cppm    → one_to_one/src/engine/handler/workflow.d
- [ ] include/worker/config.cppm              → one_to_one/src/worker/config.d
- [ ] include/worker/context.cppm             → one_to_one/src/worker/context.d
- [ ] include/worker/task_worker.cppm         → one_to_one/src/worker/task_worker.d
- [ ] include/worker/worker.cppm              → one_to_one/src/worker/worker.d
- [ ] include/worker/handler/execution.cppm   → one_to_one/src/worker/handler/execution.d
- [ ] include/worker/handler/poll.cppm        → one_to_one/src/worker/handler/poll.d
- [ ] include/worker/handler/status.cppm      → one_to_one/src/worker/handler/status.d

### Model + Serde + Connector
- [ ] include/model/common/identifiers.cppm   → one_to_one/src/model/common/identifiers.d
- [ ] include/model/common/timestamps.cppm    → one_to_one/src/model/common/timestamps.d
- [ ] include/model/common/audit.cppm         → one_to_one/src/model/common/audit.d
- [ ] include/model/common/policies.cppm      → one_to_one/src/model/common/policies.d
- [ ] include/model/task/status.cppm          → one_to_one/src/model/task/status.d
- [ ] include/model/task/definition.cppm      → one_to_one/src/model/task/definition.d
- [ ] include/model/task/instance.cppm        → one_to_one/src/model/task/instance.d
- [ ] include/model/workflow/status.cppm      → one_to_one/src/model/workflow/status.d
- [ ] include/model/workflow/event.cppm       → one_to_one/src/model/workflow/event.d
- [ ] include/model/workflow/definition.cppm  → one_to_one/src/model/workflow/definition.d
- [ ] include/model/workflow/dag.cppm         → one_to_one/src/model/workflow/dag.d
- [ ] include/model/workflow/exec.cppm        → one_to_one/src/model/workflow/exec.d
- [ ] include/model/model.cppm                → one_to_one/src/model/model.d
- [ ] include/serde/core.cppm                 → one_to_one/src/serde/core.d
- [ ] include/serde/converter.cppm            → one_to_one/src/serde/converter.d
- [ ] include/serde/json.cppm                 → one_to_one/src/serde/json.d
- [ ] include/serde/toml.cppm                 → one_to_one/src/serde/toml.d
- [ ] include/serde/cache.cppm                → one_to_one/src/serde/cache.d
- [ ] include/serde/sql.cppm                  → one_to_one/src/serde/sql.d
- [ ] include/serde/serde.cppm                → one_to_one/src/serde/serde.d
- [ ] include/connector/local_cache.cppm      → one_to_one/src/connector/local_cache.d
- [ ] include/connector/connector.cppm        → one_to_one/src/connector/connector.d

### SDK + Top-level
- [ ] sdk/worker/include/congelado/worker.h   → one_to_one/src/sdk/worker/worker.d
- [ ] sdk/plugin/include/congelado/plugin.h   → one_to_one/src/sdk/plugin/plugin.d
- [ ] sdk/worker/congelado_worker.cppm        → one_to_one/src/sdk/worker/congelado_worker.d
- [ ] sdk/plugin/congelado_plugin.cppm        → one_to_one/src/sdk/plugin/congelado_plugin.d
- [ ] include/congelado.cppm                  → one_to_one/src/congelado.d
- [ ] defaults/plugins/http2/http2.cc         → one_to_one/src/defaults/plugins/http2/http2.d

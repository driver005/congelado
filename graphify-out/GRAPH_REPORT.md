# Graph Report - .  (2026-07-18)

## Corpus Check
- 241 files · ~219,200 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 647 nodes · 849 edges · 103 communities (58 shown, 45 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 69 edges (avg confidence: 0.81)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- HPACK Codec
- HTTP/2 Plugin
- PostgreSQL Plugin
- Task Model
- Plugin SDK (C ABI)
- Engine Routes
- File Logger
- Code Generator
- Core Router
- Interfaces
- I/O Leverage (async)
- Shared Flow
- Plugin Manager
- I/O Error Hierarchy
- HTTP/2 Session
- Engine Plugin
- OpenAPI Client SDK
- Domain Model
- Echo Worker
- Transform Worker
- Community 20
- Community 21
- Community 22
- Community 23
- Community 24
- Community 25
- Community 26
- Community 27
- Community 28
- Community 29
- Community 30
- Community 31
- Community 32
- Community 33
- Community 34
- Community 35
- Community 36
- Community 37
- Community 38
- Community 39
- Community 40
- Community 41
- Community 42
- Community 43
- Community 44
- Community 45
- Community 46
- Community 47
- Community 48
- Community 49
- Community 50
- Community 51
- Community 52
- Community 53
- Community 59
- Community 60
- Community 62
- Community 63
- Community 64
- Community 65
- Community 66
- Community 67
- Community 68
- Community 69
- Community 70
- Community 71
- Community 72
- Community 73
- Community 74
- Community 75
- Community 76
- Community 77
- Community 78
- Community 79
- Community 80
- Community 81
- Community 82
- Community 83
- Community 84
- Community 85
- Community 86
- Community 87
- Community 88
- Community 89
- Community 90
- Community 91
- Community 92
- Community 93
- Community 94
- Community 95

## God Nodes (most connected - your core abstractions)
1. `Http2Plugin` - 29 edges
2. `PostgresPlugin` - 18 edges
3. `QPack (top-level codec)` - 16 edges
4. `FileLogger` - 14 edges
5. `HpackDecoderAdapter` - 13 edges
6. `Atom (HPACK/QPACK integer/string codec)` - 13 edges
7. `DecodeError` - 13 edges
8. `ISerializable (concept)` - 13 edges
9. `FfiRuntime` - 12 edges
10. `interfaces module` - 12 edges

## Surprising Connections (you probably didn't know these)
- `MetadataHandler` --conceptually_related_to--> `SchemaRegistry`  [INFERRED]
  plugins/engine/handler/metadata.cppm → include/utils/openapi/schema.cppm
- `CRTP pattern for IoServiceBase rationale` --rationale_for--> `IoServiceBase`  [INFERRED]
  docs/superpowers/specs/2026-07-07-openapi-client-sdk-design.md → src/congelado/io/service.cppm
- `kissnet` --conceptually_related_to--> `IoServiceBase`  [INFERRED]
  include/io/base/socket/README.md → src/congelado/io/service.cppm
- `Atom (HPACK/QPACK integer/string codec)` --semantically_similar_to--> `DEFAULT_MAX_TABLE_SIZE`  [AMBIGUOUS] [semantically similar]
  include/io/codec/shared/atom.cppm → include/io/codec/shared/consts.cppm
- `congelado::heart::AppContext` --conceptually_related_to--> `engine::register_routes`  [INFERRED]
  sdk/heart/context.cppm → plugins/engine/routes.cppm

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Docker compose service architecture** — docker_docker_compose_server, docker_docker_compose_worker, docker_docker_compose_test, docker_docker_compose_network [INFERRED]
- **OpenAPI client SDK code generation pipeline** — docs_superpowers_plans_2026_07_07_openapi_client_sdk_generator, docs_superpowers_plans_2026_07_07_openapi_client_sdk_schemamodelparser, docs_superpowers_plans_2026_07_07_openapi_client_sdk_dtowriter, docs_superpowers_plans_2026_07_07_openapi_client_sdk_routewriter [INFERRED]
- **Client SDK Runtime design and implementation** — docs_superpowers_plans_2026_07_07_openapi_client_sdk_runtime, docs_superpowers_specs_2026_07_07_openapi_client_sdk_design_runtime, docs_superpowers_specs_2026_07_07_openapi_client_sdk_design_sdkclientconcept [INFERRED]

## Communities (103 total, 45 thin omitted)

### Community 0 - "HPACK Codec"
Cohesion: 0.11
Nodes (43): Hpack (top-level codec facade), HpackDecoderAdapter, HpackEncoder, HpackTableSizeUpdateAdaptor, HPackStatic (61-entry static table), HPackTable, EncodePolicy (hpack), policy_for (hpack) (+35 more)

### Community 1 - "HTTP/2 Plugin"
Cohesion: 0.07
Nodes (24): Context, ContractGroup, CongeladoConfigView, CongeladoHostCallbacks, Plugin, span, string, string_view (+16 more)

### Community 2 - "PostgreSQL Plugin"
Cohesion: 0.14
Nodes (10): CongeladoConfigView, CongeladoHostCallbacks, Plugin, string_view, PostgresPlugin, m_conn, IDatabase, PGconn (+2 more)

### Community 3 - "Task Model"
Cohesion: 0.13
Nodes (24): TaskInstance, is_terminal(TaskStatus), TaskStatus, InputMapping, OutputMapping, TaskEdge, TaskNode, WorkflowDef (+16 more)

### Community 4 - "Plugin SDK (C ABI)"
Cohesion: 0.09
Nodes (7): congelado_init(), congelado_load_before_types_count(), congelado_requires_count(), congelado_worker_execute(), CongeladoConfigView, CongeladoHostCallbacks, size_t

### Community 5 - "Engine Routes"
Cohesion: 0.20
Nodes (21): engine::register_routes, worker::WorkerContext::EngineResponse, worker::WorkerContext, worker::ExecutionHandler, worker::PollHandler, worker::StatusHandler, engine:worker module aggregation, congelado::heart::LoggerAdapter (+13 more)

### Community 6 - "File Logger"
Cohesion: 0.16
Nodes (11): CongeladoConfigView, CongeladoHostCallbacks, Plugin, string_view, FileLogger, m_min_level, m_stdout_level, m_stream (+3 more)

### Community 7 - "Code Generator"
Cohesion: 0.23
Nodes (20): Generator, Field and Class (class_builder), Function, Namespace and Module, Param and Stmt, Components, Document, Info (+12 more)

### Community 8 - "Core Router"
Cohesion: 0.16
Nodes (20): core_router module, Route, RouteBuilder, Router, RouterContext, Router constants, Handler, HandlerPool (+12 more)

### Community 9 - "Interfaces"
Cohesion: 0.16
Nodes (20): interfaces (module facade), HeaderEntry, HeaderField<IsStatic>, interfaces:io (module), ReceiveDispatchFn, SendDispatchFn, IRequest, IResponse (+12 more)

### Community 10 - "I/O Leverage (async)"
Cohesion: 0.15
Nodes (20): io_base_leverage (module facade), io::base::leverage::Context (io_uring), Leverager<Context> (posix specialization), completion_callback, Leverager<SharedContext> (primary template), op_type (enum), panic(), liburing namespace (+12 more)

### Community 11 - "Shared Flow"
Cohesion: 0.14
Nodes (20): shared:flow module, FlowBase concept, FlowLayer concept, shared:handler module, HandlerBase, HandlerController concept, HandlerInterface, this_handler namespace (+12 more)

### Community 12 - "Plugin Manager"
Cohesion: 0.26
Nodes (19): LuaBridge, PythonBridge, FfiRuntime, FnEntry, core_plugin module, PluginRef, SharedLibrary, ConfigViewBuilder (+11 more)

### Community 13 - "I/O Error Hierarchy"
Cohesion: 0.16
Nodes (17): ConnectionError class, Http2ErrorCode enum class, Http2Exception class, StreamError class, get_http2_error_code function, async::Receiver class, FrameBuilder class template, FrameHeader class template (+9 more)

### Community 14 - "HTTP/2 Session"
Cohesion: 0.24
Nodes (17): HttpResponse, WriteHttpResponseAdaptor, Session, ReadSettingsAdaptor, Settings, SettingsState, WriteSettingsAdaptor, ConnectionLevelHelper (+9 more)

### Community 15 - "Engine Plugin"
Cohesion: 0.17
Nodes (8): CongeladoConfigView, CongeladoHostCallbacks, Plugin, span, string_view, EnginePlugin, m_engine_ctx, EngineContext

### Community 16 - "OpenAPI Client SDK"
Cohesion: 0.16
Nodes (15): C++26, DTO writer, Generator (client SDK), OpenAPI architecture rationale, Route writer, Runtime (client SDK), SchemaType/SchemaModel parser, SchemaRegistry (+7 more)

### Community 17 - "Domain Model"
Cohesion: 0.18
Nodes (14): AuditRecord, CorrelationId, ExecutionId, generate_id, TaskId, WorkflowId, RateLimitPolicy, RetryBackoff (+6 more)

### Community 18 - "Echo Worker"
Cohesion: 0.21
Nodes (9): CongeladoConfigView, string, string_view, vector, EchoWorker, m_key_ptrs, m_keys, m_val_ptrs (+1 more)

### Community 19 - "Transform Worker"
Cohesion: 0.21
Nodes (9): CongeladoConfigView, string, string_view, vector, TransformWorker, m_output_key_ptrs, m_output_keys, m_output_val_ptrs (+1 more)

### Community 20 - "Community 20"
Cohesion: 0.27
Nodes (11): congelado (top-level module), Connector, LocalCache, Client, BIAS_FLAG constant, Contract, ContractGroup, ContractThreadPool, SignalTree and Node, ContractState enum (+3 more)

### Community 21 - "Community 21"
Cohesion: 0.20
Nodes (10): Flow builder class, io_base_flow module, io_flow_receiver:async module, io_flow_receiver module hub, io_flow_receiver:sync module, io_flow_sender:async module, io_flow_sender module hub, io_flow_sender:sync module (+2 more)

### Community 22 - "Community 22"
Cohesion: 0.25
Nodes (9): COOKIE_SEPARATOR, ENTRY_OVERHEAD, VALUE_SEPARATOR, HeaderField, io_shared:http module, Token, tokenize, io_shared module (+1 more)

### Community 23 - "Community 23"
Cohesion: 0.28
Nodes (9): serde module, Ser (alias = SerBase<Json, Toml>), SerBase, serde:sql module, QueryOptions, Sql, serde:toml module, model::Toml (+1 more)

### Community 24 - "Community 24"
Cohesion: 0.25
Nodes (6): Node, vector, NodeUniverse, nodes, TestNode, value

### Community 25 - "Community 25"
Cohesion: 0.25
Nodes (8): CRTP pattern for IoServiceBase rationale, kissnet, Completion, IoServiceBase, IoVec, IpEndpoint, NativeHandle, OpenFlags

### Community 26 - "Community 26"
Cohesion: 0.25
Nodes (8): DecodeError class, EmptyNameError class, HuffmanDecodeError class, IntegerDecodeError template class, InvalidIndexError template class, StringDecodeError class, TableSizeError class, TruncatedDataError class

### Community 27 - "Community 27"
Cohesion: 0.39
Nodes (8): Deleter, BufferNode, AdvanceReaderAdaptor, BufferReader, NodeReader, BufferView, NodeView, BufferWriter

### Community 28 - "Community 28"
Cohesion: 0.25
Nodes (5): ITaskWorker, TaskInput, TaskOutput, string_view, EchoWorker

### Community 29 - "Community 29"
Cohesion: 0.36
Nodes (8): EngineContext, engine, MetadataHandler, TaskEnqueueBody, TaskHandler, TaskSubmitBody, WorkflowHandler, WorkflowStartBody

### Community 30 - "Community 30"
Cohesion: 0.68
Nodes (8): congelado_client module aggregation, congelado::client::DtoWriter, congelado::client::Generator, congelado::client::OperationInfo, congelado::client::RouteWriter, congelado::client::ClientRuntime, congelado::client::SchemaKind, congelado::client::SchemaType

### Community 31 - "Community 31"
Cohesion: 0.43
Nodes (7): sync::Receiver class, sync::Sender class, ClientFlowSocket class, ConnectorSocket class, ServerBaseSocket class, ServerFlowSocket class, WorkerSocket class

### Community 32 - "Community 32"
Cohesion: 0.29
Nodes (7): io_layer_http2:consts module, io_layer_http2:flow module, io_layer_http2:frame module, io_layer_http2:handshake module, io_layer_http2 module hub, io_layer_http2:plugin module, io_layer_http2:request module

### Community 33 - "Community 33"
Cohesion: 0.48
Nodes (7): AlignedManager, AtomicList, Node, Page, Pager, ConcurrentQueue, ThreadNode

### Community 34 - "Community 34"
Cohesion: 0.47
Nodes (6): asio (module), net (module), netdb (module), openssl (module), socket (module), winsock2 (module)

### Community 35 - "Community 35"
Cohesion: 0.40
Nodes (3): Node, TestNode, value

### Community 36 - "Community 36"
Cohesion: 0.60
Nodes (3): run.sh script, status(), test_result()

### Community 37 - "Community 37"
Cohesion: 0.50
Nodes (5): ClientFlow class, ServerFlow class, Client class, Http2Protocol class, Server class

### Community 38 - "Community 38"
Cohesion: 0.83
Nodes (4): congelado bridge network, congelado-server service, congelado-test service, congelado-worker service

### Community 39 - "Community 39"
Cohesion: 0.83
Nodes (4): core_config (module), config loader (parse_toml, parse_json, load), Config, PluginConfig

### Community 40 - "Community 40"
Cohesion: 0.50
Nodes (4): ReadBigEndianAdaptor, ReadVariantEndianAdaptor, VariantEndianView, WriteVariantEndianAdaptor

### Community 42 - "Community 42"
Cohesion: 0.67
Nodes (3): ByteReader, ByteWriter, VarInt

### Community 43 - "Community 43"
Cohesion: 0.67
Nodes (3): ConnectionError (http2), Http2Exception, StreamError (http2)

### Community 44 - "Community 44"
Cohesion: 1.00
Nodes (3): AsyncExecutor concept, IFlags enum, AsyncSocket concept

### Community 45 - "Community 45"
Cohesion: 0.67
Nodes (3): hashmap, Entry, SwissHashMap

## Ambiguous Edges - Review These
- `Atom (HPACK/QPACK integer/string codec)` → `DEFAULT_MAX_TABLE_SIZE`  [AMBIGUOUS]
  include/io/codec/shared/atom.cppm · relation: semantically_similar_to
- `async::Receiver class` → `HttpRequest class`  [AMBIGUOUS]
  include/io/layer/http2/req.cppm · relation: conceptually_related_to

## Knowledge Gaps
- **186 isolated node(s):** `value`, `m_engine_ctx`, `m_stream`, `m_min_level`, `m_stdout_level` (+181 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **45 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `Atom (HPACK/QPACK integer/string codec)` and `DEFAULT_MAX_TABLE_SIZE`?**
  _Edge tagged AMBIGUOUS (relation: semantically_similar_to) - confidence is low._
- **What is the exact relationship between `async::Receiver class` and `HttpRequest class`?**
  _Edge tagged AMBIGUOUS (relation: conceptually_related_to) - confidence is low._
- **Why does `interfaces module` connect `Core Router` to `Plugin Manager`?**
  _High betweenness centrality (0.002) - this node is a cross-community bridge._
- **Are the 4 inferred relationships involving `QPack (top-level codec)` (e.g. with `EncodePolicy (qpack)` and `DecodeIntAdaptor (pipeable prefix-int)`) actually correct?**
  _`QPack (top-level codec)` has 4 INFERRED edges - model-reasoned connections that need verification._
- **Are the 2 inferred relationships involving `HpackDecoderAdapter` (e.g. with `Atom (HPACK/QPACK integer/string codec)` and `Huffman (encode/decode adaptors)`) actually correct?**
  _`HpackDecoderAdapter` has 2 INFERRED edges - model-reasoned connections that need verification._
- **What connects `value`, `m_engine_ctx`, `m_stream` to the rest of the system?**
  _186 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `HPACK Codec` be split into smaller, more focused modules?**
  _Cohesion score 0.11184939091915837 - nodes in this community are weakly interconnected._
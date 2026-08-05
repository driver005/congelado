export module congelado_client;

// The OpenAPI-doc-driven codegen partitions (:document, :schema_model, :dto_writer,
// :route_writer, :generator) moved out to plugins/openapi_generator/ — build-time-only codegen,
// now behind interfaces::IOpenApiGenerator instead of a directly-imported partition here. Every
// remaining consumer of this module (src/worker_main.cc, plugins/engine/worker/context.cppm,
// generated client routes) only ever used :runtime (ClientRuntime), confirmed by grepping the
// whole repo for `congelado::client::` before this split — so this umbrella now only re-exports
// that.
export import :runtime;

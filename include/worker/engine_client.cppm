export module worker:engine_client;

import std;
import :config;

export namespace worker {

// Stub: not implemented. All methods return success.
// Real implementation: HTTP calls to the engine's REST API.
class EngineClient {
  public:
    explicit EngineClient(std::string_view engine_url) : m_url(engine_url) {}

    // Stub: always returns true. Real impl: POST /api/v1/task-defs
    [[nodiscard]] bool upsert_task_def(TaskConfig const & /*cfg*/) noexcept { return true; }

  private:
    std::string m_url;
};

} // namespace worker

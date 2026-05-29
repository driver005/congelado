export module worker:poller;

import std;
import :context;
import :engine_client;

export namespace worker {

// Stub: not implemented. start/stop/join are no-ops.
// Real implementation: spawns `concurrency` threads each running PollHandler::poll() in a loop.
class Poller {
  public:
    Poller(WorkerContext & /*ctx*/, EngineClient & /*client*/,
           std::uint32_t /*concurrency*/) noexcept {}

    void start() noexcept {}
    void stop() noexcept {}
    void join() noexcept {}
};

} // namespace worker

export module core_heart:context;

import std;
import io_shared;
import core_server;

export namespace core::heart {

class AppContext {
  public:
    AppContext() = default;

    [[nodiscard]] core::server::RouterContext<io::shared::http::Protocol> *get_router() noexcept { return &router_; }

  private:
    core::server::RouterContext<io::shared::http::Protocol> router_;
};

} // namespace core::heart

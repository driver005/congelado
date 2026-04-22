export module interfaces:logger;

import std;
import shared;

export namespace interfaces {

class ILogger {
  public:
    virtual ~ILogger() = default;

    virtual std::string name() const = 0;

    // Per rules: Settings is the first string which has to be received back.
    virtual std::string initialize() = 0;

    // The actual logging endpoint. No templates here to keep ABI stable across plugins.
    virtual void write(shared::LogLevel level, std::string_view message) noexcept = 0;

    virtual void error(std::string_view message) = 0;
};

} // namespace interfaces

module;
#include "core/manager/abi.h"
export module interfaces:plugin_bridge;

import std;

export struct FnContext {
    std::any invoke;  // std::function<Value(std::span<const Value>)>
    std::string key;
};

export namespace interfaces {

class IBridge {
  public:
    virtual ~IBridge() = default;

    [[nodiscard]] virtual CongeladoAny from_native(void *native_obj) = 0;
    virtual void *to_native(const CongeladoAny &a) = 0;
    virtual void install_method(std::unique_ptr<FnContext> ctx, const std::string &lang_name) = 0;
};

} // namespace interfaces

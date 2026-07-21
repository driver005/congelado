module;
#include "core/manager/abi.h"
export module interfaces:plugin_bridge;

import std;

export struct FnContext {
    std::any m_invoke;  // std::function<Value(std::span<const Value>)>
    std::string m_key;
};

export namespace interfaces {

class IBridge {
  public:
    /**
     * @brief Virtual dtor, default's fine — bridges clean up fine through the base pointer, no
     * extra cleanup motion required.
     */
    virtual ~IBridge() = default;
    IBridge() = default;
    IBridge(const IBridge &) = delete;
    IBridge &operator=(const IBridge &) = delete;
    IBridge(IBridge &&) = delete;
    IBridge &operator=(IBridge &&) = delete;

    /**
     * @brief Wraps a raw native pointer into a `CongeladoAny` so the plugin ABI can pass it
     * around without either side needing to know the concrete type. This is the whole point of
     * a bridge — keeping both languages from having to know each other's business.
     * @param native_obj raw pointer coming from the native (host language) side.
     * @return a `CongeladoAny` wrapping that pointer for cross-boundary use.
     */
    [[nodiscard]] virtual CongeladoAny from_native(void *native_obj) = 0;
    /**
     * @brief Unwraps a `CongeladoAny` back down to the raw native pointer it came from — the
     * exact inverse of from_native(), gotta stay symmetric with it or things get cooked fast.
     * @param value the wrapped value to unwrap.
     * @return the raw native pointer underneath.
     */
    virtual void *to_native(const CongeladoAny &value) = 0;
    /**
     * @brief Registers a method so it's callable from the target language, keyed by `lang_name`
     * — this is how a plugin actually exposes its motion to whatever language is on the other
     * side of the bridge.
     * @param ctx the function context (the callable plus its key) to install.
     * @param lang_name the name the method gets exposed as on the target-language side.
     */
    virtual void install_method(std::unique_ptr<FnContext> ctx, const std::string &lang_name) = 0;
};

} // namespace interfaces

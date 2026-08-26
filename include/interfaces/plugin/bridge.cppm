module;
#include "core/manager/abi.h"
export module interfaces:plugin_bridge;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export struct FnContext
{
    std::any m_invoke; // std::function<Value(std::span<const Value>)>
    std::string m_key;
};

export namespace interfaces {

class IBridge
{
public:
    /**
     * @brief Virtual dtor, default's fine — bridges clean up fine through the base pointer, no
     * extra cleanup motion required.
     */
    virtual ~IBridge() = default;
    IBridge() = default;
    IBridge(const IBridge&) = delete;
    IBridge& operator=(const IBridge&) = delete;
    IBridge(IBridge&&) = delete;
    IBridge& operator=(IBridge&&) = delete;

    /**
     * @brief Wraps a raw native pointer into a `CongeladoAny` so the plugin ABI can pass it
     * around without either side needing to know the concrete type. This is the whole point of
     * a bridge — keeping both languages from having to know each other's business.
     * @param native_obj raw pointer coming from the native (host language) side.
     * @return a `CongeladoAny` wrapping that pointer for cross-boundary use.
     */
    [[nodiscard]] virtual CongeladoAny from_native(void* native_obj) = 0;
    /**
     * @brief Unwraps a `CongeladoAny` back down to the raw native pointer it came from — the
     * exact inverse of from_native(), gotta stay symmetric with it or things get cooked fast.
     * @param value the wrapped value to unwrap.
     * @return the raw native pointer underneath.
     */
    virtual void* to_native(const CongeladoAny& value) = 0;
    /**
     * @brief Registers a method so it's callable from the target language, keyed by `lang_name`
     * — this is how a plugin actually exposes its motion to whatever language is on the other
     * side of the bridge.
     * @param ctx the function context (the callable plus its key) to install.
     * @param lang_name the name the method gets exposed as on the target-language side.
     */
    virtual void install_method(std::unique_ptr<FnContext> ctx, const std::string& lang_name) = 0;

    /**
     * @brief Gets this bridge's underlying native interpreter handle, if it has one crossing
     * the ABI as a plain pointer makes sense for (e.g. Lua's `lua_State*` — a script runner
     * needs the exact same state the bridge installed methods into, not a fresh one).
     * @note Defaults to `nullptr` — most bridges don't need this. Python's interpreter is
     * process-global (`Py_Initialize()`), so `PythonBridge` never overrides it; there's no
     * per-bridge handle to hand back.
     * @return the native handle, or `nullptr` if this bridge doesn't expose one.
     */
    [[nodiscard]] virtual void* native_handle() noexcept
    {
        return nullptr;
    }

    /**
     * @brief The runtime this bridge implements (e.g. `"python"`, `"lua"`, or any other
     * user-registered name) — this is the key each `core::plugin::FfiRuntime` looks bridges up
     * by (via `add_bridge`/`get_bridge`), same self-identification role
     * `ISerdeFormat::content_type()` plays for format plugins. The bridge says what it is;
     * nothing else has to guess from unrelated metadata like a plugin's display name, and
     * nothing hardcodes a fixed set of supported runtime names.
     * @return the runtime name string.
     */
    [[nodiscard]] virtual std::string_view runtime_name() const noexcept = 0;

    /**
     * @brief The file extension (dot included, e.g. `".py"`, `".lua"`) of scripts this bridge
     * can run. Self-reported so a caller with a script path never has to hardcode which
     * extension belongs to which runtime — it just asks every loaded bridge until one matches.
     * @return the extension this bridge handles.
     */
    [[nodiscard]] virtual std::string_view script_extension() const noexcept = 0;

    /**
     * @brief Runs an external script file through this bridge's own language runtime — the
     * bridge owns whatever native API call that requires (e.g. `PyRun_SimpleFile`,
     * `luaL_dofile`), so a caller never needs to touch the concrete language's C API itself.
     * @param path path to the script file to run.
     * @return the script's exit/result code (language-specific convention: 0 is success).
     */
    [[nodiscard]] virtual int run_script(std::string_view path) = 0;
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::plugin_bridge_tests {

// Minimal IBridge fixture — every pure virtual gets a trivial body so native_handle()'s
// default implementation can be exercised in isolation.
class MockBridge final : public interfaces::IBridge
{
public:
    [[nodiscard]] CongeladoAny from_native(void* native_obj) override
    {
        CongeladoAny any{};
        any.type_index = CG_PTR;
        any.v_ptr = native_obj;
        return any;
    }

    void* to_native(const CongeladoAny& value) override
    {
        return value.v_ptr;
    }

    void install_method(std::unique_ptr<FnContext>, const std::string&) override {}

    [[nodiscard]] std::string_view runtime_name() const noexcept override
    {
        return "mock";
    }

    [[nodiscard]] std::string_view script_extension() const noexcept override
    {
        return ".mock";
    }

    [[nodiscard]] int run_script(std::string_view) override
    {
        return 0;
    }
};

using namespace boost::ut;

suite<"IBridge"> bridge_suite = [] {
    "native_handle() defaults to nullptr when not overridden"_test = [] {
        MockBridge bridge;
        expect(bridge.native_handle() == nullptr);
    };
};

} // namespace interfaces::plugin_bridge_tests
#endif

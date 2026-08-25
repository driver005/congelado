export module utils_openapi:generator_registry;

import std;
import :generator_interface;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::openapi {

/**
 * @brief Holds every registered OpenAPI-generator plugin for one process — mirrors
 * `serde::SerdeFormatRegistry` exactly (same instance-owned + `set_active()`/`get_active()`
 * ambient-pointer shape), except there's no content-type-style key to look up by: unlike wire
 * formats, only one thing ever matters here — whether at least one generator backend is loaded
 * at all — so this registry only supports registering and iterating/checking, not a keyed find().
 */
class OpenApiGeneratorRegistry {
  public:
    /**
     * @brief Registers a loaded generator plugin. No-op if `generator` is null.
     * @param generator the generator instance to add.
     */
    void add_generator(std::shared_ptr<interfaces::IOpenApiGenerator> generator) {
        if (generator) {
            m_generators.push_back(std::move(generator));
        }
    }

    /**
     * @brief Checks whether at least one generator backend is registered — the only thing that
     * matters here, since there's no fixed key to look up by.
     * @return true if one or more generators are registered, false otherwise.
     */
    [[nodiscard]] bool has_generator() const noexcept { return !m_generators.empty(); }

    /// @brief Gets every registered generator. @return all registered generators, in registration order.
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::IOpenApiGenerator>> &
    get_generators() const noexcept {
        return m_generators;
    }

    /**
     * @brief Points the ambient facade at this instance — call once, right after constructing
     * the process's one `OpenApiGeneratorRegistry`, mirroring `SerdeFormatRegistry::set_active()`.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(OpenApiGeneratorRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `OpenApiGeneratorRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static OpenApiGeneratorRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::IOpenApiGenerator>> m_generators;
    static inline OpenApiGeneratorRegistry *s_active{nullptr};
};

} // namespace utils::openapi

// add_generator()/get_generators() need a concrete interfaces::IOpenApiGenerator implementation
// to exercise — that interface's serve_document() returns a core::router::Route<> by value,
// which needs live router wiring to construct meaningfully, so it's out of scope here. What's
// tested below is the registry's own state machine (default-empty, set_active/get_active),
// which doesn't touch IOpenApiGenerator at all.
#ifdef CONGELADO_TEST
namespace utils::openapi::tests {
using namespace boost::ut;

suite<"OpenApiGeneratorRegistry"> openapi_generator_registry_suite = [] {
    "defaults to no generators registered"_test = [] {
        OpenApiGeneratorRegistry registry;
        expect(not registry.has_generator());
        expect(registry.get_generators().empty());
    };
    "set_active/get_active track the ambient pointer"_test = [] {
        expect(OpenApiGeneratorRegistry::get_active() == nullptr);

        OpenApiGeneratorRegistry registry;
        OpenApiGeneratorRegistry::set_active(&registry);
        expect(OpenApiGeneratorRegistry::get_active() == &registry);

        OpenApiGeneratorRegistry::set_active(nullptr);
        expect(OpenApiGeneratorRegistry::get_active() == nullptr);
    };
    // Documents that nothing in OpenApiGeneratorRegistry's lifecycle clears s_active
    // automatically: destroying the actively-registered instance leaves the ambient pointer
    // dangling until a caller explicitly calls set_active(nullptr). This test demonstrates the
    // gap by performing that cleanup itself, from a fresh scope, after the instance is already
    // gone -- it never reads get_active() while the pointer is dangling.
    "no automatic cleanup: destroying the active instance leaves s_active dangling until cleared"_test = [] {
        auto *previous = OpenApiGeneratorRegistry::get_active();

        {
            OpenApiGeneratorRegistry registry;
            OpenApiGeneratorRegistry::set_active(&registry);
            expect(OpenApiGeneratorRegistry::get_active() == &registry);
        } // registry destroyed here -- s_active still points at the freed instance, nothing
          // clears it automatically

        // Explicit cleanup the class itself never performs on destruction.
        OpenApiGeneratorRegistry::set_active(nullptr);
        expect(OpenApiGeneratorRegistry::get_active() == nullptr);

        OpenApiGeneratorRegistry::set_active(previous);
    };
    "add_generator is a no-op for a null generator"_test = [] {
        OpenApiGeneratorRegistry registry;
        registry.add_generator(nullptr);
        expect(not registry.has_generator());
    };
};

} // namespace utils::openapi::tests
#endif

export module utils_openapi:generator_registry;

import std;
import :generator_interface;

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

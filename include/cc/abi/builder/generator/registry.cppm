module;

export module cc_abi_builder_generator:registry;

import std;
import cc_abi_builder_intern;
import :generator_builder_base;

export namespace ice {

// Registry for generator implementations.
// Generators register their factory function at startup (via module init).
// runtime/generator's GeneratorRuntime uses this registry to create generator builders by
// name — this is the whole reason cc_stable_hlo doesn't need cc_abi to know it exists:
// stable_hlo registers itself here (a downward dependency, stable_hlo -> cc_abi_builder_generator)
// instead of the ABI importing stable_hlo directly.
class GeneratorBuilderRegistry {
public:
    using Factory = GeneratorBuilderBase::Factory;

    // Register a generator factory under a name (e.g., "stablehlo").
    // Called once at module initialization by each generator implementation.
    void register_factory(std::string_view name, Factory factory) {

        factories_[std::string{name}] = factory;

    }

    // Create a generator builder by name.
    // Returns null if no factory registered for the name.
    std::unique_ptr<GeneratorBuilderBase> create(std::string_view name, std::string_view output_dir, std::string_view source_dir) {

        auto it = factories_.find(std::string{name});
        if (it == factories_.end()) {
            return nullptr;
        }
        return it->second(output_dir, source_dir);

    }

    // Get or create a generator builder instance (for the runtime C ABI adapter).
    // The registry owns the created instances.
    GeneratorBuilderBase* get_or_create(std::string_view name, std::string_view output_dir, std::string_view source_dir) {

        auto& instance = instances_[std::string{name}];
        if (!instance) {
            instance = create(name, output_dir, source_dir);
        }
        return instance.get();

    }

    // Check if a generator is registered
    bool contains(std::string_view name) const {

        return factories_.contains(std::string{name});

    }

    // List all registered generator names
    std::vector<std::string> names() const {

        std::vector<std::string> result;
        result.reserve(factories_.size());
        for (const auto& [name, _] : factories_) {
            result.push_back(name);
        }
        return result;

    }

    // Process-wide singleton registry
    static GeneratorBuilderRegistry& default_registry() {

        static GeneratorBuilderRegistry registry;
        return registry;

    }

private:
    std::map<std::string, Factory> factories_;
    std::map<std::string, std::unique_ptr<GeneratorBuilderBase>> instances_;
};

} // namespace ice

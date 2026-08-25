module;

export module cc_stable_hlo:registrar;

import std;
import cc_abi_builder_generator;
import :builder;

export namespace cc::stable_hlo {

// Registers StableHloBuilder::create into cc_abi_builder_generator's GeneratorBuilderRegistry
// under "stablehlo" at static init — this is the whole reason cc_stable_hlo doesn't need
// cc/abi to know it exists: it registers itself here (a downward dependency, stable_hlo ->
// cc_abi_builder_generator) instead of the ABI importing stable_hlo directly.
struct StableHloGeneratorRegistrar {
    StableHloGeneratorRegistrar() {

        ice::GeneratorBuilderRegistry::default_registry().register_factory(
            "stablehlo", [](std::string_view output_dir, std::string_view source_dir) {

                return StableHloBuilder::create(output_dir, source_dir);

            });

    }
};
inline StableHloGeneratorRegistrar stable_hlo_generator_registrar;

#ifdef CONGELADO_TEST
namespace tests {
using namespace boost::ut;

suite<"StableHloGeneratorRegistrar"> stable_hlo_generator_registrar_suite = [] {
    "resolving \"stablehlo\" through the registry produces a catalog"_test = [] {
        auto *generator = ice::GeneratorBuilderRegistry::default_registry().get_or_create("stablehlo", "/tmp", "");
        expect(generator != nullptr);
        expect(generator->get_definition_count() > 0_ul);
    };
};

} // namespace tests
#endif

} // namespace cc::stable_hlo

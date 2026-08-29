// Shared-library entry point for yoshi::core::PluginLoader — dlopen's this .so, dlsym's
// "TF_InitPlugin", calls it with a pointer to a TF_PluginInfo the caller allocated. Registers
// a TF_InitGenerator-shaped factory into ice::sonic::RegistrationRuntime
// (type="generator", name="stablehlo") — the shared, cross-.so-safe storage that
// ice::sonic::Generator::create looks up by name. The factory fills the flat TF_Generator
// vtable from the Builder's get_generic_vtable() and hands the Builder back as the plugin
// context, so the host reaches the generator entirely through the vtable.

import std;
import yoshi_omah_lay_stable_hlo;
import cc_abi_sonic_registration;
import cc_abi_primitives;
import cc_abi_sonic_intern;

#include "c/extern/plugin/registration.h"
#include "c/extern/generator/generator.h"

namespace {

    // Tensor runtime the Builder's definitions allocate through — obtained from the
    // host's registered "tensor" factory (ice::sonic::Tensor::create). Plugin-lifetime
    // static: every Builder holds a reference to it, so it must outlive them all. Null
    // when the host hasn't registered a tensor factory — get_definitions then fails
    // with a clear Status instead of dereferencing null.
    std::unique_ptr<ice::sonic::Tensor>& generator_tensor_runtime()
    {
        static std::unique_ptr<ice::sonic::Tensor> tensor = [] {
            auto t = ice::sonic::Tensor::create("tensor");
            if (!t) {
                return std::unique_ptr<ice::sonic::Tensor>{};
            }
            return std::move(*t);
        }();
        return tensor;
    }

    // TF_InitGenerator-shaped factory: *ops = the Builder's vtable, *plugin_context =
    // the Builder (freed by the vtable's destroy slot).
    void stable_hlo_init(TF_Generator** ops, void** plugin_context, TF_Status* /*status*/)
    {
        std::unique_ptr<cc::stable_hlo::Builder> builder;
        auto& tensor = generator_tensor_runtime();
        if (tensor) {
            builder = std::make_unique<cc::stable_hlo::Builder>(*tensor);
        } else {
            builder = std::make_unique<cc::stable_hlo::Builder>();
        }
        *ops = builder->get_generic_vtable();
        *plugin_context = builder.release();
    }

} // namespace

extern "C" void TF_InitPlugin(TF_PluginInfo* plugin_info) {
  plugin_info->name = "stablehlo";
  plugin_info->version = "1.0";
  ice::sonic::RegistrationRuntime::register_value(
      "generator", "stablehlo",
      reinterpret_cast<void*>(&stable_hlo_init)
  );
}

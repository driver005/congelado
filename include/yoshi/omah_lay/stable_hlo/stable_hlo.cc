// Shared-library entry point for yoshi::core::PluginLoader — dlopen's this .so, dlsym's
// "TF_InitPlugin", calls it with a pointer to a TF_PluginInfo the caller allocated. Also
// registers cc::stable_hlo::Builder's factory function pointer into ice::sonic::
// RegistrationRuntime (type="generator", name="stablehlo") — the shared, cross-.so-safe
// storage that ice::sonic::Generator::create looks up by name.

import yoshi_omah_lay_stable_hlo;
import cc_abi_sonic_registration;
import cc_abi_primitives;

#include "c/extern/plugin/registration.h"

namespace {

    void* create_stable_hlo_builder(const ice::String&, const ice::String&)
    {
        return new cc::stable_hlo::Builder();
    }

} // namespace

extern "C" void TF_InitPlugin(TF_PluginInfo* plugin_info) {
  plugin_info->name = "stablehlo";
  plugin_info->version = "1.0";
  ice::sonic::RegistrationRuntime::register_value(
      ice::String("generator"), ice::String("stablehlo"), reinterpret_cast<void*>(&create_stable_hlo_builder)
  );
}

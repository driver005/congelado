export module core_plugin:handle;

import std;
import core_ffi;

export namespace core::plugin {

// Re-export LoadError from core_ffi for callers that only import core_plugin.
using core::ffi::LoadError;
using core::ffi::Cap;

// PluginHandle is now just a shared_ptr<FfiBridge>.
// All lifecycle, dispatch, and capability detection live in FfiBridge.
using PluginHandle = std::shared_ptr<core::ffi::FfiBridge>;

} // namespace core::plugin

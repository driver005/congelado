module core.manager.handle;
@nogc nothrow:

import core.ffi.bridge : FfiBridge, LoadError, Cap;

// Re-export LoadError from core_ffi for callers that only import core_plugin.
public alias LoadError = core.ffi.bridge.LoadError;
public alias Cap       = core.ffi.bridge.Cap;

// PluginHandle is a plain reference to FfiBridge.
// All lifecycle, dispatch, and capability detection live in FfiBridge.
// PORT-NOTE: std::shared_ptr<FfiBridge> → plain D class reference (GC or util.alloc)
alias PluginHandle = FfiBridge;

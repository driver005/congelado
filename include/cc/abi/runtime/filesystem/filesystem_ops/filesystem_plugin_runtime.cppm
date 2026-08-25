module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:filesystem_plugin_runtime;

export namespace ice {

class FilesystemPluginRuntime {
  public:
    FilesystemPluginRuntime() : m_handle{nullptr} {}
    explicit FilesystemPluginRuntime(TF_FilesystemPluginInfo *handle) : m_handle{handle} {}

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginInfo *get_handle() const { return m_handle; }

  private:
    TF_FilesystemPluginInfo *m_handle;
};

} // namespace ice

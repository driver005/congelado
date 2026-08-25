module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:filesystem_plugin_ops_runtime;

export namespace ice {

class FilesystemPluginOpsRuntime {
  public:
    FilesystemPluginOpsRuntime() : m_handle{nullptr} {}
    explicit FilesystemPluginOpsRuntime(TF_FilesystemPluginOps *handle) : m_handle{handle} {}

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginOps *get_handle() const { return m_handle; }

  private:
    TF_FilesystemPluginOps *m_handle;
};

} // namespace ice

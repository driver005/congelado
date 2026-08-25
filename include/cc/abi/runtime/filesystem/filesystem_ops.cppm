module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:filesystem_ops;

export namespace ice {

// Non-owning wrappers around the filesystem vtables a plugin registers. Unlike the other
// capabilities, no host-side dispatcher for these ~25 callbacks exists anywhere in this repo
// yet (TF's real filesystem registry internals aren't modeled here) — kept as thin
// get_handle()-only placeholders, same reasoning as OpsRuntime, until a real invocation path
// exists to design invoke_* methods against.

class FilesystemOpsRuntime {
  public:
    FilesystemOpsRuntime() : m_handle{nullptr} {}
    explicit FilesystemOpsRuntime(TF_FilesystemOps *handle) : m_handle{handle} {}

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemOps *get_handle() const { return m_handle; }

  private:
    TF_FilesystemOps *m_handle;
};

class FilesystemPluginOpsRuntime {
  public:
    FilesystemPluginOpsRuntime() : m_handle{nullptr} {}
    explicit FilesystemPluginOpsRuntime(TF_FilesystemPluginOps *handle) : m_handle{handle} {}

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginOps *get_handle() const { return m_handle; }

  private:
    TF_FilesystemPluginOps *m_handle;
};

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

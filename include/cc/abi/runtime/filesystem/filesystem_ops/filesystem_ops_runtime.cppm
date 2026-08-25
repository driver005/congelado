module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:filesystem_ops_runtime;

export namespace ice {

// Non-owning wrapper around the filesystem vtable a plugin registers. Unlike the other
// capabilities, no host-side dispatcher for these ~25 callbacks exists anywhere in this repo
// yet (TF's real filesystem registry internals aren't modeled here) — kept as a thin
// get_handle()-only placeholder, same reasoning as OpsRuntime, until a real invocation path
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

} // namespace ice

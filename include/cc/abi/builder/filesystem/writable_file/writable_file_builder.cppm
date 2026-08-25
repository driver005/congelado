module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:writable_file_builder;

export namespace ice {

class WritableFileBuilder {
  public:
    WritableFileBuilder() : m_handle{TF_WritableFileNew()} {}
    ~WritableFileBuilder() { TF_WritableFileDelete(m_handle); }

    WritableFileBuilder(const WritableFileBuilder &) = delete;
    WritableFileBuilder &operator=(const WritableFileBuilder &) = delete;

    WritableFileBuilder(WritableFileBuilder &&other) noexcept : m_handle(other.m_handle) {

        other.m_handle = nullptr;

    }

    WritableFileBuilder &operator=(WritableFileBuilder &&other) noexcept {

        if (this != &other) {
            TF_WritableFileDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    void *get_plugin_file() const { return m_handle->plugin_file; }
    WritableFileBuilder &set_plugin_file(void *value) {

        TF_WritableFile_SetPluginFile(m_handle, value);
        return *this;

    }

    // Underlying handle — pass directly to the C ABI
    TF_WritableFile *get_handle() { return m_handle; }
    const TF_WritableFile *get_handle() const { return m_handle; }

  private:
    TF_WritableFile *m_handle;
};

} // namespace ice

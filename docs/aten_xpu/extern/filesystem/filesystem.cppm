module;

#include "include/c/extern/filesystem/filesystem.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_extern_filesystem:filesystem;

import std;
import cc_ice_extern_filesystem_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// The filesystem identity: a name. Files and the tree are separate objects.
class Filesystem : public ice::builder::Filesystem
{
public:
    explicit Filesystem(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Filesystem{string_ops}
    {
    }

    ~Filesystem() override = default;
    Filesystem(const Filesystem&) = delete;
    Filesystem& operator=(const Filesystem&) = delete;
    Filesystem(Filesystem&&) = delete;
    Filesystem& operator=(Filesystem&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("Filesystem");
    }
};

} // namespace aten_xpu

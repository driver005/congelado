module;

#include "c/extern/ops.h"

export module cc_abi_builder:shape_handle;

export namespace ice::builder {

// ShapeHandle/DimensionHandle/ShapeInferenceContextView — support types consumed *inside* a
// plugin-authored shape-inference callback body (set via
// OpsBuilder::set_shape_inference_function), operating on a TF_ShapeInferenceContext* the host
// constructs and passes in. They live with the builder side because that's whose code actually
// calls them, not because the plugin owns the context itself.

// Owned wrapper for TF_ShapeHandle
class ShapeHandle
{
public:
    ShapeHandle() :
        m_handle{nullptr}
    {
    }

    explicit ShapeHandle(TF_ShapeHandle* handle) :
        m_handle{handle}
    {
    }

    ~ShapeHandle()
    {
        if (m_handle) {
            TF_DeleteShapeHandle(m_handle);
        }
    }

    ShapeHandle(const ShapeHandle&) = delete;
    ShapeHandle& operator=(const ShapeHandle&) = delete;

    ShapeHandle(ShapeHandle&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    ShapeHandle& operator=(ShapeHandle&& other) noexcept
    {

        if (this != &other) {
            if (m_handle) {
                TF_DeleteShapeHandle(m_handle);
            }
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    static ShapeHandle create()
    {
        return ShapeHandle(TF_NewShapeHandle());
    }

    // Underlying handle — pass directly to the C ABI
    TF_ShapeHandle* get_handle()
    {
        return m_handle;
    }

    const TF_ShapeHandle* get_handle() const
    {
        return m_handle;
    }

private:
    TF_ShapeHandle* m_handle;
};

} // namespace ice::builder

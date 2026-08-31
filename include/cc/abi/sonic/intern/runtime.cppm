module;

#include "c/intern/tf_status.h"

export module cc_abi_sonic_intern:runtime;

import std;
import cc_abi_primitives;
import cc_abi_sonic_registration;

export namespace ice::sonic {

template<typename T, typename OpsStruct>
class Runtime
{
public:
    virtual ~Runtime()
    {
        if (m_host_context && m_ops) {
            // Some vtable structs (TF_Shape, TF_Buffer, TF_TensorOps, TF_DataTypeOps)
            // have no destroy slot — the plugin owns those objects and frees them via
            // delete_shape/delete_buffer/delete_tensor instead. Guard with if constexpr
            // so the destructor compiles for every OpsStruct.
            if constexpr (requires { m_ops->destroy; }) {
                if (m_ops->destroy) {
                    m_ops->destroy(m_host_context);
                }
            }
        }
    }

    template<typename... Args>
    [[nodiscard]] static std::expected<std::unique_ptr<T>, ice::Status>
    resolve(std::string_view name, Args&&... args) noexcept
    {
        try {
            ice::String name_str{name};
            void* factory_ptr =
                ice::sonic::Registration::get(T::domain_name.data(), name_str.c_str());
            if (!factory_ptr) {
                return std::unexpected{
                    ice::Status(ice::StatusCode::NotFound, "Factory not found in registry")
                };
            }

            ice::Status status;

            OpsStruct* ops = nullptr;
            void* plugin_context = nullptr;

            using InitFnType = void (*)(OpsStruct**, void**, TF_Status*);
            // factory_ptr was stored as void* in the registry — std::bit_cast recovers the
            // function pointer with a compile-time size check instead of a raw reinterpret_cast.
            auto init_fn = std::bit_cast<InitFnType>(factory_ptr);

            // The plugin's init function is the first thing that runs across the .so
            // boundary; it may throw (plugin code) — the outer try/catch converts any
            // escape into an error instead of letting it unwind through the host.
            init_fn(&ops, &plugin_context, status.get_handle());

            if (!status.ok()) {
                if constexpr (requires { ops->destroy; }) {
                    if (plugin_context && ops && ops->destroy) {
                        ops->destroy(plugin_context);
                    }
                }
                return std::unexpected{status};
            }

            // Version check: the plugin's vtable must be at least as large as the layout
            // this host was compiled against. struct_size is the plugin's declared size
            // (offset-of-end of its fields); reject undersized/absent vtables before
            // calling through them.
            if (!ops || ops->struct_size < sizeof(OpsStruct)) {
                if constexpr (requires { ops->destroy; }) {
                    if (plugin_context && ops && ops->destroy) {
                        ops->destroy(plugin_context);
                    }
                }
                return std::unexpected{
                    ice::Status(ice::StatusCode::Internal, "plugin vtable missing or undersized")
                };
            }

            auto instance = std::unique_ptr<T>(new T(ops, plugin_context));

            return instance;
        } catch (const std::exception& e) {
            return std::unexpected{
                ice::Status(ice::StatusCode::Internal, e.what())
            };
        } catch (...) {
            return std::unexpected{
                ice::Status(ice::StatusCode::Internal, "unknown exception in plugin factory")
            };
        }
    }

    void* get_handle() const noexcept
    {
        return m_host_context;
    }

    OpsStruct* get_ops() const noexcept
    {
        return m_ops;
    }

protected:
    explicit Runtime(OpsStruct* ops, void* plugin_context) noexcept :
        m_ops{ops},
        m_host_context{plugin_context}
    {
    }

    OpsStruct* m_ops;
    void* m_host_context;
};

} // namespace ice::sonic

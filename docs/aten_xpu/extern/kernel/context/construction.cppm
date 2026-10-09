module;

#include "include/c/extern/kernel/construction.h"

export module aten_xpu_extern_kernel:construction;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_kernel_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclKernelConstruction
{
public:
    explicit SyclKernelConstruction(::TF_OpKernelConstruction* construction) :
        m_ops{SyclOpsTable::getInstance()},
        m_helper{m_ops},
        m_construction{m_ops.getKernelConstructionOps(), construction},
        m_status{m_ops.getStatusOps()},
        m_scratch_name{m_ops.getStringOps()}
    {
        m_status.create();
        m_scratch_name.create();
    }

    ~SyclKernelConstruction()
    {
        m_scratch_name.destroy();
        m_status.destroy();
    }

    SyclKernelConstruction(const SyclKernelConstruction&) = delete;
    SyclKernelConstruction& operator=(const SyclKernelConstruction&) = delete;
    SyclKernelConstruction(SyclKernelConstruction&&) = delete;
    SyclKernelConstruction& operator=(SyclKernelConstruction&&) = delete;

    bool has_attribute(std::string_view name)
    {
        _Bool found = false;
        m_construction.has_attr(name_of(name), &found, m_status);
        return found;
    }

    int64_t getInt64(std::string_view name, int64_t fallback)
    {
        if (!has_attribute(name)) {
            return fallback;
        }
        int64_t value = fallback;
        m_construction.get_attr_int64(name_of(name), &value, m_status);
        return value;
    }

    float getFloat(std::string_view name, float fallback)
    {
        if (!has_attribute(name)) {
            return fallback;
        }
        float value = fallback;
        m_construction.get_attr_float(name_of(name), &value, m_status);
        return value;
    }

    bool getBool(std::string_view name, bool fallback)
    {
        if (!has_attribute(name)) {
            return fallback;
        }
        _Bool value = fallback;
        m_construction.get_attr_bool(name_of(name), &value, m_status);
        return value;
    }

    TFDataTypeEnum getType(std::string_view name, TFDataTypeEnum fallback)
    {
        if (!has_attribute(name)) {
            return fallback;
        }
        TFDataTypeEnum value = fallback;
        m_construction.get_attr_type(name_of(name), &value, m_status);
        return value;
    }

    std::size_t getInt64List(std::string_view name, std::span<int64_t> out_values)
    {
        if (!has_attribute(name)) {
            return 0;
        }
        int32_t list_size = 0;
        int32_t total_size = 0;
        m_construction.get_attr_size(name_of(name), &list_size, &total_size, m_status);
        const auto count =
            std::min(out_values.size(), static_cast<std::size_t>(std::max(list_size, 0)));
        m_construction.get_attr_int64_list(
            name_of(name),
            out_values.data(),
            static_cast<int>(count),
            m_status
        );
        return count;
    }

    std::size_t getFloatList(std::string_view name, std::span<float> out_values)
    {
        if (!has_attribute(name)) {
            return 0;
        }
        int32_t list_size = 0;
        int32_t total_size = 0;
        m_construction.get_attr_size(name_of(name), &list_size, &total_size, m_status);
        const auto count =
            std::min(out_values.size(), static_cast<std::size_t>(std::max(list_size, 0)));
        m_construction.get_attr_float_list(
            name_of(name),
            out_values.data(),
            static_cast<int>(count),
            m_status
        );
        return count;
    }

    std::string getString(std::string_view name, std::string_view fallback)
    {
        if (!has_attribute(name)) {
            return std::string{fallback};
        }
        ice::sonic::String value{m_ops.getStringOps()};
        value.create();
        m_construction.get_attr_string(name_of(name), value, m_status);
        const char* data = nullptr;
        std::size_t size = 0;
        value.get_data_pointer(&data);
        value.get_size(&size);
        std::string result{data, size};
        value.destroy();
        return result;
    }

    bool ok() const noexcept
    {
        TF_Code code = TF_OK;
        m_status.get_code(&code);
        return code == TF_OK;
    }

    void fail(TF_Code code, std::string_view message)
    {
        m_helper.fail(m_status, code, message);
        m_construction.failure(m_status);
    }

private:
    const ice::sonic::String& name_of(std::string_view name)
    {
        m_scratch_name.copy(name.data(), name.size());
        return m_scratch_name;
    }

    const SyclOpsTable& m_ops;
    SyclStatus m_helper;
    ice::sonic::TF_OpKernelConstructionOps m_construction;
    ice::sonic::Status m_status;
    ice::sonic::String m_scratch_name;
};

} // namespace aten_xpu

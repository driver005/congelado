module;

#include "include/c/extern/filesystem/filesystem.h"
#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/extern/filesystem/tree.h"
#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/extern/gradients/gradient.h"
#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/grappler/function_library.h"
#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/grappler/properties.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"
#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
#include "include/c/extern/profiler/profiler.h"
#include "include/c/extern/stream_executor/memory.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/extern/stream_executor/stream_executor.h"
#include "include/c/intern/array.h"
#include "include/c/intern/attrtype.h"
#include "include/c/intern/bitset.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/complex.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/deque.h"
#include "include/c/intern/duration.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/hash.h"
#include "include/c/intern/hive/hive.h"
#include "include/c/intern/list/list.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/set.h"
#include "include/c/intern/shape.h"
#include "include/c/intern/span.h"
#include "include/c/intern/status.h"
#include "include/c/intern/time_point.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_registration:registration;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;
import aten_xpu_extern_random_generator;
import aten_xpu_extern_grappler;
import aten_xpu_extern_kernel;
import aten_xpu_extern_gradients;
import aten_xpu_extern_filesystem;
import aten_xpu_profiler_sycl;

export namespace aten_xpu {

inline constexpr std::string_view k_plugin_name = "aten_xpu";
inline constexpr std::string_view k_plugin_version = "0.1.0";

class SyclPluginRegistry
{
public:
    SyclPluginRegistry(const SyclPluginRegistry&) = delete;
    SyclPluginRegistry& operator=(const SyclPluginRegistry&) = delete;
    SyclPluginRegistry(SyclPluginRegistry&&) = delete;
    SyclPluginRegistry& operator=(SyclPluginRegistry&&) = delete;

    static SyclPluginRegistry& getInstance()
    {
        static SyclPluginRegistry registry;
        return registry;
    }

    void initialize()
    {
        if (m_initialized) {
            return;
        }
        m_initialized = true;
        load_host_ops();
        load_plugin_ops();
        register_ops();
        register_kernels();
        register_gradients();
    }

    SyclOpsTable& getOps() noexcept
    {
        return m_ops;
    }

    const ::TF_PlatformOps& getPlatformOps() const noexcept
    {
        return m_platform.get_vtable();
    }

    const ::TF_MemoryOps& getMemoryOps() const noexcept
    {
        return m_memory.get_vtable();
    }

    const ::TF_StreamExecutorOps& getStreamExecutorOps() const noexcept
    {
        return m_stream_executor.get_vtable();
    }

    const ::TF_GrapplerOps& getGrapplerOps() const noexcept
    {
        return m_grappler.get_vtable();
    }

    const ::TFGrapplerConfigsOps& getConfigsOps() const noexcept
    {
        return m_configs.get_vtable();
    }

    const ::TFGrapplerOptimizerOps& getOptimizerOps() const noexcept
    {
        return m_optimizer.get_vtable();
    }

    const ::TF_ProfilerOps& getProfilerOps() const noexcept
    {
        return m_profiler.get_vtable();
    }

    const ::TF_StringOps& getStringOps() const noexcept
    {
        return m_string.get_vtable();
    }

    const ::TF_StatusOps& getStatusOps() const noexcept
    {
        return m_status.get_vtable();
    }

    const ::TF_BufferOps& getBufferOps() const noexcept
    {
        return m_buffer.get_vtable();
    }

    const ::TF_RegistrationOps& getRegistrationOps() const noexcept
    {
        return m_registration.get_vtable();
    }

    const ::TF_ComplexOps& getComplexOps() const noexcept
    {
        return m_complex.get_vtable();
    }

    const ::TF_DurationOps& getDurationOps() const noexcept
    {
        return m_duration.get_vtable();
    }

    const ::TF_TimePointOps& getTimePointOps() const noexcept
    {
        return m_time_point.get_vtable();
    }

    const ::TF_DataTypeOps& getDataTypeOps() const noexcept
    {
        return m_data_type.get_vtable();
    }

    const ::TF_AttrTypeOps& getAttrTypeOps() const noexcept
    {
        return m_attr_type.get_vtable();
    }

    const ::TF_ShapeOps& getShapeOps() const noexcept
    {
        return m_shape.get_vtable();
    }

    const ::TF_FileStatisticsOps& getFileStatisticsOps() const noexcept
    {
        return m_file_statistics.get_vtable();
    }

    const ::TF_HashOps& getHashOps() const noexcept
    {
        return m_hash.get_vtable();
    }

    const ::TF_ArrayOps& getArrayOps() const noexcept
    {
        return m_array.get_vtable();
    }

    const ::TF_VectorOps& getVectorOps() const noexcept
    {
        return m_vector.get_vtable();
    }

    const ::TF_DequeOps& getDequeOps() const noexcept
    {
        return m_deque.get_vtable();
    }

    const ::TF_ListOps& getListOps() const noexcept
    {
        return m_list.get_vtable();
    }

    const ::TF_ForwardListOps& getForwardListOps() const noexcept
    {
        return m_forward_list.get_vtable();
    }

    const ::TF_HiveOps& getHiveOps() const noexcept
    {
        return m_hive.get_vtable();
    }

    const ::TF_BitSetOps& getBitSetOps() const noexcept
    {
        return m_bit_set.get_vtable();
    }

    const ::TF_SpanOps& getSpanOps() const noexcept
    {
        return m_span.get_vtable();
    }

    const ::TF_MapOps& getMapOps() const noexcept
    {
        return m_map.get_vtable();
    }

    const ::TF_SetOps& getSetOps() const noexcept
    {
        return m_set.get_vtable();
    }

    const ::TF_FilesystemOps& getFilesystemOps() const noexcept
    {
        return m_filesystem.get_vtable();
    }

    const ::TF_RandomAccessFileOps& getRandomAccessFileOps() const noexcept
    {
        return m_random_access_file.get_vtable();
    }

    const ::TF_WritableFileOps& getWritableFileOps() const noexcept
    {
        return m_writable_file.get_vtable();
    }

    const ::TF_ReadOnlyMemoryRegionOps& getReadOnlyMemoryRegionOps() const noexcept
    {
        return m_read_only_memory_region.get_vtable();
    }

    const ::TFFilesystemTreeOps& getFilesystemTreeOps() const noexcept
    {
        return m_filesystem_tree.get_vtable();
    }

    // The process-wide registry; every gradient is registered in it with type "gradient".
    const ice::sonic::Registration& getGlobalRegistry() const noexcept
    {
        return *m_global_registry;
    }

    const ::TFGradientOps* getGradientOps() const noexcept
    {
        return m_gradients.findDefault(*m_global_registry);
    }

    std::size_t getOpCount() const noexcept
    {
        return m_op_count;
    }

    std::size_t getKernelCount() const noexcept
    {
        return m_kernel_count;
    }

private:
    static constexpr std::size_t k_default_bits = 64;
    static constexpr std::size_t k_default_key_size = 8;
    static constexpr std::size_t k_default_value_size = 8;

    SyclPluginRegistry() :
        m_ops{SyclOpsTable::getInstance()},
        m_platform{m_ops},
        m_device{m_ops},
        m_executor{m_ops},
        m_stream{m_ops},
        m_event{m_ops},
        m_allocator{m_ops},
        m_mem_pool{m_ops},
        m_memory{m_ops},
        m_stream_executor{m_ops},
        m_random_generator{m_ops},
        m_tensor{m_ops},
        m_device_graph{m_ops},
        m_grappler{m_ops},
        m_optimizer{m_ops},
        m_configs{m_ops},
        m_profiler{m_ops},
        m_string{},
        m_status{&m_string.get_vtable()},
        m_buffer{&m_string.get_vtable()},
        m_registration{&m_string.get_vtable()},
        m_gradients{m_ops},
        m_complex{},
        m_duration{},
        m_time_point{&m_duration.get_vtable()},
        m_data_type{m_ops.getStringOps()},
        m_attr_type{m_ops.getStringOps()},
        m_shape{m_ops.getStringOps()},
        m_file_statistics{m_ops.getStringOps()},
        m_hash{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_array{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_vector{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_deque{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_list{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_forward_list{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_hive{m_ops.getStatusOps(), m_ops.getStringOps()},
        m_bit_set{m_ops.getStatusOps(), m_ops.getStringOps(), k_default_bits},
        m_span{nullptr, m_ops.getStatusOps(), m_ops.getStringOps()},
        m_map{m_ops.getStatusOps(), m_ops.getStringOps(), k_default_key_size, k_default_value_size},
        m_set{m_ops.getStatusOps(), m_ops.getStringOps(), k_default_key_size},
        m_filesystem{m_ops.getStringOps()},
        m_random_access_file{m_ops.getStatusOps(), m_ops.getStringOps(), std::string{}},
        m_writable_file{m_ops.getStatusOps(), m_ops.getStringOps(), std::string{}, false},
        m_read_only_memory_region{m_ops.getStringOps(), std::string{}},
        m_filesystem_tree{
            &m_file_statistics.get_vtable(),
            m_ops.getStatusOps(),
            m_ops.getStringOps(),
            nullptr
        }
    {
        load_host_types();
    }

    ~SyclPluginRegistry() = default;

    // The host type prototypes; each create slot binds to the effective host status and string ops.
    void load_host_types()
    {
        m_string.get_generic_vtable(
            [](::TF_String* handle)
            {
                auto* object = new String{};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_status.get_generic_vtable(
            [](::TF_Status* handle)
            {
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object = new Status{&registry.m_string.get_vtable()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_buffer.get_generic_vtable(
            [](::TF_Buffer* handle)
            {
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object = new Buffer{&registry.m_string.get_vtable()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_registration.get_generic_vtable(
            [](::TF_Registration* handle)
            {
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object = new Registration{&registry.m_string.get_vtable()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_complex.get_generic_vtable(
            [](::TF_Complex* handle)
            {
                auto* object = new Complex{};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_duration.get_generic_vtable(
            [](::TF_Duration* handle)
            {
                auto* object = new Duration{};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_time_point.get_generic_vtable(
            [](::TF_TimePoint* handle)
            {
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object = new TimePoint{&registry.m_duration.get_vtable()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_data_type.get_generic_vtable(
            [](::TF_DataType* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new DataType{ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_attr_type.get_generic_vtable(
            [](::TF_AttrType* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new AttrType{ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_shape.get_generic_vtable(
            [](::TF_Shape* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Shape{ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_file_statistics.get_generic_vtable(
            [](::TF_FileStatistics* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new FileStatistics{ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_hash.get_generic_vtable(
            [](::TF_Hash* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Hash{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_array.get_generic_vtable(
            [](::TF_Array* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Array{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_vector.get_generic_vtable(
            [](::TF_Vector* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Vector{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_deque.get_generic_vtable(
            [](::TF_Deque* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Deque{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_list.get_generic_vtable(
            [](::TF_List* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new List{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_forward_list.get_generic_vtable(
            [](::TF_ForwardList* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new ForwardList{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_hive.get_generic_vtable(
            [](::TF_Hive* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Hive{ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_bit_set.get_generic_vtable(
            [](::TF_BitSet* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new BitSet{ops.getStatusOps(), ops.getStringOps(), k_default_bits};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_span.get_generic_vtable(
            [](::TF_Span* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object =
                    new Span{&registry.m_span.get_vtable(), ops.getStatusOps(), ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_map.get_generic_vtable(
            [](::TF_Map* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Map{
                    ops.getStatusOps(),
                    ops.getStringOps(),
                    k_default_key_size,
                    k_default_value_size
                };
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_set.get_generic_vtable(
            [](::TF_Set* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Set{ops.getStatusOps(), ops.getStringOps(), k_default_key_size};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_filesystem.get_generic_vtable(
            [](::TF_Filesystem* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new Filesystem{ops.getStringOps()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_random_access_file.get_generic_vtable(
            [](::TF_RandomAccessFile* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object =
                    new RandomAccessFile{ops.getStatusOps(), ops.getStringOps(), std::string{}};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_writable_file.get_generic_vtable(
            [](::TF_WritableFile* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object =
                    new WritableFile{ops.getStatusOps(), ops.getStringOps(), std::string{}, false};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_read_only_memory_region.get_generic_vtable(
            [](::TF_ReadOnlyMemoryRegion* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto* object = new ReadOnlyMemoryRegion{ops.getStringOps(), std::string{}};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_filesystem_tree.get_generic_vtable(
            [](::TFFilesystemTree* handle)
            {
                const auto& ops = SyclOpsTable::getInstance();
                auto& registry = SyclPluginRegistry::getInstance();
                auto* object = new FilesystemTree{
                    &registry.m_file_statistics.get_vtable(),
                    ops.getStatusOps(),
                    ops.getStringOps(),
                    ops.getTensorOps()
                };
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
    }

    void load_host_ops()
    {
        ::TF_StatusOps* status_ops = nullptr;
        ::TF_StringOps* string_ops = nullptr;
        ::TF_BufferOps* buffer_ops = nullptr;
        ::TF_OpKernelContextOps* context_ops = nullptr;
        ::TF_OpKernelConstructionOps* construction_ops = nullptr;
        ::TFGrapplerItemOps* item_ops = nullptr;
        ::TF_ShapeInferenceContextOps* shape_context_ops = nullptr;
        ::TF_ShapeHandleOps* shape_handle_ops = nullptr;
        ::TFGrapplerFunctionLibraryOps* function_library_ops = nullptr;
        ::TFGrapplerPropertiesOps* properties_ops = nullptr;
        ::TFGradientContextOps* gradient_context_ops = nullptr;
        create_status(&status_ops, &m_host_contexts[0], nullptr);
        create_string(&string_ops, &m_host_contexts[1], nullptr);
        create_buffer(&buffer_ops, &m_host_contexts[2], nullptr);
        create_op_kernel_context(&context_ops, &m_host_contexts[3], nullptr);
        create_op_kernel_construction(&construction_ops, &m_host_contexts[4], nullptr);
        create_grappler_item(&item_ops, &m_host_contexts[5], nullptr);
        create_shape_inference_context(&shape_context_ops, &m_host_contexts[6], nullptr);
        create_shape_handle(&shape_handle_ops, &m_host_contexts[7], nullptr);
        create_grappler_function_library(&function_library_ops, &m_host_contexts[8], nullptr);
        create_grappler_properties(&properties_ops, &m_host_contexts[9], nullptr);
        create_gradient_context(&gradient_context_ops, &m_host_contexts[10], nullptr);

        m_ops.setStatusOps(status_ops);
        m_ops.setStringOps(string_ops);
        m_ops.setBufferOps(buffer_ops);
        m_ops.setKernelContextOps(context_ops);
        m_ops.setKernelConstructionOps(construction_ops);
        m_ops.setGrapplerItemOps(item_ops);
        m_ops.setShapeInferenceContextOps(shape_context_ops);
        m_ops.setShapeHandleOps(shape_handle_ops);
        m_ops.setGrapplerFunctionLibraryOps(function_library_ops);
        m_ops.setGrapplerPropertiesOps(properties_ops);
        m_ops.setGradientContextOps(gradient_context_ops);
    }

    void load_plugin_ops()
    {
        m_platform.get_generic_vtable(
            [](::TF_Platform* handle)
            {
                auto* object = new SyclPlatform{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_device.get_generic_vtable(
            [](::TF_Device* handle)
            {
                auto* object = new SyclDevice{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_executor.get_generic_vtable(
            [](::TF_Executor* handle)
            {
                auto* object = new SyclExecutor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_stream.get_generic_vtable(
            [](::TF_Stream* handle)
            {
                auto* object = new SyclStream{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_event.get_generic_vtable(
            [](::TF_Event* handle)
            {
                auto* object = new SyclEvent{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_timer.get_generic_vtable(
            [](::TF_Timer* handle)
            {
                auto* object = new SyclTimer{};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_allocator.get_generic_vtable(
            [](::TF_Allocator* handle)
            {
                auto* object = new SyclAllocator{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_mem_pool.get_generic_vtable(
            [](::TF_MemPool* handle)
            {
                auto* object = new SyclMemPool{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_memory.get_generic_vtable(
            [](::TF_Memory* handle)
            {
                auto* object = new SyclMemory{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_stream_executor.get_generic_vtable(
            [](::TF_StreamExecutor* handle)
            {
                auto* object = new SyclStreamExecutor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_random_generator.get_generic_vtable(
            [](::TF_RandomGenerator* handle)
            {
                auto* object = new SyclRandomGenerator{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_tensor.get_generic_vtable(
            [](::TF_Tensor* handle)
            {
                auto* object = new SyclTensor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_device_graph.get_generic_vtable(
            [](::TFGrapplerDeviceGraph* handle)
            {
                auto* object = new SyclDeviceGraph{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_grappler.get_generic_vtable(
            [](::TF_Grappler* handle)
            {
                auto* object = new SyclGrappler{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_optimizer.get_generic_vtable(
            [](::TFGrapplerOptimizer* handle)
            {
                auto* object = new SyclGrapplerOptimizer{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );

        m_configs.get_generic_vtable(
            [](::TFGrapplerConfigs* handle)
            {
                auto* object = new SyclGrapplerConfigs{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );

        m_profiler.get_generic_vtable(
            [](::TF_Profiler* handle)
            {
                auto* object = new SyclProfiler{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );

        m_ops.setDeviceOps(&m_device.get_vtable());
        m_ops.setExecutorOps(&m_executor.get_vtable());
        m_ops.setStreamOps(&m_stream.get_vtable());
        m_ops.setEventOps(&m_event.get_vtable());
        m_ops.setTimerOps(&m_timer.get_vtable());
        m_ops.setAllocatorOps(&m_allocator.get_vtable());
        m_ops.setMemPoolOps(&m_mem_pool.get_vtable());
        m_ops.setRandomGeneratorOps(&m_random_generator.get_vtable());
        m_ops.setTensorOps(&m_tensor.get_vtable());
        m_ops.setKernelRuntime(&m_kernel_runtime);
        m_ops.setDeviceGraphOps(&m_device_graph.get_vtable());

        load_device_info();
        if (m_platform.getDeviceCount() > 0) {
            m_ops.getPlacement()
                .setPlacement(m_platform.getContext(), m_platform.getNativeDevice(0), 0);
        }
    }

    // The memory size and bandwidth of device 0, for the optimizers that budget memory and
    // replicas.
    void load_device_info()
    {
        const int count = m_platform.getDeviceCount();
        if (count <= 0) {
            return;
        }
        const sycl::device& device = m_platform.getNativeDevice(0);
        const std::uint64_t clock_mhz =
            device.has(sycl::aspect::ext_intel_memory_clock_rate)
                ? device.get_info<sycl::ext::intel::info::device::memory_clock_rate>()
                : 0;
        const std::uint64_t bus_bits =
            device.has(sycl::aspect::ext_intel_memory_bus_width)
                ? device.get_info<sycl::ext::intel::info::device::memory_bus_width>()
                : 0;
        // Double data rate: two transfers per clock.
        m_ops.setDeviceInfo(
            device.get_info<sycl::info::device::global_mem_size>(),
            clock_mhz * 1'000'000 * (bus_bits / 8) * 2,
            count
        );
    }

    void register_ops()
    {
        SyclOpRegistrar registrar{m_ops};
        SyclOpCatalog catalog{registrar};
        catalog.registerAll();
        m_op_count = registrar.getRegisteredCount();
    }

    void register_kernels()
    {
        SyclKernelRegistrar registrar{m_ops};
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_float,
              ice::sonic::DataTypeEnum::k_half,
              ice::sonic::DataTypeEnum::k_bfloat16}) {
            registrar.add<SyclAddmmKernel>(k_addmm_kernel_name, type);
            registrar.add<SyclBmmKernel>(k_bmm_kernel_name, type);
            registrar.add<SyclBaddbmmKernel>(k_baddbmm_kernel_name, type);
            registrar.add<SyclAddmvKernel>(k_addmv_kernel_name, type);
            registrar.add<SyclLinearKernel>(k_linear_kernel_name, type);
            registrar.add<SyclConvolutionKernel>(k_convolution_kernel_name, type);
            registrar.add<SyclConvolutionKernel>(k_convolution_3d_kernel_name, type);
            registrar.add<SyclConvolutionBackwardKernel>(k_convolution_backward_kernel_name, type);
            registrar.add<SyclDeconvolutionKernel>(k_deconvolution_kernel_name, type);
            registrar.add<SyclWeightOnlyMatmulKernel>(k_weight_only_matmul_kernel_name, type);
            registrar.add<SyclSdpaKernel>(k_sdpa_kernel_name, type);
            registrar.add<SyclSdpaBackwardKernel>(k_sdpa_backward_kernel_name, type);
            registrar.add<SyclLstmKernel>(k_lstm_kernel_name, type);
        }
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_float,
              ice::sonic::DataTypeEnum::k_half,
              ice::sonic::DataTypeEnum::k_bfloat16,
              ice::sonic::DataTypeEnum::k_int32,
              ice::sonic::DataTypeEnum::k_int64,
              ice::sonic::DataTypeEnum::k_uint8}) {
            registrar.add<SyclTransposeKernel>(k_transpose_kernel_name, type);
        }
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_float,
              ice::sonic::DataTypeEnum::k_half,
              ice::sonic::DataTypeEnum::k_bfloat16,
              ice::sonic::DataTypeEnum::k_int32,
              ice::sonic::DataTypeEnum::k_int64,
              ice::sonic::DataTypeEnum::k_uint8}) {
            registrar.add<SyclCopyToHostKernel>(k_copy_to_host_kernel_name, type);
            registrar.add<SyclCopyFromHostKernel>(k_copy_from_host_kernel_name, type);
        }
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_float,
              ice::sonic::DataTypeEnum::k_half,
              ice::sonic::DataTypeEnum::k_bfloat16}) {
            registrar.add<SyclScopedAllocatorKernel>(k_scoped_allocator_kernel_name, type);
            registrar.add<SyclScopedAllocatorConcatKernel>(
                k_scoped_allocator_concat_kernel_name,
                type
            );
            registrar.add<SyclScopedAllocatorSplitKernel>(
                k_scoped_allocator_split_kernel_name,
                type
            );
        }
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_qint8, ice::sonic::DataTypeEnum::k_quint8}) {
            registrar.add<SyclInt8MatmulKernel>(k_int8_matmul_kernel_name, type);
            registrar.add<SyclInt8MatmulKernel>("QuantizedLinear", type);
            registrar.add<SyclInt8ConvolutionKernel>(k_int8_convolution_kernel_name, type);
        }
        for (const auto type:
             {ice::sonic::DataTypeEnum::k_float8_e4m3fn, ice::sonic::DataTypeEnum::k_float8_e5m2}) {
            registrar.add<SyclScaledMatmulKernel>(k_scaled_matmul_kernel_name, type);
        }
        m_kernel_count = registrar.getRegisteredCount();
    }

    void register_gradients()
    {
        m_global_registry.emplace(&m_registration.get_vtable());
        m_global_registry->create();
        m_gradients.registerAll(*m_global_registry);
    }

    SyclOpsTable& m_ops;
    SyclKernelRuntime m_kernel_runtime;
    std::array<void*, 11> m_host_contexts{};
    bool m_initialized{false};
    std::size_t m_kernel_count{0};
    std::size_t m_op_count{0};
    SyclPlatform m_platform;
    SyclDevice m_device;
    SyclExecutor m_executor;
    SyclStream m_stream;
    SyclEvent m_event;
    SyclTimer m_timer;
    SyclAllocator m_allocator;
    SyclMemPool m_mem_pool;
    SyclMemory m_memory;
    SyclStreamExecutor m_stream_executor;
    SyclRandomGenerator m_random_generator;
    SyclTensor m_tensor;
    SyclDeviceGraph m_device_graph;
    SyclGrappler m_grappler;
    SyclGrapplerOptimizer m_optimizer;
    SyclGrapplerConfigs m_configs;
    SyclProfiler m_profiler;
    String m_string;
    Status m_status;
    Buffer m_buffer;
    Registration m_registration;
    std::optional<ice::sonic::Registration> m_global_registry;
    SyclGradientCatalog m_gradients;
    Complex m_complex;
    Duration m_duration;
    TimePoint m_time_point;
    DataType m_data_type;
    AttrType m_attr_type;
    Shape m_shape;
    FileStatistics m_file_statistics;
    Hash m_hash;
    Array m_array;
    Vector m_vector;
    Deque m_deque;
    List m_list;
    ForwardList m_forward_list;
    Hive m_hive;
    BitSet m_bit_set;
    Span m_span;
    Map m_map;
    Set m_set;
    Filesystem m_filesystem;
    RandomAccessFile m_random_access_file;
    WritableFile m_writable_file;
    ReadOnlyMemoryRegion m_read_only_memory_region;
    FilesystemTree m_filesystem_tree;
};

} // namespace aten_xpu

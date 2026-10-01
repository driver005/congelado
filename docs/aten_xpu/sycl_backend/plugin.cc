// SYCL reference plugin — entry points only. Every capability implementation lives in its own
// *.cppm (per repo convention); this file just exports the C symbols the host dlsyms and wires
// them to those classes.
//
// Not built by Bazel (docs/ only).
//
// Which entry point a real host actually calls is still unresolved in this repo (see the parent
// plan's blocker list: create_plugin vs init_plugin vs congelado_init). This file implements
// create_plugin (include/c/extern/plugin/registration.h) since it is what
// docs/aten_xpu/torch_csrc/Module.h already assumes, and does kernel registration there, since
// nothing else in the C ABI is documented as "called once at load".
//
// g_platform is the one process-wide instance this plugin's whole object graph hangs off —
// SyclDevice/SyclExecutor/SyclMemory/SyclAllocator/... are all created later, per-call, through
// the factory slots (create_device_internal etc.); this is not a substitute for that, it is the
// single required entry point every .so-style plugin has exactly one of.

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/kernel/builder.h"
#include "include/c/extern/plugin/registration.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/memory.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/timer.h"

import std;
import sycl_backend;
import cc_ice_extern_stream_executor_builder;
import cc_ice_extern_memory_builder;
import cc_ice_extern_random_generator_builder;
import cc_ice_extern_grappler_builder;

namespace {

sycl_backend::SyclPlatform& platform()
{
    static sycl_backend::SyclPlatform instance;
    return instance;
}

sycl_backend::SyclMemory& memory()
{
    static sycl_backend::SyclMemory instance{platform()};
    return instance;
}

void register_kernel(
    const char* op_name,
    void (*create_func)(TF_OpKernelConstruction*, void**),
    void (*compute_func)(void*, TF_OpKernelContext*),
    void (*delete_func)(void*)
)
{
    TF_KernelBuilderOps* ops = nullptr;
    void* plugin_context = nullptr;
    TF_Status status{};

    create_kernel_builder(
        &ops,
        &plugin_context,
        op_name,
        "XPU",
        create_func,
        compute_func,
        delete_func,
        &status
    );

    if (ops == nullptr) {
        return;
    }

    TF_KernelBuilder builder{plugin_context};
    ops->register_kernel_builder(&builder, op_name, &status);
}

void no_create(TF_OpKernelConstruction*, void** out_plugin_data)
{
    *out_plugin_data = nullptr;
}

void no_delete(void*) {}

void compute_addmm(void* plugin_data, TF_OpKernelContext* context)
{
    sycl_backend::kernels::MatmulKernel::compute(plugin_data, context, /*with_bias=*/true);
}

void compute_bmm(void* plugin_data, TF_OpKernelContext* context)
{
    sycl_backend::kernels::MatmulKernel::compute(plugin_data, context, /*with_bias=*/false);
}

} // namespace

extern "C"
{
    TF_CAPI_EXPORT void create_plugin(TF_PluginInfo* plugin_info)
    {
        plugin_info->name = "sycl_backend";
        plugin_info->version = "0.0.1-reference";

        register_kernel(
            "Addmm",
            &sycl_backend::kernels::MatmulKernel::create,
            &compute_addmm,
            &sycl_backend::kernels::MatmulKernel::destroy
        );
        register_kernel(
            "Bmm",
            &sycl_backend::kernels::MatmulKernel::create,
            &compute_bmm,
            &sycl_backend::kernels::MatmulKernel::destroy
        );
        register_kernel(
            "Conv2D",
            &sycl_backend::kernels::ConvKernel::create,
            &sycl_backend::kernels::ConvKernel::compute,
            &sycl_backend::kernels::ConvKernel::destroy
        );
        register_kernel(
            "Linear",
            no_create,
            &sycl_backend::kernels::LinearKernel::compute,
            no_delete
        );
        register_kernel(
            "ScaledDotProductAttention",
            no_create,
            &sycl_backend::kernels::SdpaKernel::compute,
            no_delete
        );
        register_kernel(
            "ScaledDotProductAttentionBackward",
            no_create,
            &sycl_backend::kernels::SdpaBackwardKernel::compute,
            no_delete
        );
        register_kernel(
            "ConvTranspose2D",
            &sycl_backend::kernels::DeconvKernel::create,
            &sycl_backend::kernels::DeconvKernel::compute,
            &sycl_backend::kernels::DeconvKernel::destroy
        );
        register_kernel(
            "Int8Conv2D",
            &sycl_backend::kernels::Int8ConvKernel::create,
            &sycl_backend::kernels::Int8ConvKernel::compute,
            &sycl_backend::kernels::Int8ConvKernel::destroy
        );
        register_kernel(
            "Int8Matmul",
            no_create,
            &sycl_backend::kernels::Int8MatmulKernel::compute,
            no_delete
        );
        register_kernel(
            "WoqMatmulInt4",
            &sycl_backend::kernels::WoqMatmulKernel::create,
            &sycl_backend::kernels::WoqMatmulKernel::compute,
            &sycl_backend::kernels::WoqMatmulKernel::destroy
        );
        register_kernel(
            "ScaledMm",
            no_create,
            &sycl_backend::kernels::ScaledMmKernel::compute,
            no_delete
        );
        register_kernel(
            "LstmForward",
            no_create,
            &sycl_backend::kernels::RnnLstmKernel::compute,
            no_delete
        );
    }

    TF_CAPI_EXPORT void create_platform(TF_PlatformOps** ops, void** plugin_context, TF_Status*)
    {
        *ops = ice::builder::Platform::get_generic_vtable();
        *plugin_context = &platform();
    }

    TF_CAPI_EXPORT void destroy_platform(void*) {}

    TF_CAPI_EXPORT void create_device(TF_DeviceOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Device::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_device(void*) {}

    TF_CAPI_EXPORT void create_executor(TF_ExecutorOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Executor::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_executor(void*) {}

    TF_CAPI_EXPORT void create_stream(TF_StreamOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Stream::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_stream(void*) {}

    TF_CAPI_EXPORT void create_event(TF_EventOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Event::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_event(void*) {}

    TF_CAPI_EXPORT void create_timer(TF_TimerOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Timer::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_timer(void*) {}

    TF_CAPI_EXPORT void create_memory(TF_MemoryOps** ops, void** plugin_context, TF_Status*)
    {
        *ops = ice::builder::Memory::get_generic_vtable();
        *plugin_context = &memory();
    }

    TF_CAPI_EXPORT void destroy_memory(void*) {}

    TF_CAPI_EXPORT void create_allocator(TF_AllocatorOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::Allocator::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_allocator(void*) {}

    TF_CAPI_EXPORT void create_mem_pool(TF_MemPoolOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::MemPool::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_mem_pool(void*) {}

    TF_CAPI_EXPORT void create_random_generator(TF_RandomGeneratorOps** ops, void**, TF_Status*)
    {
        *ops = ice::builder::RandomGenerator::get_generic_vtable();
    }

    TF_CAPI_EXPORT void destroy_random_generator(void*) {}

    TF_CAPI_EXPORT void
    create_grappler_device_graph(TFGrapplerDeviceGraphOps** ops, void** plugin_context, TF_Status*)
    {
        *ops = ice::builder::TFGrapplerDeviceGraph::get_generic_vtable();
        *plugin_context = new sycl_backend::SyclDeviceGraph{};
    }

    TF_CAPI_EXPORT void destroy_grappler_device_graph(void* plugin_context)
    {
        delete static_cast<sycl_backend::SyclDeviceGraph*>(plugin_context);
    }

    TF_CAPI_EXPORT void
    create_grappler_optimizer(TFGrapplerOptimizerOps** ops, void** plugin_context, TF_Status*)
    {
        *ops = ice::builder::TFGrapplerOptimizer::get_generic_vtable();
        *plugin_context = new sycl_backend::SyclGrapplerOptimizer{};
    }

    TF_CAPI_EXPORT void destroy_grappler_optimizer(void* plugin_context)
    {
        delete static_cast<sycl_backend::SyclGrapplerOptimizer*>(plugin_context);
    }

} // extern "C"

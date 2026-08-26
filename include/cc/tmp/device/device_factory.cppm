module;

/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "absl/base/attributes.h"
#include "absl/container/flat_hash_set.h"
#include "tensorflow/core/framework/device.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session_options.h"
#include "tensorflow/core/util/device_name_utils.h"
#include "tensorflow/core/util/env_var.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

export module cc_tmp:device_device_factory;

import std;
import cc_abi;

export {

    namespace tensorflow {

        class Device;
        struct SessionOptions;

        class DeviceFactory
        {
        public:
            virtual ~DeviceFactory() = default;
            static void Register(
                const std::string& device_type,
                std::unique_ptr<DeviceFactory> factory,
                int priority,
                bool is_pluggable_device
            );
            ABSL_DEPRECATED("Use the `Register` function above instead")

            static void Register(
                const std::string& device_type,
                DeviceFactory* factory,
                int priority,
                bool is_pluggable_device
            )
            {
                Register(
                    device_type, std::unique_ptr<DeviceFactory>(factory), priority,
                    is_pluggable_device
                );
            }

            static DeviceFactory* GetFactory(const std::string& device_type);

            // Append to "*devices" CPU devices.
            static absl::Status AddCpuDevices(
                const SessionOptions& options,
                const std::string& name_prefix,
                std::vector<std::unique_ptr<Device>>* devices
            );

            // Append to "*devices" all suitable devices, respecting
            // any device type specific properties/counts listed in "options".
            //
            // CPU devices are added first.
            static absl::Status AddDevices(
                const SessionOptions& options,
                const std::string& name_prefix,
                std::vector<std::unique_ptr<Device>>* devices
            );

            // Helper for tests.  Create a single device of type "type".  The
            // returned device is always numbered zero, so if creating multiple
            // devices of the same type, supply distinct name_prefix arguments.
            static std::unique_ptr<Device> NewDevice(
                const std::string& type,
                const SessionOptions& options,
                const std::string& name_prefix
            );

            // Iterate through all device factories and build a list of all of the
            // possible physical devices.
            //
            // CPU is are added first.
            static absl::Status ListAllPhysicalDevices(std::vector<std::string>* devices);

            // Iterate through all device factories and build a list of all of the
            // possible pluggable physical devices.
            static absl::Status ListPluggablePhysicalDevices(std::vector<std::string>* devices);

            // Get details for a specific device among all device factories.
            // 'device_index' indexes into devices from ListAllPhysicalDevices.
            static absl::Status GetAnyDeviceDetails(
                int device_index, std::unordered_map<std::string, std::string>* details
            );

            // For a specific device factory list all possible physical devices.
            virtual absl::Status ListPhysicalDevices(std::vector<std::string>* devices) = 0;

            // Get details for a specific device for a specific factory. Subclasses
            // can store arbitrary device information in the map. 'device_index' indexes
            // into devices from ListPhysicalDevices.
            virtual absl::Status GetDeviceDetails(
                int device_index, std::unordered_map<std::string, std::string>* details
            )
            {
                return absl::OkStatus();
            }

            // Most clients should call AddDevices() instead.
            virtual absl::Status CreateDevices(
                const SessionOptions& options,
                const std::string& name_prefix,
                std::vector<std::unique_ptr<Device>>* devices
            ) = 0;

            // Return the device priority number for a "device_type" string.
            //
            // Higher number implies higher priority.
            //
            // In standard TensorFlow distributions, GPU device types are
            // preferred over CPU, and by default, custom devices that don't set
            // a custom priority during registration will be prioritized lower
            // than CPU.  Custom devices that want a higher priority can set the
            // 'priority' field when registering their device to something
            // higher than the packaged devices.  See calls to
            // REGISTER_LOCAL_DEVICE_FACTORY to see the existing priorities used
            // for built-in devices.
            static int32_t DevicePriority(const std::string& device_type);

            // Returns true if 'device_type' is registered from plugin. Returns false if
            // 'device_type' is a first-party device.
            static bool IsPluggableDevice(const std::string& device_type);
        };

        namespace dfactory {

            template<class Factory>
            class Registrar
            {
            public:
                // Multiple registrations for the same device type with different priorities
                // are allowed.  Priorities are used in two different ways:
                //
                // 1) When choosing which factory (that is, which device
                //    implementation) to use for a specific 'device_type', the
                //    factory registered with the highest priority will be chosen.
                //    For example, if there are two registrations:
                //
                //      Registrar<CPUFactory1>("CPU", 125);
                //      Registrar<CPUFactory2>("CPU", 150);
                //
                //    then CPUFactory2 will be chosen when
                //    DeviceFactory::GetFactory("CPU") is called.
                //
                // 2) When choosing which 'device_type' is preferred over other
                //    DeviceTypes in a DeviceSet, the ordering is determined
                //    by the 'priority' set during registration.  For example, if there
                //    are two registrations:
                //
                //      Registrar<CPUFactory>("CPU", 100);
                //      Registrar<GPUFactory>("GPU", 200);
                //
                //    then DeviceType("GPU") will be prioritized higher than
                //    DeviceType("CPU").
                //
                // The default priority values for built-in devices is:
                // GPU: 210
                // GPUCompatibleCPU: 70
                // ThreadPoolDevice: 60
                // Default: 50
                explicit Registrar(const std::string& device_type, int priority = 50)
                {
                    DeviceFactory::Register(
                        device_type, std::make_unique<Factory>(), priority,
                        /*is_pluggable_device*/ false
                    );
                }
            };

        } // namespace dfactory

#define REGISTER_LOCAL_DEVICE_FACTORY(device_type, device_factory, ...)                            \
    INTERNAL_REGISTER_LOCAL_DEVICE_FACTORY(device_type, device_factory, __COUNTER__, ##__VA_ARGS__)

#define INTERNAL_REGISTER_LOCAL_DEVICE_FACTORY(device_type, device_factory, ctr, ...)              \
    static ::tensorflow::dfactory::Registrar<device_factory>                                       \
    INTERNAL_REGISTER_LOCAL_DEVICE_FACTORY_NAME(ctr)(device_type, ##__VA_ARGS__)

// __COUNTER__ must go through another macro to be properly expanded
#define INTERNAL_REGISTER_LOCAL_DEVICE_FACTORY_NAME(ctr) ___##ctr##__object_

    } // namespace tensorflow

    // ==================================================================
    // Implementation: device_factory.cc
    // ==================================================================

    namespace tensorflow {

        namespace {

            static mutex* get_device_factory_lock()
            {
                static mutex device_factory_lock(LINKER_INITIALIZED);
                return &device_factory_lock;
            }

            struct FactoryItem
            {
                std::unique_ptr<DeviceFactory> factory;
                int priority;
                bool is_pluggable_device;
            };

            std::unordered_map<std::string, FactoryItem>& device_factories()
            {
                static std::unordered_map<std::string, FactoryItem>* factories =
                    new std::unordered_map<std::string, FactoryItem>;
                return *factories;
            }

            bool IsDeviceFactoryEnabled(const std::string& device_type)
            {
                std::vector<std::string> enabled_devices;
                TF_CHECK_OK(
                    tensorflow::ReadStringsFromEnvVar(
                        /*env_var_name=*/"TF_ENABLED_DEVICE_TYPES", /*default_val=*/"",
                        &enabled_devices
                    )
                );
                if (enabled_devices.empty()) {
                    return true;
                }
                return std::find(enabled_devices.begin(), enabled_devices.end(), device_type) !=
                       enabled_devices.end();
            }
        } // namespace

        // static
        int32_t DeviceFactory::DevicePriority(const std::string& device_type)
        {
            tf_shared_lock l(*get_device_factory_lock());
            std::unordered_map<std::string, FactoryItem>& factories = device_factories();
            auto iter = factories.find(device_type);
            if (iter != factories.end()) {
                return iter->second.priority;
            }

            return -1;
        }

        bool DeviceFactory::IsPluggableDevice(const std::string& device_type)
        {
            tf_shared_lock l(*get_device_factory_lock());
            std::unordered_map<std::string, FactoryItem>& factories = device_factories();
            auto iter = factories.find(device_type);
            if (iter != factories.end()) {
                return iter->second.is_pluggable_device;
            }
            return false;
        }

        // static
        void DeviceFactory::Register(
            const std::string& device_type,
            std::unique_ptr<DeviceFactory> factory,
            int priority,
            bool is_pluggable_device
        )
        {
            if (!IsDeviceFactoryEnabled(device_type)) {
                LOG(INFO) << "Device factory '" << device_type << "' disabled by "
                          << "TF_ENABLED_DEVICE_TYPES environment variable.";
                return;
            }
            mutex_lock l(*get_device_factory_lock());
            std::unordered_map<std::string, FactoryItem>& factories = device_factories();
            auto iter = factories.find(device_type);
            if (iter == factories.end()) {
                factories[device_type] = {std::move(factory), priority, is_pluggable_device};
            } else {
                if (iter->second.priority < priority) {
                    iter->second = {std::move(factory), priority, is_pluggable_device};
                } else if (iter->second.priority == priority) {
                    LOG(FATAL) << "Duplicate registration of device factory for type "
                               << device_type << " with the same priority " << priority;
                }
            }
        }

        DeviceFactory* DeviceFactory::GetFactory(const std::string& device_type)
        {
            tf_shared_lock l(*get_device_factory_lock());
            auto it = device_factories().find(device_type);
            if (it == device_factories().end()) {
                return nullptr;
            } else if (!IsDeviceFactoryEnabled(device_type)) {
                LOG(FATAL) << "Device type " << device_type // Crash OK
                           << " had factory registered but was explicitly disabled by "
                           << "`TF_ENABLED_DEVICE_TYPES`. This environment variable needs "
                           << "to be set at program startup.";
            }
            return it->second.factory.get();
        }

        absl::Status DeviceFactory::ListAllPhysicalDevices(std::vector<std::string>* devices)
        {
            // CPU first. A CPU device is required.
            // TODO(b/183974121): Consider merge the logic into the loop below.
            auto cpu_factory = GetFactory("CPU");
            if (!cpu_factory) {
                return absl::NotFoundError(
                    "CPU Factory not registered. Did you link in threadpool_device?"
                );
            }

            size_t init_size = devices->size();
            TF_RETURN_IF_ERROR(cpu_factory->ListPhysicalDevices(devices));
            if (devices->size() == init_size) {
                return absl::NotFoundError("No CPU devices are available in this process");
            }

            // Then the rest (including GPU).
            tf_shared_lock l(*get_device_factory_lock());
            for (auto& p: device_factories()) {
                auto factory = p.second.factory.get();
                if (factory != cpu_factory) {
                    TF_RETURN_IF_ERROR(factory->ListPhysicalDevices(devices));
                }
            }

            return absl::OkStatus();
        }

        absl::Status DeviceFactory::ListPluggablePhysicalDevices(std::vector<std::string>* devices)
        {
            tf_shared_lock l(*get_device_factory_lock());
            for (auto& p: device_factories()) {
                if (p.second.is_pluggable_device) {
                    auto factory = p.second.factory.get();
                    TF_RETURN_IF_ERROR(factory->ListPhysicalDevices(devices));
                }
            }
            return absl::OkStatus();
        }

        absl::Status DeviceFactory::GetAnyDeviceDetails(
            int device_index, std::unordered_map<std::string, std::string>* details
        )
        {
            if (device_index < 0) {
                return absl::InvalidArgumentError(
                    absl::StrCat("Device index out of bounds: ", device_index)
                );
            }
            const int orig_device_index = device_index;

            // Iterate over devices in the same way as in ListAllPhysicalDevices.
            auto cpu_factory = GetFactory("CPU");
            if (!cpu_factory) {
                return absl::NotFoundError(
                    "CPU Factory not registered. Did you link in threadpool_device?"
                );
            }

            // TODO(b/183974121): Consider merge the logic into the loop below.
            std::vector<std::string> devices;
            TF_RETURN_IF_ERROR(cpu_factory->ListPhysicalDevices(&devices));
            if (device_index < devices.size()) {
                return cpu_factory->GetDeviceDetails(device_index, details);
            }
            device_index -= devices.size();

            // Then the rest (including GPU).
            tf_shared_lock l(*get_device_factory_lock());
            for (auto& p: device_factories()) {
                auto factory = p.second.factory.get();
                if (factory != cpu_factory) {
                    devices.clear();
                    // TODO(b/146009447): Find the factory size without having to allocate a
                    // vector with all the physical devices.
                    TF_RETURN_IF_ERROR(factory->ListPhysicalDevices(&devices));
                    if (device_index < devices.size()) {
                        return factory->GetDeviceDetails(device_index, details);
                    }
                    device_index -= devices.size();
                }
            }

            return absl::InvalidArgumentError(
                absl::StrCat("Device index out of bounds: ", orig_device_index)
            );
        }

        absl::Status DeviceFactory::AddCpuDevices(
            const SessionOptions& options,
            const std::string& name_prefix,
            std::vector<std::unique_ptr<Device>>* devices
        )
        {
            auto cpu_factory = GetFactory("CPU");
            if (!cpu_factory) {
                return absl::NotFoundError(
                    "CPU Factory not registered. Did you link in threadpool_device?"
                );
            }
            size_t init_size = devices->size();
            TF_RETURN_IF_ERROR(cpu_factory->CreateDevices(options, name_prefix, devices));
            if (devices->size() == init_size) {
                return absl::NotFoundError("No CPU devices are available in this process");
            }

            return absl::OkStatus();
        }

        absl::Status DeviceFactory::AddDevices(
            const SessionOptions& options,
            const std::string& name_prefix,
            std::vector<std::unique_ptr<Device>>* devices
        )
        {
            // CPU first. A CPU device is required.
            // TODO(b/183974121): Consider merge the logic into the loop below.
            TF_RETURN_IF_ERROR(AddCpuDevices(options, name_prefix, devices));

            absl::flat_hash_set<std::string> allowed_device_types;
            for (const auto& device_filter: options.config.device_filters()) {
                DeviceNameUtils::ParsedName parsed;
                if (!DeviceNameUtils::ParseFullOrLocalName(device_filter, &parsed)) {
                    return absl::InvalidArgumentError(
                        absl::StrCat("Invalid device filter: ", device_filter)
                    );
                }
                if (parsed.has_type) {
                    allowed_device_types.insert(parsed.type);
                }
            }

            auto cpu_factory = GetFactory("CPU");
            // Then the rest (including GPU).
            mutex_lock l(*get_device_factory_lock());
            for (auto& p: device_factories()) {
                if (!allowed_device_types.empty() && !allowed_device_types.contains(p.first)) {
                    continue; // Skip if the device type is not found from the device filter.
                }
                auto factory = p.second.factory.get();
                if (factory != cpu_factory) {
                    TF_RETURN_IF_ERROR(factory->CreateDevices(options, name_prefix, devices));
                }
            }

            return absl::OkStatus();
        }

        std::unique_ptr<Device> DeviceFactory::NewDevice(
            const std::string& type, const SessionOptions& options, const std::string& name_prefix
        )
        {
            auto device_factory = GetFactory(type);
            if (!device_factory) {
                return nullptr;
            }
            SessionOptions opt = options;
            (*opt.config.mutable_device_count())[type] = 1;
            std::vector<std::unique_ptr<Device>> devices;
            TF_CHECK_OK(device_factory->CreateDevices(opt, name_prefix, &devices));
            int expected_num_devices = 1;
            auto iter = options.config.device_count().find(type);
            if (iter != options.config.device_count().end()) {
                expected_num_devices = iter->second;
            }
            DCHECK_EQ(devices.size(), static_cast<size_t>(expected_num_devices));
            return std::move(devices[0]);
        }

    } // namespace tensorflow

} // export

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

#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/kernel_def.pb.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/gtl/array_slice.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/platform/types.h"

export module cc_tmp:kernel_kernel_def_builder;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // Forward declare proto so that kernels don't need to depend on it
        class KernelDef;

        // Builder class passed to the REGISTER_KERNEL_BUILDER() macro.
        class KernelDefBuilder
        {
        public:
            // Starts with just the name field set.
            // Caller MUST call Build() and take ownership of the result.
            explicit KernelDefBuilder(const char* op_name);
            ~KernelDefBuilder();

            // Required: specify the type of device this kernel supports.
            // Returns *this.
            KernelDefBuilder& Device(const char* device_type);

            // Specify that this kernel supports a limited set of values for a
            // particular type or list(type) attr (a further restriction than
            // what the Op allows).
            // Returns *this.
            template<typename T>
            KernelDefBuilder& AttrConstraint(const char* attr_name, gtl::ArraySlice<T> allowed);

            // Like AttrConstraint above but supports just a single value.
            template<typename T>
            KernelDefBuilder& AttrConstraint(const char* attr_name, T allowed);

            // Specify that this kernel supports a limited set of values for a
            // particular type or list(type) attr (a further restriction than
            // what the Op allows).
            // Returns *this.
            KernelDefBuilder&
            TypeConstraint(const char* attr_name, absl::Span<const DataType> allowed);

            // Like TypeConstraint but supports just a single type.
            KernelDefBuilder& TypeConstraint(const char* attr_name, DataType allowed);

            // Like TypeConstraint, but (a) gets the type from a template parameter
            // and (b) only supports a constraint to a single type.
            template<class T>
            KernelDefBuilder& TypeConstraint(const char* attr_name) TF_ATTRIBUTE_NOINLINE;
            // TODO(josh11b): Support other types of attr constraints as needed.

            // Specify that this kernel requires/provides an input/output arg
            // in host memory (instead of the default, device memory).
            // Returns *this.
            KernelDefBuilder& HostMemory(const char* arg_name);

            // Specify that this kernel requires a particular value for the
            // "_kernel" attr.  May only be specified once.  Returns *this.
            KernelDefBuilder& Label(const char* label);

            // Specify a priority number for this kernel.
            KernelDefBuilder& Priority(int32_t priority);

            // Returns a pointer to a KernelDef with fields set based on the
            // above calls to this instance.
            // Caller takes ownership of the result.
            const KernelDef* Build();

        private:
            KernelDef* kernel_def_;

            KernelDefBuilder(const KernelDefBuilder&) = delete;
            void operator=(const KernelDefBuilder&) = delete;
        };

        // IMPLEMENTATION

        template<class T>
        KernelDefBuilder& KernelDefBuilder::TypeConstraint(const char* attr_name)
        {
            return this->TypeConstraint(attr_name, DataTypeToEnum<T>::v());
        }

    } // namespace tensorflow

    // ==================================================================
    // Implementation: kernel_def_builder.cc
    // ==================================================================

    namespace tensorflow {

        KernelDefBuilder::KernelDefBuilder(const char* op_name)
        {
            kernel_def_ = new KernelDef;
            kernel_def_->set_op(op_name);
        }

        KernelDefBuilder::~KernelDefBuilder()
        {
            DCHECK(kernel_def_ == nullptr) << "Did not call Build()";
        }

        KernelDefBuilder& KernelDefBuilder::Device(const char* device_type)
        {
            kernel_def_->set_device_type(device_type);
            return *this;
        }

        template<>
        KernelDefBuilder& KernelDefBuilder::AttrConstraint<int64_t>(
            const char* attr_name, absl::Span<const int64_t> allowed
        )
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            auto* allowed_values = constraint->mutable_allowed_values()->mutable_list();
            for (const int64_t integer: allowed) {
                allowed_values->add_i(integer);
            }
            return *this;
        }

        template<>
        KernelDefBuilder&
        KernelDefBuilder::AttrConstraint<int64_t>(const char* attr_name, int64_t allowed)
        {
            return AttrConstraint(
                attr_name, absl::Span<const int64_t>(std::initializer_list<int64_t>({allowed}))
            );
        }

        template<>
        KernelDefBuilder& KernelDefBuilder::AttrConstraint<std::string>(
            const char* attr_name, absl::Span<const std::string> allowed
        )
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            auto* allowed_values = constraint->mutable_allowed_values()->mutable_list();
            for (const auto& str: allowed) {
                allowed_values->add_s(str);
            }
            return *this;
        }

        template<>
        KernelDefBuilder&
        KernelDefBuilder::AttrConstraint<std::string>(const char* attr_name, std::string allowed)
        {
            return AttrConstraint(
                attr_name,
                absl::Span<const std::string>(std::initializer_list<std::string>({allowed}))
            );
        }

        template<>
        KernelDefBuilder& KernelDefBuilder::AttrConstraint<const char*>(
            const char* attr_name, absl::Span<const char* const> allowed
        )
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            auto* allowed_values = constraint->mutable_allowed_values()->mutable_list();
            for (const auto& str: allowed) {
                allowed_values->add_s(str);
            }
            return *this;
        }

        template<>
        KernelDefBuilder&
        KernelDefBuilder::AttrConstraint<const char*>(const char* attr_name, const char* allowed)
        {
            return AttrConstraint(
                attr_name,
                absl::Span<const char* const>(std::initializer_list<const char*>({allowed}))
            );
        }

        template<>
        KernelDefBuilder&
        KernelDefBuilder::AttrConstraint<bool>(const char* attr_name, bool allowed)
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            auto* allowed_values = constraint->mutable_allowed_values()->mutable_list();
            allowed_values->add_b(allowed);
            return *this;
        }

        KernelDefBuilder&
        KernelDefBuilder::TypeConstraint(const char* attr_name, absl::Span<const DataType> allowed)
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            auto* allowed_values = constraint->mutable_allowed_values()->mutable_list();
            for (DataType dt: allowed) {
                allowed_values->add_type(dt);
            }
            return *this;
        }

        KernelDefBuilder& KernelDefBuilder::TypeConstraint(const char* attr_name, DataType allowed)
        {
            auto* constraint = kernel_def_->add_constraint();
            constraint->set_name(attr_name);
            constraint->mutable_allowed_values()->mutable_list()->add_type(allowed);
            return *this;
        }

        KernelDefBuilder& KernelDefBuilder::HostMemory(const char* arg_name)
        {
            kernel_def_->add_host_memory_arg(arg_name);
            return *this;
        }

        KernelDefBuilder& KernelDefBuilder::Label(const char* label)
        {
            CHECK_EQ(kernel_def_->label(), "") << "Trying to set a kernel's label a second time: '"
                                               << label << "' in: " << kernel_def_->DebugString();
            kernel_def_->set_label(label);
            return *this;
        }

        KernelDefBuilder& KernelDefBuilder::Priority(int32_t priority)
        {
            kernel_def_->set_priority(priority);
            return *this;
        }

        const KernelDef* KernelDefBuilder::Build()
        {
            KernelDef* r = kernel_def_;
            kernel_def_ = nullptr;
            return r;
        }

    } // namespace tensorflow

} // export

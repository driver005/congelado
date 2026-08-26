module;

/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor.pb.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/types.h"

#include <algorithm>
#include <type_traits>
#include <vector>

export module cc_tmp:variant_variant_tensor_data;

import std;
import cc_abi;

export {

    namespace tensorflow {

        class VariantTensorDataProto;

        // The serialization format for Variant objects. Objects with references to
        // other Tensors can simply store those tensors in the `tensors` field, and
        // serialize other metadata content in to the `metadata` field. Objects can
        // optionally set the `type_name` for type-checking before deserializing an
        // object.
        //
        // This is the native C++ class equivalent of VariantTensorDataProto. They are
        // separate so that kernels do not need to depend on protos.
        class VariantTensorData
        {
        public:
            VariantTensorData() = default;

            // TODO(b/118823936): This silently returns if the proto is invalid.
            // Consider calling FromProto explicitly instead.
            VariantTensorData(VariantTensorDataProto proto);

            // Name of the type of objects being serialized.
            const std::string& type_name() const
            {
                return type_name_;
            }

            void set_type_name(const std::string& type_name)
            {
                type_name_ = type_name;
            }

            template<
                typename T,
                bool = std::is_trivially_copyable<typename std::decay<T>::type>::value &&
                       !std::is_pointer<typename std::decay<T>::type>::value>
            struct PODResolver
            {};

            // Portions of the object that are not Tensors.
            // Directly supported types include string POD types.
            template<typename T>
            void set_metadata(const T& value)
            {
                SetMetadata(value, PODResolver<T>());
            }

            template<typename T>
            bool get_metadata(T* value) const
            {
                return GetMetadata(value, PODResolver<T>());
            }

            std::string& metadata_string()
            {
                return metadata_;
            }

            const std::string& metadata_string() const
            {
                return metadata_;
            }

            // Tensors contained within objects being serialized.
            int tensors_size() const;
            const Tensor& tensors(int index) const;
            const std::vector<Tensor>& tensors() const;
            Tensor* add_tensors();

            // A more general version of add_tensors. Parameters are perfectly forwarded
            // to the constructor of the tensor added here.
            template<typename... TensorConstructorArgs>
            Tensor* add_tensor(TensorConstructorArgs&&... args);

            // Conversion to and from VariantTensorDataProto
            void ToProto(VariantTensorDataProto* proto) const;
            // This allows optimizations via std::move.
            bool FromProto(VariantTensorDataProto proto);
            bool FromConstProto(const VariantTensorDataProto& proto);

            // Serialization via VariantTensorDataProto
            std::string SerializeAsString() const;
            bool SerializeToString(std::string* buf);
            bool ParseFromString(std::string s);

            std::string DebugString() const;

        public:
            std::string type_name_;
            std::string metadata_;
            std::vector<Tensor> tensors_;

        private:
            void SetMetadata(const std::string& value, PODResolver<std::string, false /* is_pod */>)
            {
                metadata_ = value;
            }

            bool GetMetadata(std::string* value, PODResolver<std::string, false /* is_pod */>) const
            {
                *value = metadata_;
                return true;
            }

            // Specialize for bool, it is undefined behvaior to assign a non 0/1 value to
            // a bool. Now we coerce a non-zero value to true.
            bool GetMetadata(bool* value, PODResolver<bool, true /* is_pod */>) const
            {
                if (metadata_.size() != sizeof(bool)) {
                    return false;
                }
                *value = false;
                for (size_t i = 0; i < sizeof(bool); ++i) {
                    *value = *value || (metadata_.data()[i] != 0);
                }
                return true;
            }

            template<typename T>
            void SetMetadata(const T& value, PODResolver<T, true /* is_pod */>)
            {
                metadata_.assign(reinterpret_cast<const char*>(&value), sizeof(T));
            }

            template<typename T>
            bool GetMetadata(T* value, PODResolver<T, true /* is_pod */>) const
            {
                if (metadata_.size() != sizeof(T)) {
                    return false;
                }
                std::copy_n(metadata_.data(), sizeof(T), reinterpret_cast<char*>(value));
                return true;
            }

            template<typename T>
            void SetMetadata(const T& value, PODResolver<T, false /* is_pod */>)
            {
                static_assert(
                    std::is_pointer<typename std::decay<T>::type>::value,
                    "Only strings and pointers are supported for non-POD SetMetadata"
                );
            }

            template<typename T>
            bool GetMetadata(T* value, PODResolver<T, false /* is_pod */>) const
            {
                static_assert(
                    std::is_pointer<typename std::decay<T>::type>::value,
                    "Only strings and pointers are supported for non-POD GetMetadata"
                );
                return false;
            }
        };

        // For backwards compatibility for when this was a proto
        std::string ProtoDebugString(const VariantTensorData& object);

        template<typename... TensorConstructorArgs>
        Tensor* VariantTensorData::add_tensor(TensorConstructorArgs&&... args)
        {
            tensors_.emplace_back(std::forward<TensorConstructorArgs>(args)...);
            return &tensors_.back();
        }

    } // namespace tensorflow

    // ==================================================================
    // Implementation: variant_tensor_data.cc
    // ==================================================================

    namespace tensorflow {

        VariantTensorData::VariantTensorData(VariantTensorDataProto proto)
        {
            FromProto(std::move(proto));
        }

        int VariantTensorData::tensors_size() const
        {
            return tensors_.size();
        }

        const Tensor& VariantTensorData::tensors(int index) const
        {
            return tensors_[index];
        }

        const std::vector<Tensor>& VariantTensorData::tensors() const
        {
            return tensors_;
        }

        Tensor* VariantTensorData::add_tensors()
        {
            tensors_.emplace_back();
            return &(tensors_[tensors_.size() - 1]);
        }

        void VariantTensorData::ToProto(VariantTensorDataProto* proto) const
        {
            proto->set_type_name(type_name());
            proto->set_metadata(metadata_);
            proto->clear_tensors();
            for (const auto& tensor: tensors_) {
                tensor.AsProtoField(proto->mutable_tensors()->Add());
            }
        }

        bool VariantTensorData::FromProto(VariantTensorDataProto proto)
        {
            // TODO(ebrevdo): Do this lazily.
            set_type_name(proto.type_name());
            set_metadata(proto.metadata());
            for (const auto& tensor: proto.tensors()) {
                Tensor tmp;
                if (!tmp.FromProto(tensor)) {
                    return false;
                }
                tensors_.push_back(tmp);
            }
            return true;
        }

        bool VariantTensorData::FromConstProto(const VariantTensorDataProto& proto)
        {
            set_type_name(proto.type_name());
            set_metadata(proto.metadata());
            for (const auto& tensor: proto.tensors()) {
                Tensor tmp;
                if (!tmp.FromProto(tensor)) {
                    return false;
                }
                tensors_.push_back(tmp);
            }
            return true;
        }

        std::string VariantTensorData::SerializeAsString() const
        {
            VariantTensorDataProto proto;
            ToProto(&proto);
            return proto.SerializeAsString();
        }

        bool VariantTensorData::SerializeToString(std::string* buf)
        {
            VariantTensorDataProto proto;
            ToProto(&proto);
            return proto.SerializeToString(buf);
        }

        bool VariantTensorData::ParseFromString(std::string s)
        {
            VariantTensorDataProto proto;
            const bool status = proto.ParseFromString(s);
            if (status) {
                FromProto(std::move(proto));
            }
            return status;
        }

        std::string VariantTensorData::DebugString() const
        {
            std::string repeated_field = "";
            for (const auto& t: tensors_) {
                repeated_field = absl::StrCat(repeated_field, " tensors: ", t.DebugString());
            }
            return strings::StrCat(
                "type_name: ", type_name(), " metadata: ", metadata_, repeated_field
            );
        }

        std::string ProtoDebugString(const VariantTensorData& object)
        {
            return object.DebugString();
        }

    } // namespace tensorflow

} // export

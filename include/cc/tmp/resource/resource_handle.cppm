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

#include <optional>
#include <string>
#include "tensorflow/core/framework/resource_base.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/type_index.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/platform/casts.h"
#include "tensorflow/core/platform/intrusive_ptr.h"
#include "tensorflow/core/platform/statusor.h"
#include "tensorflow/core/platform/tensor_coding.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/managed_stack_trace.h"
#include <utility>
#include <vector>
#include "absl/strings/str_format.h"
#include "tensorflow/core/framework/resource_handle.pb.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/demangle.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/macros.h"

export module cc_tmp:resource_resource_handle;

import std;
import cc_abi;

export {

namespace tensorflow {

class ResourceHandleProto;

// Class representing a handle to a tensorflow resource. Handles are
// not valid across executions, but can be serialized back and forth from within
// a single run (except for those created from MakeRefCountingHandle i.e. whose
// resource_ field is not empty).
//
// This is the native C++ class equivalent of ResourceHandleProto.  They are
// separate so that kernels do not need to depend on protos.
class ResourceHandle {
 public:
  ResourceHandle();
  ResourceHandle(const ResourceHandleProto& proto);
  ~ResourceHandle();

  // Use this factory method if the `proto` comes from user controlled input, to
  // prevent a denial of service.
  static absl::Status BuildResourceHandle(const ResourceHandleProto& proto,
                                          ResourceHandle* out);

  // Unique name for the device containing the resource.
  const std::string& device() const { return device_; }

  void set_device(const std::string& device) { device_ = device; }

  // Container in which this resource is placed.
  const std::string& container() const { return container_; }
  void set_container(const std::string& container) { container_ = container; }

  // Unique name of this resource.
  const std::string& name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  // Hash code for the type of the resource. Is only valid in the same device
  // and in the same execution.
  uint64_t hash_code() const { return hash_code_; }
  void set_hash_code(uint64_t hash_code) { hash_code_ = hash_code; }

  // For debug-only, the name of the type pointed to by this handle, if
  // available.
  const std::string& maybe_type_name() const { return maybe_type_name_; }
  void set_maybe_type_name(const std::string& value) {
    maybe_type_name_ = value;
  }

  // Data types and shapes for the underlying resource.
  std::vector<DtypeAndPartialTensorShape> dtypes_and_shapes() const {
    return dtypes_and_shapes_;
  }
  void set_dtypes_and_shapes(
      const std::vector<DtypeAndPartialTensorShape>& dtypes_and_shapes) {
    dtypes_and_shapes_ = dtypes_and_shapes;
  }

  void set_definition_stack_trace(
      const std::optional<ManagedStackTrace>& definition_stack_trace) {
    definition_stack_trace_ = definition_stack_trace;
  }

  const std::optional<ManagedStackTrace>& definition_stack_trace() const {
    return definition_stack_trace_;
  }

  // Conversion to and from ResourceHandleProto
  void AsProto(ResourceHandleProto* proto) const;
  absl::Status FromProto(const ResourceHandleProto& proto);

  // Serialization via ResourceHandleProto
  std::string SerializeAsString() const;
  bool ParseFromString(const std::string& s);

  std::string DebugString() const;

  std::string SummarizeValue() const;

  // GUID for anonymous resources. Resources with this shared_name will have
  // their shared_name replaced with a GUID at creation time
  static constexpr const char* ANONYMOUS_NAME =
      "cd2c89b7-88b7-44c8-ad83-06c2a9158347";

  // Creates a `ResourceHandle` that holds a pointer to a resource and takes
  // ownership of it. Normally a `ResourceHandle` only contains the name (and
  // some other metadata) of the resource. When created via this function,
  // the handle will own the resource, in the sense that it will destroy the
  // resource automatically when the resource is no longer needed. It does this
  // via automatic ref-counting on the resource: when the handle is copied, it
  // will call `Ref` on the resource (remember that all resources inherit from
  // `ResourceBase` which inherits from `RefCounted`), and when the handle is
  // destroyed, it will call `Unref` on the resource. When the last handle goes
  // out of scope, the resource's ref-count will go down to zero and the
  // resource will be destroyed. When calling this function, the `resource`
  // argument should have a ref-count of one (which is the case when the
  // resource is newly created).
  //
  // For those familiar with `ResourceMgr`, when you create a handle by the
  // `MakeResourceHandle` function in resource_mgr.h, the handle doesn't hold a
  // strong reference to the resource, and the resource is owned by the
  // resource manager whose strong reference must be manually deleted by
  // calling `ResourceMgr::Delete`. In contrast, a handle created by this
  // function holds a strong reference to the resource. The resource manager
  // does not hold a strong reference to the resource.
  template <typename T>
  static ResourceHandle MakeRefCountingHandle(
      T* resource, const std::string& device_name,
      const std::vector<DtypeAndPartialTensorShape>& dtypes_and_shapes = {},
      const std::optional<ManagedStackTrace>& definition_stack_trace = {}) {
    return MakeRefCountingHandle(resource, device_name, TypeIndex::Make<T>(),
                                 dtypes_and_shapes, definition_stack_trace);
  }

  static ResourceHandle MakeRefCountingHandle(
      ResourceBase* resource, const std::string& device_name,
      const TypeIndex& type_index,
      const std::vector<DtypeAndPartialTensorShape>& dtypes_and_shapes = {},
      const std::optional<ManagedStackTrace>& definition_stack_trace = {});

  // Pointer to the resource.
  const core::IntrusivePtr<ResourceBase>& resource() const { return resource_; }

  // Gets the resource pointer in `handle` as `T*`, or an error if the actual
  // resource type is not `T`.
  template <typename T>
  StatusOr<T*> GetResource() const {
    TF_RETURN_IF_ERROR(ValidateType<T>());
    return down_cast<T*>(resource_.get());
  }

  // Returns True if the resource handle is ref-counting.
  // See MakeRefCountingHandle.
  bool IsRefCounting() const { return resource_.get() != nullptr; }

  // Validates that the resource type in `handle` is `T`.
  template <typename T>
  absl::Status ValidateType() const {
    return ValidateType(TypeIndex::Make<T>());
  }

  absl::Status ValidateType(const TypeIndex& type_index) const;

  // Generates unique IDs (e.g. for names of anonymous variables)
  static int64_t GenerateUniqueId();

 private:
  std::string device_;
  std::string container_;
  std::string name_;
  uint64_t hash_code_ = 0;
  std::string maybe_type_name_;
  std::vector<DtypeAndPartialTensorShape> dtypes_and_shapes_;
  std::optional<ManagedStackTrace> definition_stack_trace_;
  // A smart pointer to the actual resource. When this field is not empty, the
  // handle is in a "ref-counting" mode, owning the resource; otherwise it's in
  // a "weak-ref" mode, only containing the name of the resource (conceptually a
  // weak reference).
  core::IntrusivePtr<ResourceBase> resource_;
  static std::atomic<int64_t> current_id_;
};

// For backwards compatibility for when this was a proto
std::string ProtoDebugString(const ResourceHandle& handle);

// Encodes a list of ResourceHandle protos in the given StringListEncoder.
void EncodeResourceHandleList(const ResourceHandle* p, int64_t n,
                              std::unique_ptr<port::StringListEncoder> e);

// Decodes a list of ResourceHandle protos from the given StringListDecoder.
bool DecodeResourceHandleList(std::unique_ptr<port::StringListDecoder> d,
                              ResourceHandle* ps, int64_t n);

}  // namespace tensorflow

// ==================================================================
// Implementation: resource_handle.cc
// ==================================================================

namespace tensorflow {

namespace {
std::string DtypeAndShapesToString(
    const std::vector<DtypeAndPartialTensorShape>& dtype_and_shapes) {
  std::vector<std::string> dtype_and_shape_strings;
  dtype_and_shape_strings.reserve(dtype_and_shapes.size());
  for (const DtypeAndPartialTensorShape& dtype_and_shape : dtype_and_shapes) {
    // Note that it is a bit unfortunate to return int/enum as dtype, given we
    // can't directly use DataTypeString due to circular dependency.
    dtype_and_shape_strings.push_back(
        absl::StrFormat("DType enum: %d, Shape: %s", dtype_and_shape.dtype,
                        dtype_and_shape.shape.DebugString()));
  }
  return absl::StrFormat("[ %s ]", absl::StrJoin(dtype_and_shape_strings, ","));
}
}  // namespace

// Must be declared here for pre-C++17 compatibility.
/* static */ constexpr const char* ResourceHandle::ANONYMOUS_NAME;

ResourceHandle::ResourceHandle() {}

ResourceHandle::ResourceHandle(const ResourceHandleProto& proto) {
  TF_CHECK_OK(FromProto(proto));
}

absl::Status ResourceHandle::BuildResourceHandle(
    const ResourceHandleProto& proto, ResourceHandle* out) {
  if (out == nullptr)
    return absl::InternalError(
        "BuildResourceHandle() was called with nullptr for the output");
  return out->FromProto(proto);
}

ResourceHandle::~ResourceHandle() {}

void ResourceHandle::AsProto(ResourceHandleProto* proto) const {
  proto->set_device(device());
  proto->set_container(container());
  proto->set_name(name());
  proto->set_hash_code(hash_code());
  proto->set_maybe_type_name(maybe_type_name());
  for (const auto& dtype_and_shape_pair : dtypes_and_shapes_) {
    auto dtype_and_shape = proto->add_dtypes_and_shapes();
    dtype_and_shape->set_dtype(dtype_and_shape_pair.dtype);
    dtype_and_shape_pair.shape.AsProto(dtype_and_shape->mutable_shape());
  }
}

absl::Status ResourceHandle::FromProto(const ResourceHandleProto& proto) {
  set_device(proto.device());
  set_container(proto.container());
  set_name(proto.name());
  set_hash_code(proto.hash_code());
  set_maybe_type_name(proto.maybe_type_name());
  std::vector<DtypeAndPartialTensorShape> dtypes_and_shapes;
  for (const auto& dtype_and_shape : proto.dtypes_and_shapes()) {
    DataType dtype = dtype_and_shape.dtype();
    PartialTensorShape shape;
    absl::Status s = PartialTensorShape::BuildPartialTensorShape(
        dtype_and_shape.shape(), &shape);
    if (!s.ok()) {
      return s;
    }
    dtypes_and_shapes.push_back(DtypeAndPartialTensorShape{dtype, shape});
  }
  dtypes_and_shapes_ = std::move(dtypes_and_shapes);
  return absl::OkStatus();
}

std::string ResourceHandle::SerializeAsString() const {
  ResourceHandleProto proto;
  AsProto(&proto);
  return proto.SerializeAsString();
}

bool ResourceHandle::ParseFromString(const std::string& s) {
  ResourceHandleProto proto;
  return proto.ParseFromString(s) && FromProto(proto).ok();
}

std::string ResourceHandle::DebugString() const {
  return absl::StrFormat(
      "device: %s container: %s name: %s hash_code: 0x%X maybe_type_name %s, "
      "dtype and shapes : %s",
      device(), container(), name(), hash_code(),
      port::Demangle(maybe_type_name()),
      DtypeAndShapesToString(dtypes_and_shapes()));
}
std::string ResourceHandle::SummarizeValue() const {
  return absl::StrFormat(
      "ResourceHandle(name=\"%s\", device=\"%s\", container=\"%s\", "
      "type=\"%s\", dtype and shapes : \"%s\")",
      name(), device(), container(), port::Demangle(maybe_type_name()),
      DtypeAndShapesToString(dtypes_and_shapes()));
}

ResourceHandle ResourceHandle::MakeRefCountingHandle(
    ResourceBase* resource, const std::string& device_name,
    const TypeIndex& type_index,
    const std::vector<DtypeAndPartialTensorShape>& dtypes_and_shapes,
    const std::optional<ManagedStackTrace>& definition_stack_trace) {
  ResourceHandle result;
  result.resource_.reset(resource, /*add_ref=*/false);
  result.set_device(device_name);
  // All resources owned by anonymous handles are put into the same container,
  // and they get process-unique handle names.
  result.set_container("Anonymous");
  result.set_definition_stack_trace(definition_stack_trace);
  auto resource_id = GenerateUniqueId();
  std::string handle_name = resource->MakeRefCountingHandleName(resource_id);
  result.set_name(handle_name);
  result.set_hash_code(type_index.hash_code());
  result.set_maybe_type_name(type_index.name());
  result.set_dtypes_and_shapes(dtypes_and_shapes);
  return result;
}

absl::Status ResourceHandle::ValidateType(const TypeIndex& type_index) const {
  if (type_index.hash_code() != hash_code()) {
    return errors::InvalidArgument(
        "Trying to access a handle's resource using the wrong type. ",
        "The handle points to a resource (name '", name(), "') of type '",
        port::Demangle(maybe_type_name()), "' (hash code ", hash_code(),
        ") but you are trying to access the resource as type '",
        port::Demangle(type_index.name()), "' (hash code ",
        type_index.hash_code(), ")");
  }
  return absl::OkStatus();
}

std::atomic<int64_t> ResourceHandle::current_id_;

int64_t ResourceHandle::GenerateUniqueId() { return current_id_.fetch_add(1); }

std::string ProtoDebugString(const ResourceHandle& handle) {
  return handle.DebugString();
}

void EncodeResourceHandleList(const ResourceHandle* p, int64_t n,
                              std::unique_ptr<port::StringListEncoder> e) {
  ResourceHandleProto proto;
  for (int i = 0; i < n; ++i) {
    p[i].AsProto(&proto);
    e->Append(proto);
  }
  e->Finalize();
}

bool DecodeResourceHandleList(std::unique_ptr<port::StringListDecoder> d,
                              ResourceHandle* ps, int64_t n) {
  std::vector<uint32_t> sizes(n);
  if (!d->ReadSizes(&sizes)) return false;

  ResourceHandleProto proto;
  for (int i = 0; i < n; ++i) {
    if (!proto.ParseFromString(
            absl::string_view(d->Data(sizes[i]), sizes[i]))) {
      return false;
    }
    if (!ps[i].FromProto(proto).ok()) {
      return false;
    }
  }
  return true;
}

}  // namespace tensorflow

} // export

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

#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/lib/core/errors.h"

export module cc_tmp:io_lookup_interface;

import std;
import cc_abi;

export {

namespace tensorflow {

class OpKernelContext;

namespace lookup {

// Forward declaration so we can define GetInitializableLookupTable() in
// LookupInterface.
class InitializableLookupTable;

// Lookup interface for batch lookups used by table lookup ops.
class LookupInterface : public ResourceBase {
 public:
  // Performs batch lookups, for every element in the key tensor, Find returns
  // the corresponding value into the values tensor.
  // If an element is not present in the table, the given default value is used.

  // For tables that require initialization, Find is available once the table
  // is marked as initialized.

  // Returns the following statuses:
  // - OK: when the find finishes successfully.
  // - FailedPrecondition: if the table is not initialized.
  // - InvalidArgument: if any of the preconditions on the lookup key or value
  //   fails.
  // - In addition, other implementations may provide another non-OK status
  //   specific to their failure modes.
  virtual absl::Status Find(OpKernelContext* ctx, const Tensor& keys,
                            Tensor* values, const Tensor& default_value) = 0;

  // Inserts elements into the table. Each element of the key tensor is
  // associated with the corresponding element in the value tensor.
  // This method is only implemented in mutable tables that can be updated over
  // the execution of the graph. It returns Status::NotImplemented for read-only
  // tables that are initialized once before they can be looked up.

  // Returns the following statuses:
  // - OK: when the insert finishes successfully.
  // - InvalidArgument: if any of the preconditions on the lookup key or value
  //   fails.
  // - Unimplemented: if the table does not support insertions.
  virtual absl::Status Insert(OpKernelContext* ctx, const Tensor& keys,
                              const Tensor& values) = 0;

  // Removes elements from the table.
  // This method is only implemented in mutable tables that can be updated over
  // the execution of the graph. It returns Status::NotImplemented for read-only
  // tables that are initialized once before they can be looked up.

  // Returns the following statuses:
  // - OK: when the remove finishes successfully.
  // - InvalidArgument: if any of the preconditions on the lookup key fails.
  // - Unimplemented: if the table does not support removals.
  virtual absl::Status Remove(OpKernelContext* ctx, const Tensor& keys) = 0;

  // Returns the number of elements in the table.
  virtual size_t size() const = 0;

  // Exports the values of the table to two tensors named keys and values.
  // Note that the shape of the tensors is completely up to the implementation
  // of the table and can be different than the tensors used for the Insert
  // function above.
  virtual absl::Status ExportValues(OpKernelContext* ctx) = 0;

  // Imports previously exported keys and values.
  // As mentioned above, the shape of the keys and values tensors are determined
  // by the ExportValues function above and can be different than for the
  // Insert function.
  virtual absl::Status ImportValues(OpKernelContext* ctx, const Tensor& keys,
                                    const Tensor& values) = 0;

  // Returns the data type of the key.
  virtual DataType key_dtype() const = 0;

  // Returns the data type of the value.
  virtual DataType value_dtype() const = 0;

  // Returns the shape of a key in the table.
  virtual TensorShape key_shape() const = 0;

  // Returns the shape of a value in the table.
  virtual TensorShape value_shape() const = 0;

  // Check format of the key and value tensors for the Insert function.
  // Returns OK if all the following requirements are satisfied, otherwise it
  // returns InvalidArgument:
  // - DataType of the tensor keys equals to the table key_dtype
  // - DataType of the tensor values equals to the table value_dtype
  // - the values tensor has the required shape given keys and the tables's
  //   value shape.
  virtual absl::Status CheckKeyAndValueTensorsForInsert(const Tensor& keys,
                                                        const Tensor& values);

  // Similar to the function above but instead checks eligibility for the Import
  // function.
  virtual absl::Status CheckKeyAndValueTensorsForImport(const Tensor& keys,
                                                        const Tensor& values);

  // Check format of the key tensor for the Remove function.
  // Returns OK if all the following requirements are satisfied, otherwise it
  // returns InvalidArgument:
  // - DataType of the tensor keys equals to the table key_dtype
  virtual absl::Status CheckKeyTensorForRemove(const Tensor& keys);

  // Check the arguments of a find operation. Returns OK if all the following
  // requirements are satisfied, otherwise it returns InvalidArgument:
  // - DataType of the tensor keys equals to the table key_dtype
  // - DataType of the tensor default_value equals to the table value_dtype
  // - the default_value tensor has the required shape given keys and the
  //   tables's value shape.
  absl::Status CheckFindArguments(const Tensor& keys,
                                  const Tensor& default_value);

  std::string DebugString() const override {
    return absl::StrCat("A lookup table of size: ", size());
  }

  // Returns an InitializableLookupTable, a subclass of LookupInterface, if the
  // current object is an InitializableLookupTable. Otherwise, returns nullptr.
  virtual InitializableLookupTable* GetInitializableLookupTable() {
    return nullptr;
  }

 protected:
  ~LookupInterface() override = default;

  // Makes sure that the key and value tensor DataType's match the table
  // key_dtype and value_dtype.
  absl::Status CheckKeyAndValueTypes(const Tensor& keys, const Tensor& values);

  // Makes sure that the provided shape is consistent with the table keys shape.
  absl::Status CheckKeyShape(const TensorShape& shape);

 private:
  absl::Status CheckKeyAndValueTensorsHelper(const Tensor& keys,
                                             const Tensor& values);
};

}  // namespace lookup
}  // namespace tensorflow

// ==================================================================
// Implementation: lookup_interface.cc
// ==================================================================

namespace tensorflow {
namespace lookup {

Status LookupInterface::CheckKeyShape(const TensorShape& shape) {
  if (!TensorShapeUtils::EndsWith(shape, key_shape())) {
    return errors::InvalidArgument("Input key shape ", shape.DebugString(),
                                   " must end with the table's key shape ",
                                   key_shape().DebugString());
  }
  return OkStatus();
}

Status LookupInterface::CheckKeyAndValueTypes(const Tensor& keys,
                                              const Tensor& values) {
  if (keys.dtype() != key_dtype()) {
    return errors::InvalidArgument("Key must be type ", key_dtype(),
                                   " but got ", keys.dtype());
  }
  if (values.dtype() != value_dtype()) {
    return errors::InvalidArgument("Value must be type ", value_dtype(),
                                   " but got ", values.dtype());
  }
  return OkStatus();
}

Status LookupInterface::CheckKeyAndValueTensorsHelper(const Tensor& keys,
                                                      const Tensor& values) {
  TF_RETURN_IF_ERROR(CheckKeyAndValueTypes(keys, values));
  TF_RETURN_IF_ERROR(CheckKeyShape(keys.shape()));

  TensorShape expected_value_shape = keys.shape();
  for (int i = 0; i < key_shape().dims(); ++i) {
    expected_value_shape.RemoveDim(expected_value_shape.dims() - 1);
  }
  expected_value_shape.AppendShape(value_shape());
  if (values.shape() != expected_value_shape) {
    return errors::InvalidArgument(
        "Expected shape ", expected_value_shape.DebugString(),
        " for value, got ", values.shape().DebugString());
  }
  return OkStatus();
}

Status LookupInterface::CheckKeyAndValueTensorsForInsert(const Tensor& keys,
                                                         const Tensor& values) {
  return CheckKeyAndValueTensorsHelper(keys, values);
}

Status LookupInterface::CheckKeyAndValueTensorsForImport(const Tensor& keys,
                                                         const Tensor& values) {
  return CheckKeyAndValueTensorsHelper(keys, values);
}

Status LookupInterface::CheckKeyTensorForRemove(const Tensor& keys) {
  if (keys.dtype() != key_dtype()) {
    return errors::InvalidArgument("Key must be type ", key_dtype(),
                                   " but got ", keys.dtype());
  }
  return CheckKeyShape(keys.shape());
}

Status LookupInterface::CheckFindArguments(const Tensor& key,
                                           const Tensor& default_value) {
  TF_RETURN_IF_ERROR(CheckKeyAndValueTypes(key, default_value));
  TF_RETURN_IF_ERROR(CheckKeyShape(key.shape()));
  TensorShape fullsize_value_shape = key.shape();
  for (int i = 0; i < key_shape().dims(); ++i) {
    fullsize_value_shape.RemoveDim(fullsize_value_shape.dims() - 1);
  }
  fullsize_value_shape.AppendShape(value_shape());
  if (default_value.shape() != value_shape() &&
      default_value.shape() != fullsize_value_shape) {
    return errors::InvalidArgument(
        "Expected shape ", value_shape().DebugString(), " or ",
        fullsize_value_shape.DebugString(), " for default value, got ",
        default_value.shape().DebugString());
  }
  return OkStatus();
}

}  // namespace lookup
}  // namespace tensorflow

} // export

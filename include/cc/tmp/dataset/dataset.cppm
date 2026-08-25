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

#include <cstdlib>
#include <deque>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "xla/tsl/framework/allocator.h"
#include "xla/tsl/platform/errors.h"
#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/attr_value_util.h"
#include "tensorflow/core/framework/cancellation.h"
#include "tensorflow/core/framework/collective.h"
#include "tensorflow/core/framework/dataset_metadata.pb.h"
#include "tensorflow/core/framework/dataset_options.pb.h"
#include "tensorflow/core/framework/dataset_stateful_op_allowlist.h"
#include "tensorflow/core/framework/function.h"
#include "tensorflow/core/framework/function_handle_cache.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/model.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/thread_factory.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/framework/variant_encode_decode.h"
#include "tensorflow/core/framework/variant_tensor_data.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/core/threadpool_interface.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/cpu_info.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/refcount.h"
#include "tensorflow/core/platform/status.h"
#include "tsl/platform/thread_annotations.h"
#include "tensorflow/core/activity_watcher/activity.h"
#include "tensorflow/core/framework/dataset.pb.h"
#include "tensorflow/core/framework/device_base.h"
#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/framework/variant_op_registry.h"
#include "tensorflow/core/framework/versions.pb.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/graph/node_builder.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/resource.h"
#include "tensorflow/core/platform/strcat.h"
#include "tensorflow/core/profiler/lib/traceme.h"
#include "tensorflow/core/public/version.h"

export module cc_tmp:dataset_dataset;

import std;
import cc_abi;

export {

// Polymorphic datasets should support all primitive TensorFlow
// types. Use this macro to expand `m(T)` once for each primitive type
// `T`, e.g. to build a `switch` statement.
#define TF_CALL_DATASET_TYPES(m) TF_CALL_ALL_TYPES(m) TF_CALL_QUANTIZED_TYPES(m)

namespace tensorflow {

// Forward declarations to avoid introducing a dependency on headers in
// "tensorflow/core/graph/...".
class GraphDefBuilder;
class Node;

namespace data {

namespace internal {
// Merges Options from source to destination. If there is a conflict on a field,
// the field value from the source takes precedence.
void MergeOptions(const protobuf::Message& source,
                  protobuf::Message* destination);
void MergeOptions(const protobuf::MessageLite& source,
                  protobuf::MessageLite* destination);
}  // namespace internal

using TraceMeMetadata = std::vector<std::pair<absl::string_view, std::string>>;

// Maps the index of dataset elements to a globally shuffled index. See the
// comment for IteratorContext::Params::index_mapper for more details.
// Notes:
// * `absl::OutOfRangeError` indicates the input index argument exceeds
//   the cardinality of the dataset.
// * `absl::NotFoundError` indicates we should skip this element.
//    This happens in the case we mix multiple datasets into one. For example,
//    `dataset1.concatenate(dataset2)`.
// See go/tf-data-random-access-iterator and
// go/tf-data-random-access-iterator-for-concatenate for more info.
using IndexMapperFn = std::function<absl::StatusOr<size_t>(size_t)>;

constexpr char kTFDataFunction[] = "_tf_data_function";

constexpr int kInfiniteCardinality = -1;
constexpr int kUnknownCardinality = -2;

// This constant is a magic number that is used (as a prefix) to identify keys
// used for serialization of iterator state.
constexpr char kFullNameRandomHex[] = "60d899aa0d8ce4351e7c3b419e92d25b";
constexpr int kFullNameRandomHexLen = std::size(kFullNameRandomHex) - 1;
constexpr char kPipe[] = "|";
constexpr char kColon[] = ":";

constexpr char kTFDataResourceTag[] = "tfdata";
constexpr char kTraceInfoUnavailable[] = "unavailable";
constexpr char kMetadata[] = "metadata";

constexpr char kCardinalityAttrForRewrite[] = "_cardinality";

class DatasetBase;
class IteratorContext;
class SerializationContext;

inline bool IsTFDataFunction(const FunctionDef& func) {
  auto iter = func.attr().find(data::kTFDataFunction);
  return (iter != func.attr().end() && iter->second.b());
}

// Interface for reading values from a key-value store.
// Used for restoring iterator state. This class is thread safe.
// Please see comment on IteratorStateWriter for guidance around using the
// Read*(key, val) vs Read*(name, key, val).
class IteratorStateReader {
 public:
  // Determines whether the iterator state contains the given key.
  virtual bool Contains(absl::string_view key) const = 0;
  virtual bool Contains(absl::string_view name,
                        absl::string_view key) const = 0;

  // Reads an integer for the given key.
  virtual absl::Status ReadScalar(absl::string_view key,
                                  int64_t* val) const = 0;
  virtual absl::Status ReadScalar(absl::string_view name, absl::string_view key,
                                  int64_t* val) const = 0;

  // Reads a string for the given key.
  virtual absl::Status ReadScalar(absl::string_view key,
                                  tstring* val) const = 0;
  virtual absl::Status ReadScalar(absl::string_view name, absl::string_view key,
                                  tstring* val) const = 0;

  // Reads a tensor for the given key.
  // TODO(jsimsa): Remove non-FLR overrides once all callers are updated.
  virtual absl::Status ReadTensor(absl::string_view key, Tensor* val) const = 0;
  virtual absl::Status ReadTensor(FunctionLibraryRuntime* flr,
                                  absl::string_view key, Tensor* val) const = 0;
  virtual absl::Status ReadTensor(absl::string_view name, absl::string_view key,
                                  Tensor* val) const = 0;
  virtual absl::Status ReadTensor(FunctionLibraryRuntime* flr,
                                  absl::string_view name, absl::string_view key,
                                  Tensor* val) const = 0;

  virtual ~IteratorStateReader() = default;
};

// Interface for writing values to a key-value store.
// Used for saving iterator state. Not thread safe.
// The IteratorStateWriter creates a tensor for each unique iterator name it
// sees. For the Write*(key, val) API's the key is expected to encode this
// name as keys are required to be produced using the full_name() method.
// Each tensor has an upper limit of 2 GB and so if the state for an iterator
// might exceed the 2 GB limit, you can pass an explicit name in via the
// Write*(name, key, val) APIs allowing you to further split up the state
// into more manageable chunks.
class IteratorStateWriter {
 public:
  // Writes an integer for the given key.
  virtual absl::Status WriteScalar(absl::string_view key,
                                   const int64_t val) = 0;
  virtual absl::Status WriteScalar(absl::string_view name,
                                   absl::string_view key,
                                   const int64_t val) = 0;

  // Writes a string for the given key.
  virtual absl::Status WriteScalar(absl::string_view key,
                                   const tstring& val) = 0;
  virtual absl::Status WriteScalar(absl::string_view name,
                                   absl::string_view key,
                                   const tstring& val) = 0;

  // Writes a tensor for the given key.
  virtual absl::Status WriteTensor(absl::string_view key,
                                   const Tensor& val) = 0;
  virtual absl::Status WriteTensor(absl::string_view name,
                                   absl::string_view key,
                                   const Tensor& val) = 0;

  virtual ~IteratorStateWriter() = default;

 protected:
  // Accessible only through derived concrete class's copy/move constructors
  IteratorStateWriter() = default;
  IteratorStateWriter(const IteratorStateWriter&) = default;
  IteratorStateWriter(IteratorStateWriter&&) = default;
};

// Generates a full name key for iterator checkpointing. All keys generated for
// iterator checkpoints should go through this function.
std::string FullName(const std::string& prefix, const std::string& name);

// Extracts iterator prefix from key generated by `FullName`.
absl::Status ExtractIteratorPrefix(absl::string_view key, std::string* prefix);

// Interface for objects that can be checkpointed.
class Checkpointable {
 public:
  Checkpointable() = default;
  virtual ~Checkpointable() = default;

  virtual absl::Status Save(SerializationContext* ctx,
                            IteratorStateWriter* writer) = 0;
  virtual absl::Status Restore(IteratorContext* ctx,
                               IteratorStateReader* reader) = 0;
};

// Wrapper around GraphDefBuilder. Used to serialize Dataset graph.
class GraphDefBuilderWrapper {
 public:
  explicit GraphDefBuilderWrapper(GraphDefBuilder* b) : b_(b) {}

  // Adds a Const node with scalar value to the Graph.
  // `*output` contains a pointer to the output `Node`. It is guaranteed to be
  // non-null if the method returns with an OK status.
  // The returned Node pointer is owned by the backing Graph of GraphDefBuilder.
  template <typename T>
  absl::Status AddScalar(const T& val, Node** output) {
    Tensor val_t = Tensor(DataTypeToEnum<T>::v(), TensorShape({}));
    val_t.scalar<T>()() = val;
    AddTensorInternal(val_t, output);
    if (*output == nullptr) {
      return absl::InternalError("AddScalar: Failed to build Const op.");
    }
    return absl::OkStatus();
  }

  // Adds a Const node with vector value to the Graph.
  // `*output` contains a pointer to the output `Node`. It is guaranteed to be
  // non-null if the method returns with an OK status.
  // The returned Node pointer is owned by the backing Graph of GraphDefBuilder.
  // TODO(shivaniagrawal): Consider changing to gtl::ArraySlice?
  template <typename T>
  absl::Status AddVector(const std::vector<T>& val, Node** output) {
    Tensor val_t = Tensor(DataTypeToEnum<T>::v(),
                          TensorShape({static_cast<int64_t>(val.size())}));
    for (size_t i = 0; i < val.size(); i++) {
      val_t.flat<T>()(i) = val[i];
    }
    AddTensorInternal(val_t, output);
    if (*output == nullptr) {
      return absl::InternalError("AddVector: Failed to build Const op.");
    }
    return absl::OkStatus();
  }

  absl::Status AddVector(const std::vector<std::string>& val, Node** output) {
    Tensor val_t = Tensor(DataTypeToEnum<tstring>::v(),
                          TensorShape({static_cast<int64_t>(val.size())}));
    for (size_t i = 0; i < val.size(); i++) {
      val_t.flat<tstring>()(i) = val[i];
    }
    AddTensorInternal(val_t, output);
    if (*output == nullptr) {
      return absl::InternalError("AddVector: Failed to build Const op.");
    }
    return absl::OkStatus();
  }

  // Adds a `Const` node for the given tensor value to the graph.
  //
  // `*output` contains a pointer to the output `Node`. It is guaranteed to be
  // non-null if the method returns with an OK status. The returned `Node`
  // pointer is owned by the backing graph of `GraphDefBuilder`.
  absl::Status AddTensor(const Tensor& val, Node** output) {
    AddTensorInternal(val, output);
    if (*output == nullptr) {
      return absl::InternalError("AddTensor: Failed to build Const op.");
    }
    return absl::OkStatus();
  }

  // Adds a `Placeholder` node for the given tensor value to the graph.
  //
  // `*output` contains a pointer to the output `Node`. It is guaranteed to be
  // non-null if the method returns with an OK status. The returned `Node`
  // pointer is owned by the backing graph of `GraphDefBuilder`.
  absl::Status AddPlaceholder(const Tensor& val, Node** output) {
    AddPlaceholderInternal(val, output);
    if (*output == nullptr) {
      return absl::InternalError(
          "AddPlaceholder: Failed to build Placeholder op.");
    }
    return absl::OkStatus();
  }

  // Adds a node for the given dataset to the `Graph`. The value of
  // `DatasetBase::type_string()` is used as the op type for the node. Values
  // for the `output_types` and `output_shapes` node attributes are also written
  // if those attributes are defined in the `OpDef`.
  //
  // If `use_dataset_name` is set, the value of `DatasetBase::node_name()` is
  // used as the op name for the node. This argument should only be set when
  // serializing `DatasetBase` instances which might not have been created
  // through op kernel execution to make sure the dataset op name is preserved
  // across serialization boundaries, which is in turn needed to make sure
  // iterator checkpoints are valid across serialization boundaries. When
  // `use_dataset_name` is set, the caller is responsible for making sure that
  // the op name is unique across the graph.
  //
  // `*output` contains a pointer to the output `Node`. It is guaranteed to be
  // non-null if the method returns with an OK status. The returned `Node`
  // pointer is owned by the backing `Graph` of `GraphDefBuilder`.
  absl::Status AddDataset(const DatasetBase* dataset,
                          const std::vector<Node*>& inputs, Node** output);
  absl::Status AddDataset(
      const DatasetBase* dataset, const std::vector<Node*>& inputs,
      const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
      Node** output);
  absl::Status AddDataset(
      const DatasetBase* dataset,
      const std::vector<std::pair<size_t, Node*>>& inputs,
      const std::vector<std::pair<size_t, absl::Span<Node* const>>>&
          list_inputs,
      const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
      Node** output);
  absl::Status AddDataset(
      const DatasetBase* dataset,
      const std::vector<std::pair<size_t, Node*>>& inputs,
      const std::vector<std::pair<size_t, absl::Span<Node* const>>>&
          list_inputs,
      const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
      bool use_dataset_name, Node** output);

  // Adds a user-defined function with name `function_name` to the graph and
  // recursively adds all functions it references. If a function with a matching
  // name has already been added, returns with OK status. If a user-defined with
  // name `function_name` is not found in the context's function library,
  // returns an InvalidArgumentError. If the function with name `function_name`
  // or any of its dependent functions are stateful, and the context does not
  // explicitly permit stateful functions, returns an InvalidArgument error.
  absl::Status AddFunction(SerializationContext* ctx,
                           const std::string& function_name,
                           const FunctionLibraryDefinition& lib_def);

  template <typename T>
  void BuildAttrValue(const T& value, AttrValue* attr) {
    SetAttrValue(value, attr);
  }

  template <typename T>
  AttrValue BuildAttrValue(const T& value) {
    AttrValue attr;
    SetAttrValue(value, &attr);
    return attr;
  }

 protected:
  GraphDefBuilder* builder() { return b_; }

 private:
  void AddPlaceholderInternal(const Tensor& val, Node** output);
  void AddTensorInternal(const Tensor& val, Node** output);
  bool HasAttr(const std::string& op_type_name,
               const std::string& attr_name) const;

  bool HasAttr(const OpDef* op_def, const std::string& attr_name) const {
    for (const auto& attr : op_def->attr()) {
      if (attr.name() == attr_name) {
        return true;
      }
    }
    return false;
  }

  absl::Status AddAttrFunctions(SerializationContext* ctx,
                                const AttrValue& attr_value,
                                const FunctionLibraryDefinition& lib_def) {
    if (attr_value.has_func()) {
      TF_RETURN_IF_ERROR(AddFunction(ctx, attr_value.func().name(), lib_def));
    } else if (attr_value.has_list()) {
      for (const NameAttrList& name_attr_list : attr_value.list().func()) {
        TF_RETURN_IF_ERROR(AddFunction(ctx, name_attr_list.name(), lib_def));
      }
    }
    return absl::OkStatus();
  }

  GraphDefBuilder* b_;
};

class StatsAggregator;

// A utility class for running a function and ensuring that there is always a
// `tensorflow::data` symbol on the stack.
class Runner {
 public:
  virtual ~Runner() = default;

  // Runs the given function.
  virtual void Run(const std::function<void()>& f) = 0;

  // Returns a global singleton Runner.
  static Runner* get();
};

// A class which provides a sequence of splits. Splits represent subdivisions of
// a dataset, e.g. filenames or ranges within files. We use splitting to
// partition input data into smaller pieces for distributed processing (see
// go/tf-data-splitting-design). The SplitProvider subclasses are expected to be
// thread-safe.
//
// Datasets provide a `MakeSplitProvider` method to expose a listing of their
// splits.
//
// Iterators created with a split provider will only iterate over the splits
// provided by the split provider.
class SplitProvider {
 public:
  virtual ~SplitProvider() = default;
  // Stores the next split in `*split`, setting `*end_of_splits` to indicate
  // whether there were any splits left.
  virtual absl::Status GetNext(Tensor* split, bool* end_of_splits) = 0;
  // Resets the split provider to its beginning.
  virtual absl::Status Reset() = 0;
  // Saves the state of this split provider.
  virtual absl::Status Save(std::function<std::string(std::string)> full_name,
                            IteratorStateWriter* writer) = 0;
  // Restores the state of this split provider.
  virtual absl::Status Restore(
      std::function<std::string(std::string)> full_name,
      IteratorStateReader* reader) = 0;
  // Returns the number of splits:
  // - If there are a finite number of splits, returns a non-negative count.
  // - If there are an infinite number of splits, returns kInfiniteCardinality.
  // - If the number of splits is unknown or can't be efficiently computed,
  // returns kUnknownCardinality.
  virtual int64_t Cardinality() const { return kUnknownCardinality; }
  // Cancels the split provider. After cancelling, all other existing and future
  // calls should return quickly without blocking.
  virtual void Cancel() {}
  // Used to determine if the split provider is dynamic. Dynamic split providers
  // are expected to be non-deterministic and may return different splits upon
  // reinitialization.
  virtual bool IsDynamic() const { return false; }
};

// Returns the runner threadpool size from an OpKernelContext.
int32_t GetRunnerThreadpoolSizeFromOpKernelContext(OpKernelContext* ctx);

// In-memory representation of a checkpoint. The checkpoint is represented as a
// collection of key-value pairs and are expected to be written using the
// `IteratorStateWriter` interface.
//
// The implementation is not thread-safe.
class MemoryCheckpoint final : public IteratorStateWriter {
 public:
  // IdRegistry maintains a bi-directional mapping between string and integer
  // representations of checkpoint keys.
  //
  // The reason we need both is that integer ids are used for fast lookups and
  // comparisons, while string ids are used for prefix matching.
  class IdRegistry {
   public:
    IdRegistry() = default;

    // Adds the given string id to the registry, generating a unique integer id
    // for it. If the string id already exists, its integer id is returned.
    int64_t Add(const std::string& prefix, const std::string& key);

    // Gets all integer ids for string ids matching the given prefix.
    std::vector<int64_t> GetMatchingIds(const std::string& prefix_to_match);

    // Gets the string id for the given integer id.
    std::pair<std::string, std::string> Get(int64_t id);

    // Removes the entries matching the given integer ids from the registry.
    void RemoveIds(const std::vector<int64_t>& ids);

   private:
    mutex mu_;
    int64_t next_id_ TF_GUARDED_BY(mu_) = 0;
    absl::flat_hash_map<int64_t, std::pair<std::string, std::string>>
        int_to_string_ TF_GUARDED_BY(mu_);
    absl::flat_hash_map<std::pair<std::string, std::string>, int64_t>
        string_to_int_ TF_GUARDED_BY(mu_);
  };

  MemoryCheckpoint() = delete;
  explicit MemoryCheckpoint(std::shared_ptr<IdRegistry> registry)
      : id_registry_(registry) {}

  MemoryCheckpoint(MemoryCheckpoint&& other) = default;
  MemoryCheckpoint(const MemoryCheckpoint& other) = default;

  static MemoryCheckpoint CreateRootCheckpoint(
      std::shared_ptr<IdRegistry> registry) {
    return MemoryCheckpoint(/*id_registry*/ registry, /*is_root=*/true);
  }

  // BEGIN implementation of `IteratorStateWriter` interface
  absl::Status WriteScalar(absl::string_view key, int64_t val) override {
    std::string prefix;
    TF_RETURN_IF_ERROR(ExtractIteratorPrefix(key, &prefix));
    return WriteScalar(prefix, key, val);
  }
  absl::Status WriteScalar(absl::string_view name, absl::string_view key,
                           int64_t val) override {
    auto id = id_registry_->Add(std::string(name), std::string(key));
    int_values_[id] = val;
    return absl::OkStatus();
  }
  absl::Status WriteScalar(absl::string_view key, const tstring& val) override {
    std::string prefix;
    TF_RETURN_IF_ERROR(ExtractIteratorPrefix(key, &prefix));
    return WriteScalar(prefix, key, val);
  }
  absl::Status WriteScalar(absl::string_view name, absl::string_view key,
                           const tstring& val) override {
    auto id = id_registry_->Add(std::string(name), std::string(key));
    str_values_[id] = val;
    return absl::OkStatus();
  }
  absl::Status WriteTensor(absl::string_view key, const Tensor& val) override {
    std::string prefix;
    TF_RETURN_IF_ERROR(ExtractIteratorPrefix(key, &prefix));
    return WriteTensor(prefix, key, val);
  }
  absl::Status WriteTensor(absl::string_view name, absl::string_view key,
                           const Tensor& val) override {
    auto id = id_registry_->Add(std::string(name), std::string(key));
    tensor_values_[id] = val;
    return absl::OkStatus();
  }
  // END implementation of `IteratorStateWriter` interface

  // String representation for the in-memory checkpoint suitable for debugging.
  std::string DebugString() const;

  // Returns the status of the in-memory checkpoint.
  absl::Status GetStatus() const { return status_; }

  // Merges state of another checkpoint into this checkpoint, overwriting
  // existing state (if applicable).
  //
  // Merge also garbage collects state that is no longer needed.
  void Merge(MemoryCheckpoint* other);

  // Purge removes all keys with given prefix from checkpoint. It also adds the
  // prefix for tracking unless it is the root checkpoint.
  void Purge(const std::string& prefix);

  // Stores the in-memory checkpoint to the given writer.
  absl::Status Save(IteratorStateWriter* writer) const;

  // Updates the status of the in-memory checkpoint with the given status.
  void UpdateStatus(absl::Status status) { status_.Update(status); }

 private:
  explicit MemoryCheckpoint(std::shared_ptr<IdRegistry> registry, bool is_root)
      : is_root_(is_root), id_registry_(registry) {}
  void operator=(const MemoryCheckpoint&) = delete;

  absl::Status status_ = absl::OkStatus();
  // Only set to true for the checkpoint in IteratorResource.
  // Root checkpoint does not track expired prefixes.
  const bool is_root_ = false;
  absl::flat_hash_map<int64_t, int64_t> int_values_;
  absl::flat_hash_map<int64_t, std::string> str_values_;
  absl::flat_hash_map<int64_t, Tensor> tensor_values_;

  // Keeps track of expired prefixes for propagation. Cleaned after it's merged.
  absl::flat_hash_set<std::string> expired_prefixes_;

  std::shared_ptr<IdRegistry> id_registry_;
};

// Aggregates runtime support needed for dataset and iterator serialization.
class SerializationContext {
 public:
  // Handles the external state according to the external state policy.
  absl::Status HandleCheckExternalStateStatus(absl::Status s) {
    if (s.ok()) {
      return s;
    }
    switch (params_.external_state_policy) {
      case ExternalStatePolicy::POLICY_WARN:
        LOG(WARNING) << s.ToString();
        return absl::OkStatus();
      case ExternalStatePolicy::POLICY_IGNORE:
        VLOG(2) << "Ignoring error status: " << s.ToString();
        return absl::OkStatus();
      case ExternalStatePolicy::POLICY_FAIL:
        return s;
      default:
        return absl::InvalidArgumentError(
            absl::StrCat("Unexpected value of external policy: ",
                         params_.external_state_policy));
    }
  }

  struct Params {
    explicit Params() = default;

    explicit Params(OpKernelContext* ctx)
        : resource_mgr(ctx->resource_manager()),
          device_name(ctx->device()->attributes().name()) {}

    std::vector<std::pair<std::string, Tensor>>* input_list =
        nullptr;  // Not owned.

    // Indicates what to do if the dataset depends on external state.
    ExternalStatePolicy external_state_policy =
        ExternalStatePolicy::POLICY_WARN;

    // Indicates whether the serialization is for rewrites.
    //
    // If true:
    //   * A dataset that doesn't implement serialization is replaced with a
    //     placeholder returned in `input_list`.
    //   * Data tensors are replaced with a placeholder returned in
    //     `input_list`.
    //   * Datasets that use random seeds should not serialize the random seeds.
    //     This doesn't affect datasets that use fixed seeds; fixed seeds will
    //     always be preserved.
    //   * Cardinality is serialized as an unregistered attribute
    //     `_cardinality`.
    // If false:
    //   * A dataset that doesn't implement serialization should result in an
    //     error.
    //   * Data tensors (potentially large) should be serialized.
    //   * Datasets that use random seeds should serialize the random seeds.
    bool is_graph_rewrite = false;

    // A resource manager for looking up resources during serialization.
    ResourceMgr* resource_mgr;

    // The name of the device doing the serialization.
    std::string device_name;

    // Determines whether checkpointing should represent input pipeline state
    // symbolically, using cursors into source iterators, or explicitly, by
    // storing internal state of each iterator.
    bool symbolic_checkpoint = false;
  };

  explicit SerializationContext(Params params) : params_(params) {}

  std::vector<std::pair<std::string, Tensor>>* input_list() {
    return params_.input_list;
  }

  ExternalStatePolicy external_state_policy() const {
    return params_.external_state_policy;
  }

  bool is_graph_rewrite() const { return params_.is_graph_rewrite; }

  const ResourceMgr* resource_mgr() const { return params_.resource_mgr; }

  const std::string& device_name() const { return params_.device_name; }

  bool symbolic_checkpoint() const { return params_.symbolic_checkpoint; }

 private:
  Params params_;

  SerializationContext(const SerializationContext&) = delete;
  void operator=(const SerializationContext&) = delete;
};

// Specifies the tf.data pipeline run mode.
enum RunMode { DEFAULT, STANDALONE };

// A cut-down version of `OpKernelContext` for running computations in
// iterators. Note that we cannot simply use `OpKernelContext` here because we
// might run computation in an iterator whose lifetime is not nested within the
// lifetime of a single `OpKernelContext` (e.g. asynchronous prefetching).
//
// TODO(mrry): We're making some daring assumptions about the lifetime of the
// runner passed in here. A runner will be deleted when the original step ends,
// but all existing runners only close over session-lifetime (or longer-lived)
// state, so we can make a copy of the function. There's nothing in the
// definition of the API from which we took the runner to guarantee that what we
// are doing is safe. We should formalize the properties here.
class IteratorContext {
 public:
  struct Params {
    explicit Params(IteratorContext* ctx)
        : accelerator_device_info(ctx->accelerator_device_info()),
          allocator_getter(ctx->allocator_getter()),
          cancellation_manager(ctx->cancellation_manager()),
          collective_executor(ctx->collective_executor()),
          env(ctx->env()),
          flr(ctx->flr()),
          function_handle_cache(ctx->function_handle_cache()),
          interleave_depth(ctx->interleave_depth()),
          is_restoring(ctx->is_restoring()),
          model(ctx->model()),
          options(ctx->options()),
          ram_budget_manager(ctx->ram_budget_manager()),
          resource_mgr(ctx->resource_mgr()),
          runner(*(ctx->runner())),
          runner_threadpool_size(ctx->runner_threadpool_size()),
          split_providers(ctx->split_providers()),
          stats_aggregator(ctx->stats_aggregator()),
          symbolic_checkpoint(ctx->symbolic_checkpoint()),
          thread_factory(ctx->thread_factory()),
          thread_pool(ctx->thread_pool()),
          id_registry(ctx->id_registry()),
          warm_start(ctx->warm_start()),
          index_mapper(ctx->index_mapper()),
          data_service_address(ctx->data_service_address()) {}

    explicit Params(OpKernelContext* ctx)
        : collective_executor(ctx->collective_executor()),
          env(ctx->env()),
          flr(ctx->function_library()) {
      // NOTE: need reinterpret_cast because function.h forward-declares Device.
      DeviceBase* device =
          reinterpret_cast<DeviceBase*>(ctx->function_library()->device());
      accelerator_device_info = device->tensorflow_accelerator_device_info();
      allocator_getter = [device](AllocatorAttributes attrs) {
        return device->GetAllocator(attrs);
      };

      runner_threadpool_size = GetRunnerThreadpoolSizeFromOpKernelContext(ctx);

      // NOTE: Wrap every runner invocation in a call to Runner()->Run(), so
      // that a symbol in the tensorflow::data namespace is always on the stack
      // when executing a function inside a Dataset.
      runner = std::bind(
          [](
              // Note: `runner` is a const reference to avoid copying it.
              const std::function<void(std::function<void()>)>& ctx_runner,
              std::function<void()> fn) {
            std::function<void()> wrapped_fn = std::bind(
                [](const std::function<void()>& fn) { Runner::get()->Run(fn); },
                std::move(fn));
            ctx_runner(std::move(wrapped_fn));
          },
          *ctx->runner(), std::placeholders::_1);
    }

    // If non-null, information about the GPU or TPU on which the op is placed.
    const DeviceBase::AcceleratorDeviceInfo* accelerator_device_info = nullptr;

    // The Allocator to be used to allocate the output of an iterator.
    std::function<Allocator*(AllocatorAttributes)> allocator_getter = nullptr;

    // The CancellationManager to be used to cancel execution of ops.
    CancellationManager* cancellation_manager = nullptr;

    // Collective support.
    CollectiveExecutor* collective_executor = nullptr;

    // Interface to operating system functionality.
    Env* env = nullptr;

    // The FunctionLibraryRuntime object to be used to make function calls.
    FunctionLibraryRuntime* flr = nullptr;

    // A FunctionHandleCache that owns all the function handles. Not owned.
    FunctionHandleCache* function_handle_cache = nullptr;

    // Records the number of ParallelInterleave operations in the path from the
    // root node to this node (not including this node) in the input pipeline
    // tree.
    int64_t interleave_depth = 0;

    // Marks whether the iterator is restored from a checkpoint.
    bool is_restoring = false;

    // If non-null, identifies the object used for performance modeling.
    std::shared_ptr<model::Model> model = nullptr;

    // The input pipeline options.
    const Options* options = nullptr;

    // Manager for the ram budget when using autotune.
    std::shared_ptr<model::RamBudgetManager> ram_budget_manager = nullptr;

    // A resource manager for storing dataset-related state, e.g. random
    // seeds or cached tensors. Not owned.
    ResourceMgr* resource_mgr = nullptr;

    // Function call support.
    std::function<void(std::function<void()>)> runner = nullptr;

    // Number of threads used for executing user-defined functions.
    int32_t runner_threadpool_size = 0;

    // Split providers indicating which splits to process. May be empty,
    // indicating that the iterator should process all splits.
    std::vector<std::shared_ptr<SplitProvider>> split_providers;

    // The `StatsAggregator` object to record statistics about the iterator.
    //
    // TODO(b/147325552): Remove this API and any of its uses after we switch to
    // using C++ based implementation for tf.data options (on 4/12/2021).
    std::shared_ptr<StatsAggregator> stats_aggregator = nullptr;

    // Indicates whether to use symbolic checkpointing.
    bool symbolic_checkpoint = false;

    // A factory for creating threads to perform blocking work.
    std::shared_ptr<ThreadFactory> thread_factory = nullptr;

    // A shared thread pool to schedule computation into.
    thread::ThreadPoolInterface* thread_pool = nullptr;

    std::shared_ptr<MemoryCheckpoint::IdRegistry> id_registry =
        std::make_shared<MemoryCheckpoint::IdRegistry>();

    // If `true` background threads of asynchronous operations are started when
    // the iterator is created. Otherwise, they are started upon first `GetNext`
    // request. Default value is set to false to ensure backward compatibility.
    bool warm_start = false;

    // Specifies the tf.data pipeline run mode.
    RunMode run_mode = RunMode::DEFAULT;

    // Maps the index of dataset elements to a shuffled index. In other words,
    // given an index i, returns the permuted index p(i) for the iterator. Used
    // to support global shuffling of datasets that support random access.
    IndexMapperFn index_mapper = nullptr;

    // Records the number of elements that have been produced prior to a
    // checkpoint. This is set by globally shuffled iterators so that upstream
    // iterators can restore the element counts in the random access mode.
    std::optional<size_t> restored_element_count = std::nullopt;

    // The address of the tf.data service job.
    std::string data_service_address;
  };

  explicit IteratorContext(IteratorContext* ctx)
      : IteratorContext(Params{ctx}) {}

  explicit IteratorContext(OpKernelContext* ctx)
      : IteratorContext(Params{ctx}) {}

  explicit IteratorContext(Params params)
      : params_(std::move(params)),
        checkpoint_(MemoryCheckpoint{params_.id_registry}) {}

  IteratorContext(const IteratorContext& other)
      : IteratorContext(Params{other.params_}) {
    // MemoryCheckpoint should not be copied over as the child context should
    // not care what's in the checkpoint of parent context.
  }

  std::shared_ptr<MemoryCheckpoint::IdRegistry> id_registry() {
    return params_.id_registry;
  }

  const DeviceBase::AcceleratorDeviceInfo* accelerator_device_info() {
    return params_.accelerator_device_info;
  }

  Allocator* allocator(AllocatorAttributes attrs) {
    return params_.allocator_getter(attrs);
  }

  std::function<Allocator*(AllocatorAttributes)> allocator_getter() {
    return params_.allocator_getter;
  }

  CancellationManager* cancellation_manager() {
    return params_.cancellation_manager;
  }

  CollectiveExecutor* collective_executor() {
    return params_.collective_executor;
  }

  Env* env() const { return params_.env; }

  FunctionLibraryRuntime* flr() { return params_.flr; }

  FunctionHandleCache* function_handle_cache() {
    return params_.function_handle_cache;
  }

  MemoryCheckpoint* checkpoint() { return &checkpoint_; }

  int64_t interleave_depth() { return params_.interleave_depth; }

  bool is_restoring() { return params_.is_restoring; }

  const std::shared_ptr<model::Model>& model() const { return params_.model; }

  const Options* options() const { return params_.options; }

  const std::shared_ptr<model::RamBudgetManager>& ram_budget_manager() {
    return params_.ram_budget_manager;
  }

  ResourceMgr* resource_mgr() { return params_.resource_mgr; }

  std::function<void(std::function<void()>)>* runner() {
    return &params_.runner;
  }

  int32_t runner_threadpool_size() { return params_.runner_threadpool_size; }

  std::vector<std::shared_ptr<SplitProvider>> split_providers() const {
    return params_.split_providers;
  }

  std::shared_ptr<StatsAggregator> stats_aggregator() {
    return params_.stats_aggregator;
  }

  bool symbolic_checkpoint() { return params_.symbolic_checkpoint; }

  const std::shared_ptr<ThreadFactory>& thread_factory() {
    return params_.thread_factory;
  }

  thread::ThreadPoolInterface* thread_pool() { return params_.thread_pool; }

  bool warm_start() { return params_.warm_start; }

  RunMode run_mode() { return params_.run_mode; }

  IndexMapperFn index_mapper() const { return params_.index_mapper; }

  void set_restored_element_count(size_t element_count) {
    params_.restored_element_count.emplace(element_count);
  }

  std::optional<int64_t> restored_element_count() const {
    return params_.restored_element_count;
  }

  std::string data_service_address() { return params_.data_service_address; }

  void SetModel(std::shared_ptr<model::Model> model) { params_.model = model; }

  void SetIndexMapper(const IndexMapperFn& index_mapper) {
    params_.index_mapper = index_mapper;
  };

  std::unique_ptr<thread::ThreadPool> CreateThreadPool(const std::string& name,
                                                       int num_threads) {
    if (params_.thread_pool) {
      // Create a `ThreadPool` instance by wrapping `params_.thread_pool` (which
      // is an instance of `thread::ThreadPoolInterface`). Notably, the
      // ownership of `params_.thread_pool` is *not* transferred onto the newly
      // created `ThreadPool` instance.
      return std::make_unique<thread::ThreadPool>(params_.thread_pool);
    } else {
      return std::make_unique<thread::ThreadPool>(params_.env, ThreadOptions(),
                                                  name, num_threads,
                                                  /*low_latency_hint=*/false);
    }
  }

  // Merges the given checkpoint with the checkpoint of this context.
  //
  // The intended for this API is that methods, such as
  // `IteratorBase::Initialize`, `IteratorBase::GetNextInternal`, or
  // `IteratorBase::RestoreInternal` that store data in the in-memory
  // checkpoint, use a separate instance of `IteratorContext` for a nested call,
  // then the checkpoint collected by the `IteratorContext` instance passed into
  // the callee should be merged into the `IteratorContext` of the caller:
  //
  // ```
  // Status GetNextInternal(IteratorContext* ctx, ...) {
  //   ...
  //   IteratorContext nested_ctx(...);
  //   TF_RETURN_IF_ERROR(input_impl_->GetNext(&nested_ctx, ...));
  //   ctx->MergeCheckpoint(nested_ctx->checkpoint());
  //   ...
  // }
  // ```
  void MergeCheckpoint(MemoryCheckpoint* checkpoint) {
    if (symbolic_checkpoint()) {
      checkpoint_.Merge(checkpoint);
    }
  }

  // Removes any keys with the given prefix from the checkpoint.
  //
  // The intended use for this API is to clean the stale state in checkpoint,
  // e.g. when a pipeline created by `flat_map` is exhausted, the state
  // associated with the iterator of that pipeline is no longer needed and
  // should be removed.
  void PurgeCheckpoint(const std::string& prefix) {
    if (symbolic_checkpoint()) {
      checkpoint_.Purge(prefix);
    }
  }

  // Saves the state of the given iterator into the checkpoint.
  void SaveCheckpoint(Checkpointable* iterator) {
    if (symbolic_checkpoint()) {
      SerializationContext::Params params;
      params.symbolic_checkpoint = true;
      SerializationContext ctx(std::move(params));
      checkpoint_.UpdateStatus(iterator->Save(&ctx, &checkpoint_));
    }
  }

  std::unique_ptr<Thread> StartThread(const std::string& name,
                                      std::function<void()> fn) {
    if (params_.thread_factory) {
      return params_.thread_factory->StartThread(name, std::move(fn));
    } else {
      return absl::WrapUnique(
          Env::Default()->StartThread({}, name, std::move(fn)));
    }
  }

  // Updates the status of the checkpoint with the given status.
  void UpdateCheckpointStatus(std::function<absl::Status()> status_fn) {
    if (symbolic_checkpoint()) {
      checkpoint_.UpdateStatus(status_fn());
    }
  }

 private:
  Params params_;
  MemoryCheckpoint checkpoint_;
};

// Generic context that can be constructed with either an `OpKernelContext` or
// `IteratorContext`.
struct AnyContext {
  Allocator* allocator;
  std::function<void(std::function<void()>)>* runner;
  int64_t runner_threadpool_size;

  explicit AnyContext(IteratorContext* ctx) {
    allocator = ctx->allocator({});
    runner = ctx->runner();
    runner_threadpool_size = ctx->runner_threadpool_size();
  }

  explicit AnyContext(OpKernelContext* ctx) {
    allocator = ctx->get_allocator({});
    runner = ctx->runner();
    runner_threadpool_size = GetRunnerThreadpoolSizeFromOpKernelContext(ctx);
  }
};

// Represents the current position in a range of outputs, where the
// range of outputs is typically represented by an `DatasetBase`,
// defined below.
class IteratorBase : public Checkpointable {
 public:
  ~IteratorBase() override {
    for (auto rit = cleanup_fns_.rbegin(); rit != cleanup_fns_.rend(); ++rit) {
      (*rit)();
    }
  }

  // Gets the next output from the range that this iterator is traversing.
  //
  // If at least one output remains in this iterator's range, that
  // output will be stored in `*out_tensors` and `false` will be
  // stored in `*end_of_sequence`.
  //
  // If no more outputs remain in this iterator's range, `true` will be stored
  // in `*end_of_sequence`, and `*out_tensors` will be empty.
  //
  // Implementations should never return `OutOfRange` error. If at end of
  // sequence, set `*end_of_sequence = true` and return `OkStatus()`.
  // Internally raised `OutOfRange` errors that do not imply end of sequence
  // should be converted to a different error type before being propagated to
  // the caller.
  //
  // Implementations must explicitly set `*end_of_sequence = false` if an
  // `OkStatus()` status is returned and the iterator is not at the end of the
  // sequence.
  //
  // `out_tensors` and `end_of_sequence` are output parameters. `*out_tensors`
  // and `*end_of_sequence` should not be read by implementations of `GetNext`
  // before they are assigned.
  //
  // This method is thread-safe.
  //
  // TODO(mrry): Define `GetNextAsync()` or `GetNextManyAsync()`, and
  // potentially remove this method.
  virtual absl::Status GetNext(IteratorContext* ctx,
                               std::vector<Tensor>* out_tensors,
                               bool* end_of_sequence) = 0;

  absl::Status GetNext(IteratorContext&& ctx, std::vector<Tensor>* out_tensors,
                       bool* end_of_sequence) {
    return GetNext(&ctx, out_tensors, end_of_sequence);
  }

  // If a dataset needs to provide its own index mapper behavior to support
  // global shuffling, implement this method.
  virtual IndexMapperFn GetIndexMapper(
      IndexMapperFn parent_index_mapper) const {
    return parent_index_mapper;
  }

  // Skips the next `num_to_skip` outputs from the range that this iterator
  // is traversing.
  //
  // If there are not enough outputs to skip, it will set
  // `*end_of_sequence = true` and return `OkStatus()`. `*num_skipped` will
  // store the number of outputs that are skipped. When `*end_of_sequence` is
  // `false`, `*num_skipped` should equal to `num_to_skip`.
  virtual absl::Status Skip(IteratorContext* ctx, int num_to_skip,
                            bool* end_of_sequence, int* num_skipped) = 0;

  virtual absl::Status Skip(IteratorContext&& ctx, int num_to_skip,
                            bool* end_of_sequence, int* num_skipped) {
    return Skip(&ctx, num_to_skip, end_of_sequence, num_skipped);
  }

  // Returns a vector of DataType values, representing the respective
  // element types of each tuple component in the outputs of this
  // iterator.
  virtual const DataTypeVector& output_dtypes() const = 0;

  // Returns a vector of tensor shapes, representing the respective
  // (and possibly partially defined) shapes of each tuple component
  // in the outputs of this iterator.
  virtual const std::vector<PartialTensorShape>& output_shapes() const = 0;

  // Returns a string that identifies the sequence of iterators leading up to
  // this iterator.
  virtual const std::string& prefix() const = 0;

  // Indicates whether the iterator is compatible with symbolic checkpointing.
  virtual bool SymbolicCheckpointCompatible() const { return false; }

  // Performs initialization that needs to happen outside of a constructor to
  // properly propagate errors.
  virtual absl::Status Initialize(IteratorContext* ctx) {
    return absl::OkStatus();
  }

  // Performs initialization of the base iterator.
  absl::Status InitializeBase(IteratorContext* ctx, const IteratorBase* parent);

  // Saves the state of this iterator.
  absl::Status Save(SerializationContext* ctx,
                    IteratorStateWriter* writer) override {
    int64_t start_us = EnvTime::NowMicros();
    TF_RETURN_IF_ERROR(SaveInternal(ctx, writer));
    VLOG(1) << "Saved " << prefix() << " in "
            << (EnvTime::NowMicros() - start_us) << "us";
    return absl::OkStatus();
  }

  // Restores the state of this iterator.
  absl::Status Restore(IteratorContext* ctx,
                       IteratorStateReader* reader) override {
    int64_t start_us = EnvTime::NowMicros();
    TF_RETURN_IF_ERROR(RestoreInternal(ctx, reader));
    ctx->SaveCheckpoint(this);
    VLOG(1) << "Restored " << prefix() << " in "
            << (EnvTime::NowMicros() - start_us) << "us";
    return absl::OkStatus();
  }

  // Returns the total number of bytes buffered by the iterator across all nodes
  // in the subtree for which autotuning is enabled.
  int64_t TotalBufferedBytes() const {
    if (node_) return node_->TotalBufferedBytes();
    return 0;
  }

 protected:
  // Returns a node that models this iterator.
  virtual std::shared_ptr<model::Node> CreateNode(
      IteratorContext* ctx, model::Node::Args args) const = 0;

  // This is needed so that sub-classes of IteratorBase can call
  // `SaveInternal` on their input iterators.
  absl::Status SaveInput(SerializationContext* ctx, IteratorStateWriter* writer,
                         const std::unique_ptr<IteratorBase>& input) {
    if (ctx->symbolic_checkpoint()) {
      return absl::OkStatus();
    }
    return input->Save(ctx, writer);
  }

  // This is needed so that sub-classes of IteratorBase can call
  // `RestoreInternal` on their input iterators.
  absl::Status RestoreInput(IteratorContext* ctx, IteratorStateReader* reader,
                            const std::unique_ptr<IteratorBase>& input) {
    return input->Restore(ctx, reader);
  }

  absl::Status RestoreInput(IteratorContext&& ctx, IteratorStateReader* reader,
                            const std::unique_ptr<IteratorBase>& input) {
    return RestoreInput(&ctx, reader, input);
  }

  // Saves the state of this iterator.
  //
  // This method is used to store the state of the iterator in a checkpoint.
  // implementations have an override.
  virtual absl::Status SaveInternal(SerializationContext* ctx,
                                    IteratorStateWriter* writer) = 0;

  // Restores the state of this iterator.
  //
  // This method is used to restore the state of the iterator from a checkpoint.
  //
  // Implementations may assume that the iterator is in a clean state. That is,
  // its `Initialize` method has been called, but its `GetNext` method has
  // never been called.
  // implementations have an override.
  virtual absl::Status RestoreInternal(IteratorContext* ctx,
                                       IteratorStateReader* reader) = 0;

  // Returns a pointer to the node representing this iterator in the performance
  // model. It may be null, if performance modeling is not enabled for this
  // iterator.
  std::shared_ptr<model::Node> model_node() const { return node_; }

  // Returns the number of elements produced by this iterator.
  int64_t num_elements() const {
    if (node_) return node_->num_elements();
    return 0;
  }

  std::shared_ptr<model::Node> node_ = nullptr;

 private:
  // For access to `AddCleanupFunction` and `Restore`.
  friend class DatasetBase;
  friend class DatasetBaseIterator;  // for access to `node_`

  std::vector<std::function<void()>> cleanup_fns_;
  const IteratorBase* parent_ = nullptr;  // Not owned.
  uint64_t id_ = 0;
  uint64_t parent_id_ = 0;
};

// Represents runtime information needed to construct a dataset.
class DatasetContext {
 public:
  struct Params {
    std::string type_string;  // op type name of this dataset.
    std::string node_name;    // graph node name of this dataset op, uniquely
                              // identifying the dataset in the graph.
  };

  explicit DatasetContext(Params params) : params_(std::move(params)) {}

  explicit DatasetContext(OpKernelContext* ctx) {
    params_.type_string = ctx->op_kernel().type_string();
    params_.node_name = ctx->op_kernel().name();
  }

  const std::string& type_string() const { return params_.type_string; }
  const std::string& node_name() const { return params_.node_name; }

 private:
  Params params_;
};

// Returns the number of bytes allocated for the given tensor.
int64_t GetAllocatedBytes(const std::vector<Tensor>& element);

// Returns the estimated memory usage in bytes of the given tensor.
int64_t GetTotalBytes(const std::vector<Tensor>& element);

// Validates and extracts a `DatasetBase` object from `tensor`.
//
// `tensor` must have been written by a call to SetVariantTensorToDataset().
//
// The retrieved pointer is a borrowed reference to the dataset, which is owned
// by the tensor. The consumer must either acquire its own reference to the
// dataset by calling `(*out_dataset)->Ref()`, or ensure that `tensor` is not
// destroyed or mutated while the retrieved pointer is in use.
absl::Status GetDatasetFromVariantTensor(const Tensor& tensor,
                                         DatasetBase** out_dataset);

// Stores a `DatasetBase` object in `tensor`.
//
// The ownership of `dataset` is transferred to `tensor`.
absl::Status StoreDatasetInVariantTensor(DatasetBase* dataset, Tensor* tensor);

// Represents a (potentially infinite) range of outputs, where each
// output is a tuple of tensors.
class DatasetBase : public core::RefCounted {
 public:
  // Key for storing the Dataset graph in the serialized format.
  TF_EXPORT static const char kDatasetGraphKey[];

  // Key for storing the output node of the Dataset graph in the serialized
  // format.
  TF_EXPORT static const char kDatasetGraphOutputNodeKey[];

  explicit DatasetBase(DatasetContext&& ctx)
      : type_string_(ctx.type_string()), node_name_(ctx.node_name()) {}

  // Op type name of this dataset.
  const std::string& type_string() const { return type_string_; }

  // Graph node name of this dataset op, uniquely identifying the dataset in
  // the graph.
  const std::string& node_name() const { return node_name_; }

  const Metadata& metadata() const { return metadata_; }

  const Options& options() const { return options_; }

  int64_t num_sources() const { return num_sources_; }

  // Initializes the dataset using the given metadata.
  void Initialize(const Metadata& metadata);

  // Returns a new iterator for iterating over the range of elements in
  // this dataset.
  //
  // This method may be called multiple times on the same instance,
  // and the resulting iterators will have distinct state. Each
  // iterator will traverse all elements in this dataset from the
  // start.
  //
  // The prefix identifies the sequence of iterators leading up to the newly
  // created iterator.
  absl::Status MakeIterator(IteratorContext* ctx, const IteratorBase* parent,
                            const std::string& output_prefix,
                            std::unique_ptr<IteratorBase>* iterator) const;

  absl::Status MakeIterator(IteratorContext&& ctx, const IteratorBase* parent,
                            const std::string& output_prefix,
                            std::unique_ptr<IteratorBase>* iterator) const {
    return MakeIterator(&ctx, parent, output_prefix, iterator);
  }

  // Returns a new iterator restored from the checkpoint data in `reader`.
  absl::Status MakeIteratorFromCheckpoint(
      IteratorContext* ctx, const std::string& output_prefix,
      IteratorStateReader* reader,
      std::unique_ptr<IteratorBase>* iterator) const {
    std::unique_ptr<IteratorBase> it;
    IteratorContext::Params params(ctx);
    params.is_restoring = true;
    IteratorContext restore_ctx(std::move(params));
    TF_RETURN_IF_ERROR(MakeIterator(&restore_ctx,
                                    /*parent=*/nullptr, output_prefix, &it));
    TF_RETURN_IF_ERROR(it->Restore(&restore_ctx, reader));
    ctx->MergeCheckpoint(restore_ctx.checkpoint());
    *iterator = std::move(it);
    return absl::OkStatus();
  }

  absl::Status MakeIteratorFromCheckpoint(
      IteratorContext&& ctx, const std::string& output_prefix,
      IteratorStateReader* reader,
      std::unique_ptr<IteratorBase>* iterator) const {
    return MakeIteratorFromCheckpoint(&ctx, output_prefix, reader, iterator);
  }

  // Returns a split provider which partitions the dataset's data into splits
  // and provides them in a sequence. The split provider is stored in
  // `*split_provider`.
  virtual absl::Status MakeSplitProviders(
      std::vector<std::unique_ptr<SplitProvider>>* split_providers) const;

  // Returns a vector of DataType values, representing the respective
  // element types of each tuple component in the outputs of this
  // dataset.
  virtual const DataTypeVector& output_dtypes() const = 0;

  // Returns a vector of tensor shapes, representing the respective
  // (and possibly partially defined) shapes of each tuple component
  // in the outputs of this dataset.
  virtual const std::vector<PartialTensorShape>& output_shapes() const = 0;

  // Returns the number of bytes allocated for tensors of this dataset.
  virtual int64_t AllocatedBytes() const { return 0; }

  // Returns the estimated element size based on `output_shapes()` and
  // `output_dtypes()`.
  virtual std::optional<int64_t> GetEstimatedElementSize() const;

  // Returns the estimated number of bytes used for tensors of this dataset.
  virtual int64_t TotalBytes() const { return 0; }

  // Returns the cardinality of this dataset.
  // TODO(shilpakrish): Remove this overload once all callers are migrated
  // to the API which passes in the options parameter.
  ABSL_DEPRECATED("Use the overload that passes in the options parameter.")
  int64_t Cardinality() const;

  // Returns the cardinality of this dataset based on the options.
  int64_t Cardinality(CardinalityOptions options) const;

  // Internal implementation of cardinality for a dataset based on the options.
  virtual int64_t CardinalityInternal(CardinalityOptions options) const
      TF_EXCLUSIVE_LOCKS_REQUIRED(cardinality_mu_) {
    return kUnknownCardinality;
  }

  // A human-readable debug string for this dataset.
  virtual std::string DebugString() const = 0;

  // Stores the dataset's input datasets in `*inputs`. The pointers stored in
  // `*inputs` are borrowed. The only valid non-ok return status is
  // UNIMPLEMENTED in case `InputDatasets` is not implemented by a dataset
  // subclass. Implementing `InputDatasets` enables `DatasetBase` to provide a
  // default implementation of `MakeSplitProvider` when there is a single input
  // dataset.
  virtual absl::Status InputDatasets(
      std::vector<const DatasetBase*>* inputs) const;

  // Indicates whether the dataset depends on any external state which would
  // prevent it from being serializable. If so, the method returns
  // `errors::FailedPrecondition` with a message that identifies the external
  // state. Otherwise, the method returns `OkStatus()`.
  virtual absl::Status CheckExternalState() const = 0;

  // Indicates whether the dataset is compatible with random access.
  absl::Status CheckRandomAccessCompatible(const int64_t index) const;

  // Return the element at a particular index for a randomly accessible dataset.
  virtual absl::Status Get(OpKernelContext* ctx, int64_t index,
                           std::vector<Tensor>* out_tensors) const;

  // Same as above, but with an `AnyContext`, which can be constructed from
  // either an `OpKernelContext` or `IteratorContext`. Used to support datasets
  // that provide random access through both the dataset and iterator APIs.
  virtual absl::Status Get(AnyContext ctx, int64_t index,
                           std::vector<Tensor>* out_tensors) const;

  // Returns true if the dataset and its inputs support random access.
  virtual absl::Status RandomIndexingCompatible() const {
    return absl::FailedPreconditionError(
        absl::StrCat(type_string(), " does not support random access."));
  }

  // Return a finalized version of the dataset.  The returned DatasetBase is
  // unowned and lives for as long as this dataset.
  virtual absl::StatusOr<DatasetBase*> Finalize(
      OpKernelContext* ctx,
      std::function<absl::StatusOr<core::RefCountPtr<DatasetBase>>()>
          make_finalized_dataset) const;

  // Wrapper around a GraphDefBuilder which provides support for serializing
  // Datasets as GraphDefs.
  class DatasetGraphDefBuilder : public GraphDefBuilderWrapper {
   public:
    explicit DatasetGraphDefBuilder(GraphDefBuilder* b)
        : GraphDefBuilderWrapper(b) {}
    absl::Status AddInputDataset(SerializationContext* ctx,
                                 const DatasetBase* dataset, Node** output);
    absl::Status AddDatasetOrTensor(SerializationContext* ctx,
                                    const Tensor& val, Node** output);
    absl::Status AddIdentity(SerializationContext* ctx,
                             const std::string& name_prefix, Node** input,
                             Node** output);

   private:
    absl::Status AddDatasetOrTensorHelper(SerializationContext* ctx,
                                          const Tensor& val, Node** output);
    absl::Status AddResourceHelper(SerializationContext* ctx, const Tensor& val,
                                   Node** output);
  };

 protected:
  friend class CapturedFunction;

  // Serializes the dataset into a `GraphDef`, which has two uses:
  //
  // 1) To perform static input pipeline optimizations, tf.data serializes the
  // dataset graph, applies graph rewrites, and then deserializes the graph.
  // If a subclass of `DatasetBase` does not implement this method, then it will
  // be excluded from static optimizations (and so will any upstream datasets).
  //
  // 2) To save the dataset so that it can restore at a later point (possibly in
  // different environment). If a subclass of `DatasetBase` does not implement
  // this method, then this migration will not be possible.
  virtual absl::Status AsGraphDefInternal(SerializationContext* ctx,
                                          DatasetGraphDefBuilder* b,
                                          Node** node) const = 0;

  virtual std::unique_ptr<IteratorBase> MakeIteratorInternal(
      const std::string& prefix) const = 0;

  void set_options(const Options& options) { options_ = options; }

 private:
  // Computes and stores the cardinality of a given dataset.
  absl::Status ComputeCardinality();

  // Computes the number of source datasets feeding into this dataset. A source
  // dataset is a leaf in the subtree of dataset inputs.
  absl::Status ComputeNumSources();

  // Merges options from inputs to this dataset. If there is a conflict in a
  // field value, the options set on this dataset takes precedence over those in
  // the inputs. The order of precedence on the inputs is in the same order as
  // how they appear for this dataset.
  absl::Status MergeOptionsFromInputs();

  const std::string type_string_;
  const std::string node_name_;
  Metadata metadata_;
  Options options_;
  mutable mutex mu_;
  mutable mutex cardinality_mu_;
  mutable core::RefCountPtr<DatasetBase> finalized_dataset_;
  //  The number of source datasets feeding into the dataset. A source dataset
  //  is a leaf in the subtree of dataset inputs.
  int64_t num_sources_ = -1;
  mutable int64_t cardinality_ TF_GUARDED_BY(cardinality_mu_) =
      kUnknownCardinality;
};

// Represents an iterator that is associated with a particular dataset.
class DatasetBaseIterator : public IteratorBase {
 public:
  struct BaseParams {
    // Owns one reference on the shared dataset object.
    const DatasetBase* dataset;

    // Identifies the sequence of iterators leading up to this iterator.
    const std::string prefix;
  };

  explicit DatasetBaseIterator(const BaseParams& params);

  ~DatasetBaseIterator() override;

  virtual const DatasetBase* dataset() const { return params_.dataset; }

  const DataTypeVector& output_dtypes() const override {
    return params_.dataset->output_dtypes();
  }

  const std::vector<PartialTensorShape>& output_shapes() const override {
    return params_.dataset->output_shapes();
  }

  const std::string& prefix() const override { return params_.prefix; }

  // Returns a name to be used for the TraceMe event.
  //
  // NOTE: TraceMe supports passing key-value pairs of "arguments" using the
  // following format "name#arg_1=value_,...,arg_n=value_n".
  std::string BuildTraceMeName();

  absl::Status GetNext(IteratorContext* ctx, std::vector<Tensor>* out_tensors,
                       bool* end_of_sequence) final;

  absl::Status GetNext(IteratorContext&& ctx, std::vector<Tensor>* out_tensors,
                       bool* end_of_sequence) {
    return GetNext(&ctx, out_tensors, end_of_sequence);
  }

  absl::Status Skip(IteratorContext* ctx, int num_to_skip,
                    bool* end_of_sequence, int* num_skipped) final;

  absl::Status Save(SerializationContext* ctx,
                    IteratorStateWriter* writer) final {
    VLOG(2) << "Attempting to save checkpoints on iterator (prefix: "
            << prefix() << ") from " << dataset()->DebugString();
    return IteratorBase::Save(ctx, writer);
  }

  // Returns a copy of the `status` where the error message is prepended with
  // dataset name and the iterator prefix.
  absl::Status AddErrorContext(const absl::Status& status) const {
    return absl::Status(
        status.code(),
        strings::StrCat("Error in user-defined function passed to ",
                        dataset()->metadata().name(),
                        " transformation with iterator: ", prefix(), ": ",
                        status.message()));
  }

 protected:
  absl::Status Restore(IteratorContext* ctx,
                       IteratorStateReader* reader) final {
    VLOG(2) << "Attempting to restore checkpoints on iterator (prefix: "
            << prefix() << ") from " << dataset()->DebugString();
    return IteratorBase::Restore(ctx, reader);
  }

  // Internal implementation of GetNext that is wrapped in tracing logic.
  //
  // See the docstring of `GetNext` method regaring the contract for
  // `out_tensors` and `end_of_sequence`. Implementations may assume that
  // `*out_tensors` is empty.
  virtual absl::Status GetNextInternal(IteratorContext* ctx,
                                       std::vector<Tensor>* out_tensors,
                                       bool* end_of_sequence) = 0;

  // Internal implementation of Skip that is wrapped in tracing logic
  virtual absl::Status SkipInternal(IteratorContext* ctx, int num_to_skip,
                                    bool* end_of_sequence, int* num_skipped);

  std::string full_name(const std::string& name) const {
    return FullName(params_.prefix, name);
  }

  // Returns a map of key-value pairs to included in the TraceMe string.
  virtual TraceMeMetadata GetTraceMeMetadata() const { return {}; }

  // By default we model iterators using an unknown node, which acts as
  // pass-through with respect to performance modeling.
  std::shared_ptr<model::Node> CreateNode(
      IteratorContext* ctx, model::Node::Args args) const override {
    return model::MakeUnknownNode(std::move(args));
  }

  // When modeling is enabled, this method disables autotuning for the given
  // iterator (and the transitive closure of its inputs).
  void DisableAutotune(IteratorContext* ctx, IteratorBase* iterator) {
    if (iterator->node_) {
      iterator->node_->set_autotune(false);
    }
  }

  // When modeling is enabled, this method enables autotuning for the given
  // iterator (and the transitive closure of its inputs).
  void EnableAutotune(IteratorContext* ctx, IteratorBase* iterator) {
    if (iterator->node_) {
      iterator->node_->set_autotune(true);
    }
  }

  // When modeling is enabled, this method records the fact that this iterator
  // has dequeued an element from an internal buffer.
  void RecordBufferDequeue(IteratorContext* ctx,
                           const std::vector<Tensor>& element) {
    if (collect_resource_usage(ctx)) {
      node_->record_buffer_event(-GetAllocatedBytes(element), -1);
      DCHECK_GE(node_->buffered_elements(), 0);
    }
  }

  // When modeling is enabled, this method records the fact that this iterator
  // has enqueued an element in an internal buffer.
  void RecordBufferEnqueue(IteratorContext* ctx,
                           const std::vector<Tensor>& element) {
    if (collect_resource_usage(ctx)) {
      node_->record_buffer_event(GetAllocatedBytes(element), 1);
    }
  }

  // When modeling is enabled, this method records the fact that this iterator
  // has produced an element and its size in bytes.
  void RecordElement(IteratorContext* ctx, std::vector<Tensor>* out_tensors) {
    if (collect_resource_usage(ctx)) {
      int64_t num_bytes = GetAllocatedBytes(*out_tensors);
      node_->record_element();
      node_->record_bytes_produced(num_bytes);
      if (node_->output()) {
        node_->output()->record_bytes_consumed(num_bytes);
      }
    }
  }

  // When modeling is enabled, this method records the fact that a thread of
  // this iterator has started work.
  void RecordStart(IteratorContext* ctx) {
    if (collect_resource_usage(ctx)) {
      int64_t now_nanos = EnvTime::NowNanos();
      node_->record_start(now_nanos);
    }
  }

  // When modeling is enabled, this method records the fact that a thread of
  // this iterator has stopped work.
  void RecordStop(IteratorContext* ctx) {
    if (collect_resource_usage(ctx)) {
      int64_t now_nanos = EnvTime::NowNanos();
      node_->record_stop(now_nanos);
    }
  }

  // Returns whether work is currently being recorded, i.e. whether we are
  // currently between a `RecordStart` and a `RecordStop`.
  bool IsRecording(IteratorContext* ctx) {
    return node_ && node_->is_recording();
  }

 private:
  bool collect_resource_usage(IteratorContext* ctx) {
    return ctx->model() && node_;
  }

  std::string traceme_metadata_;
  BaseParams params_;
};

// Represents an iterator that is associated with a particular dataset
// with a particular type.
template <class DatasetType>
class DatasetIterator : public DatasetBaseIterator {
 public:
  struct Params {
    // Borrowed pointer to the dataset.
    const DatasetType* dataset;

    // Identifies the sequence of iterators leading up to this iterator.
    const std::string prefix;
  };

  explicit DatasetIterator(const Params& params)
      : DatasetBaseIterator({params.dataset, params.prefix}),
        typed_dataset_(params.dataset) {}

  // The dataset from which this iterator was created.
  const DatasetType* dataset() const final { return typed_dataset_; }

 private:
  const DatasetType* const typed_dataset_;  // Not owned.
};

template <typename T>
absl::Status ParseScalarArgument(OpKernelContext* ctx,
                                 const absl::string_view& argument_name,
                                 T* output) {
  const Tensor* argument_t;
  TF_RETURN_IF_ERROR(ctx->input(argument_name, &argument_t));
  if (!TensorShapeUtils::IsScalar(argument_t->shape())) {
    return absl::InvalidArgumentError(
        absl::StrCat(argument_name, " must be a scalar"));
  }
  *output = argument_t->scalar<T>()();
  return absl::OkStatus();
}

template <typename T>
absl::Status ParseVectorArgument(OpKernelContext* ctx,
                                 const absl::string_view& argument_name,
                                 std::vector<T>* output) {
  const Tensor* argument_t;
  TF_RETURN_IF_ERROR(ctx->input(argument_name, &argument_t));
  if (!TensorShapeUtils::IsVector(argument_t->shape())) {
    return absl::InvalidArgumentError(
        absl::StrCat(argument_name, " must be a vector"));
  }
  int size = argument_t->vec<T>().size();
  output->reserve(size);
  for (int i = 0; i < size; ++i) {
    output->push_back(argument_t->vec<T>()(i));
  }
  return absl::OkStatus();
}

// Encapsulates the work required to plug a DatasetBase into the core TensorFlow
// graph execution engine.
class DatasetOpKernel : public OpKernel {
 public:
  explicit DatasetOpKernel(OpKernelConstruction* ctx) : OpKernel(ctx) {
    if (ctx->HasAttr(kMetadata)) {
      std::string serialized_metadata;
      OP_REQUIRES_OK(ctx, ctx->GetAttr(kMetadata, &serialized_metadata));
      OP_REQUIRES(ctx, metadata_.ParseFromString(serialized_metadata),
                  absl::InvalidArgumentError(
                      "Could not parse the 'metadata' attribute."));
    }
  }

  void Compute(OpKernelContext* ctx) final;

  // Checks whether the given op is a tf.data operation.
  //
  // NOTE: The check uses a heuristic and can produce both false positives and
  // false negatives. In particular, tf.data operations are expected to use
  // names that end with "Dataset" or "DatasetV[0-9]+".
  static bool IsDatasetOp(const OpDef& op_def);

  std::string TraceString(const OpKernelContext& ctx,
                          bool verbose) const override;

 protected:
  // Subclasses should implement this method. It will be called during Compute
  // execution.
  virtual void MakeDataset(OpKernelContext* ctx, DatasetBase** output) = 0;

 private:
  Metadata metadata_;
};

// Encapsulates the work required to plug unary Datasets into the core
// TensorFlow graph execution engine.
class UnaryDatasetOpKernel : public DatasetOpKernel {
 public:
  explicit UnaryDatasetOpKernel(OpKernelConstruction* ctx)
      : DatasetOpKernel(ctx) {}

 protected:
  void MakeDataset(OpKernelContext* ctx, DatasetBase** output) final;
  virtual void MakeDataset(OpKernelContext* ctx, DatasetBase* input,
                           DatasetBase** output) = 0;
};

// Encapsulates the work required to plug binary Datasets into the core
// TensorFlow graph execution engine.
class BinaryDatasetOpKernel : public DatasetOpKernel {
 public:
  explicit BinaryDatasetOpKernel(OpKernelConstruction* ctx)
      : DatasetOpKernel(ctx) {}

 protected:
  void MakeDataset(OpKernelContext* ctx, DatasetBase** output) final;
  virtual void MakeDataset(OpKernelContext* ctx, DatasetBase* input,
                           DatasetBase* another_input,
                           DatasetBase** output) = 0;
};

// A simple background worker that executes closures asynchronously and without
// blocking.
//
// A `BackgroundWorker` is used to offload blocking work from an `AsyncOpKernel`
// to avoid blocking an executor thread that may be required by the blocking
// work.
//
// NOTE(mrry): We do not use a regular `tensorflow::thread::ThreadPool` for this
// purpose because its current implementation (in Eigen) uses a finite-length
// queue and will block the caller when full. This can lead to deadlock under
// heavy load. Since the number of concurrent work items in each user of a
// `BackgroundWorker` is at most one per op invocation, the dynamic allocation
// overhead is tolerable.
class BackgroundWorker {
 public:
  BackgroundWorker(Env* env, const char* name);

  ~BackgroundWorker();

  void Schedule(std::function<void()> work_item);

 private:
  void WorkerLoop();

  Env* const env_;
  const char* const name_;

  std::unique_ptr<Thread> thread_;
  mutex mu_;
  condition_variable cond_var_;
  bool cancelled_ TF_GUARDED_BY(mu_) = false;
  std::deque<std::function<void()>> work_queue_ TF_GUARDED_BY(mu_);
};

}  // namespace data
}  // namespace tensorflow

// ==================================================================
// Implementation: dataset.cc
// ==================================================================

// On Windows, disable some macros that would break compile
#if defined(PLATFORM_WINDOWS)
#undef GetMessage
#endif

namespace tensorflow {
namespace data {
namespace {

static mutex* get_dataset_op_registry_lock() {
  static mutex dataset_op_registry_lock(LINKER_INITIALIZED);
  return &dataset_op_registry_lock;
}

static std::unordered_set<std::string>* get_dataset_op_registry() {
  static std::unordered_set<std::string>* names =
      new std::unordered_set<std::string>;
  return names;
}

std::string UniqueNodeName(const std::string& base) {
  static std::atomic<int64_t> counter(0);
  return absl::StrCat(base, "/", counter.fetch_add(1));
}

// A wrapper class for storing a `DatasetBase` instance in a DT_VARIANT tensor.
// Objects of the wrapper class own a reference on an instance of `DatasetBase`,
// and the wrapper's copy constructor and destructor take care of managing the
// reference count.
//
// NOTE(mrry): This is not a feature-complete implementation of the DT_VARIANT
// specification. In particular, we cannot currently serialize an arbitrary
// `DatasetBase` object, so the `Encode()` and `Decode()` methods are not
// implemented.
class DatasetVariantWrapper {
 public:
  DatasetVariantWrapper() : dataset_(nullptr) {}

  // Transfers ownership of `dataset` to `*this`.
  explicit DatasetVariantWrapper(DatasetBase* dataset) : dataset_(dataset) {}

  DatasetVariantWrapper(const DatasetVariantWrapper& other)
      : dataset_(other.dataset_) {
    if (dataset_) dataset_->Ref();
  }

  DatasetVariantWrapper& operator=(DatasetVariantWrapper&& other) {
    if (&other == this) return *this;
    std::swap(dataset_, other.dataset_);
    return *this;
  }

  DatasetVariantWrapper& operator=(const DatasetVariantWrapper& other) = delete;

  ~DatasetVariantWrapper() {
    if (dataset_) dataset_->Unref();
  }

  DatasetBase* get() const { return dataset_; }

  std::string TypeName() const { return "tensorflow::DatasetVariantWrapper"; }
  std::string DebugString() const {
    if (dataset_) {
      return dataset_->DebugString();
    } else {
      return "<Uninitialized DatasetVariantWrapper>";
    }
  }
  void Encode(VariantTensorData* data) const {
    LOG(ERROR) << "The Encode() method is not implemented for "
                  "DatasetVariantWrapper objects.";
  }
  bool Decode(const VariantTensorData& data) {
    LOG(ERROR) << "The Decode() method is not implemented for "
                  "DatasetVariantWrapper objects.";
    return false;
  }

 private:
  DatasetBase* dataset_;  // Owns one reference.
};

const char kWrappedDatasetVariantTypeName[] =
    "tensorflow::data::WrappedDatasetVariant";

class WrappedDatasetVariantWrapper {
 public:
  WrappedDatasetVariantWrapper() = default;

  explicit WrappedDatasetVariantWrapper(const Tensor& ds_tensor)
      : ds_tensor_(ds_tensor) {}

  Tensor get() const { return ds_tensor_; }

  std::string TypeName() const {
    return "tensorflow::WrappedDatasetVariantWrapper";
  }

  std::string DebugString() const {
    return "tensorflow::WrappedDatasetVariantWrapper::DebugString";
  }

  void Encode(VariantTensorData* data) const {
    *(data->add_tensors()) = ds_tensor_;
  }

  bool Decode(const VariantTensorData& data) {
    ds_tensor_ = data.tensors(0);
    return true;
  }

 private:
  Tensor ds_tensor_;
};

class WrapDatasetVariantOp : public OpKernel {
 public:
  explicit WrapDatasetVariantOp(OpKernelConstruction* ctx) : OpKernel(ctx) {}

  void Compute(OpKernelContext* ctx) override {
    const Tensor& tensor = ctx->input(0);
    OP_REQUIRES(ctx,
                tensor.dtype() == DT_VARIANT &&
                    TensorShapeUtils::IsScalar(tensor.shape()),
                absl::InvalidArgumentError(
                    "Dataset tensor must be a scalar of dtype DT_VARIANT."));
    DatasetBase* unused;
    OP_REQUIRES_OK(ctx, GetDatasetFromVariantTensor(tensor, &unused));
    Tensor* output = nullptr;
    OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}), &output));
    output->scalar<Variant>()() = WrappedDatasetVariantWrapper(tensor);
  }
};

REGISTER_KERNEL_BUILDER(Name("WrapDatasetVariant").Device(DEVICE_CPU),
                        WrapDatasetVariantOp);
REGISTER_KERNEL_BUILDER(Name("WrapDatasetVariant")
                            .HostMemory("input_handle")
                            .HostMemory("output_handle")
                            .Device(DEVICE_GPU),
                        WrapDatasetVariantOp);

class UnwrapDatasetVariantOp : public OpKernel {
 public:
  explicit UnwrapDatasetVariantOp(OpKernelConstruction* ctx) : OpKernel(ctx) {}

  void Compute(OpKernelContext* ctx) override {
    const Tensor& tensor = ctx->input(0);
    OP_REQUIRES(ctx,
                tensor.dtype() == DT_VARIANT &&
                    TensorShapeUtils::IsScalar(tensor.shape()),
                absl::InvalidArgumentError(
                    "Dataset tensor must be a scalar of dtype DT_VARIANT."));
    Variant variant = tensor.scalar<Variant>()();
    const WrappedDatasetVariantWrapper* wrapper =
        variant.get<WrappedDatasetVariantWrapper>();
    OP_REQUIRES(ctx, wrapper != nullptr,
                absl::InvalidArgumentError(
                    "Tensor must be a WrappedDataset variant object."));
    Tensor ds_tensor = wrapper->get();
    OP_REQUIRES_OK(ctx, ctx->set_output("output_handle", ds_tensor));
  }
};

REGISTER_KERNEL_BUILDER(Name("UnwrapDatasetVariant").Device(DEVICE_CPU),
                        UnwrapDatasetVariantOp);
REGISTER_KERNEL_BUILDER(Name("UnwrapDatasetVariant")
                            .HostMemory("input_handle")
                            .HostMemory("output_handle")
                            .Device(DEVICE_GPU),
                        UnwrapDatasetVariantOp);

static absl::Status WrappedDatasetVariantDeviceCopy(
    const WrappedDatasetVariantWrapper& from, WrappedDatasetVariantWrapper* to,
    const UnaryVariantOpRegistry::AsyncTensorDeviceCopyFn& copy) {
  *to = WrappedDatasetVariantWrapper(from);
  return absl::OkStatus();
}

#define REGISTER_OPTIONAL_COPY(DIRECTION)               \
  INTERNAL_REGISTER_UNARY_VARIANT_DEVICE_COPY_FUNCTION( \
      WrappedDatasetVariantWrapper, DIRECTION,          \
      WrappedDatasetVariantDeviceCopy)

REGISTER_OPTIONAL_COPY(VariantDeviceCopyDirection::HOST_TO_DEVICE);
REGISTER_OPTIONAL_COPY(VariantDeviceCopyDirection::DEVICE_TO_HOST);
REGISTER_OPTIONAL_COPY(VariantDeviceCopyDirection::DEVICE_TO_DEVICE);

REGISTER_UNARY_VARIANT_DECODE_FUNCTION(WrappedDatasetVariantWrapper,
                                       kWrappedDatasetVariantTypeName);

}  // namespace

absl::Status GraphDefBuilderWrapper::AddDataset(
    const DatasetBase* dataset, const std::vector<Node*>& inputs,
    Node** output) {
  return AddDataset(dataset, inputs, {}, output);
}

absl::Status GraphDefBuilderWrapper::AddDataset(
    const DatasetBase* dataset, const std::vector<Node*>& inputs,
    const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
    Node** output) {
  std::vector<std::pair<size_t, Node*>> enumerated_inputs(inputs.size());
  for (size_t i = 0; i < inputs.size(); i++) {
    enumerated_inputs[i] = std::make_pair(i, inputs[i]);
  }
  return AddDataset(dataset, enumerated_inputs, {}, attrs, output);
}

absl::Status GraphDefBuilderWrapper::AddDataset(
    const DatasetBase* dataset,
    const std::vector<std::pair<size_t, Node*>>& inputs,
    const std::vector<std::pair<size_t, absl::Span<Node* const>>>& list_inputs,
    const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
    Node** output) {
  return AddDataset(dataset, inputs, list_inputs, attrs,
                    /*use_dataset_name=*/false, output);
}

absl::Status GraphDefBuilderWrapper::AddDataset(
    const DatasetBase* dataset,
    const std::vector<std::pair<size_t, Node*>>& inputs,
    const std::vector<std::pair<size_t, absl::Span<Node* const>>>& list_inputs,
    const std::vector<std::pair<absl::string_view, AttrValue>>& attrs,
    bool use_dataset_name, Node** output) {
  auto& type_string = dataset->type_string();
  auto opts = std::make_unique<GraphDefBuilder::Options>(b_->opts());
  // TODO(srbs|mrry): Not all datasets have output_types and output_shapes
  // attributes defined. It will be nice to have a consistent pattern.
  bool has_output_types_attr = HasAttr(type_string, "output_types");
  bool has_output_shapes_attr = HasAttr(type_string, "output_shapes");
  if (has_output_shapes_attr) {
    opts = std::make_unique<GraphDefBuilder::Options>(
        opts->WithAttr("output_shapes", dataset->output_shapes()));
  }
  if (has_output_types_attr) {
    opts = std::make_unique<GraphDefBuilder::Options>(
        opts->WithAttr("output_types", dataset->output_dtypes()));
  }
  bool has_metadata_attr = HasAttr(type_string, "metadata");
  if (has_metadata_attr) {
    std::string serialized_metadata;
    dataset->metadata().SerializeToString(&serialized_metadata);
    opts = std::make_unique<GraphDefBuilder::Options>(
        opts->WithAttr("metadata", serialized_metadata));
  }
  for (const auto& attr : attrs) {
    opts = std::make_unique<GraphDefBuilder::Options>(
        opts->WithAttr(attr.first, attr.second));
  }
  if (opts->HaveError()) {
    return absl::InternalError(
        absl::StrCat("AddDataset: Failed to build Options with error ",
                     opts->StatusToString()));
  }
  NodeBuilder node_builder(
      use_dataset_name ? dataset->node_name() : opts->GetNameForOp(type_string),
      type_string, opts->op_registry());
  {
    size_t total_size = inputs.size() + list_inputs.size();
    auto inputs_iter = inputs.begin();
    auto list_inputs_iter = list_inputs.begin();
    for (int i = 0; i < total_size; i++) {
      if (inputs_iter != inputs.end() && inputs_iter->first == i) {
        node_builder.Input(NodeBuilder::NodeOut(inputs_iter->second));
        inputs_iter++;
      } else if (list_inputs_iter != list_inputs.end() &&
                 list_inputs_iter->first == i) {
        std::vector<NodeBuilder::NodeOut> nodeout_inputs;
        nodeout_inputs.reserve(list_inputs_iter->second.size());
        for (Node* n : list_inputs_iter->second) {
          nodeout_inputs.emplace_back(n);
        }
        node_builder.Input(nodeout_inputs);
        list_inputs_iter++;
      } else {
        return absl::InvalidArgumentError(
            absl::StrCat("No input found for index ", i));
      }
    }
  }
  *output = opts->FinalizeBuilder(&node_builder);
  if (*output == nullptr) {
    return absl::InternalError(absl::StrCat("AddDataset: Failed to build ",
                                            type_string, " op with error ",
                                            opts->StatusToString()));
  }
  return absl::OkStatus();
}

absl::Status GraphDefBuilderWrapper::AddFunction(
    SerializationContext* ctx, const std::string& function_name,
    const FunctionLibraryDefinition& lib_def) {
  if (b_->HasFunction(function_name)) {
    VLOG(1) << "Function with name " << function_name << "already exists in"
            << " the graph. It will not be added again.";
    return absl::OkStatus();
  }
  const FunctionDef* f_def = lib_def.Find(function_name);
  if (f_def == nullptr) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Unable to find FunctionDef for ", function_name, " in the registry."));
  }
  FunctionDefLibrary def;
  *def.add_function() = *f_def;
  const std::string gradient_func = lib_def.FindGradient(function_name);
  if (!gradient_func.empty()) {
    GradientDef* g_def = def.add_gradient();
    g_def->set_function_name(function_name);
    g_def->set_gradient_func(gradient_func);
  }
  TF_RETURN_IF_ERROR(b_->AddFunctionLibrary(def));

  // Recursively add functions in inputs of function_name.
  for (const NodeDef& node_def : f_def->node_def()) {
    const OpRegistrationData* op_reg_data = nullptr;
    TF_RETURN_IF_ERROR(lib_def.LookUp(node_def.op(), &op_reg_data));
    if (op_reg_data->is_function_op) {
      TF_RETURN_IF_ERROR(AddFunction(ctx, op_reg_data->op_def.name(), lib_def));
    }
    // Recursively add functions in attrs of this NodeDef.
    for (const auto& pair : node_def.attr()) {
      TF_RETURN_IF_ERROR(AddAttrFunctions(ctx, pair.second, lib_def));
    }
  }

  // Recursively add functions in attrs of function_name.
  for (auto iter = f_def->attr().begin(); iter != f_def->attr().end(); iter++) {
    TF_RETURN_IF_ERROR(AddAttrFunctions(ctx, iter->second, lib_def));
  }
  return absl::OkStatus();
}

void GraphDefBuilderWrapper::AddPlaceholderInternal(const Tensor& val,
                                                    Node** output) {
  *output = ops::SourceOp(
      "Placeholder",
      b_->opts().WithAttr("dtype", val.dtype()).WithAttr("shape", val.shape()));
}

void GraphDefBuilderWrapper::AddTensorInternal(const Tensor& val,
                                               Node** output) {
  *output = ops::SourceOp(
      "Const",
      b_->opts().WithAttr("dtype", val.dtype()).WithAttr("value", val));
}

bool GraphDefBuilderWrapper::HasAttr(const std::string& name,
                                     const std::string& attr_name) const {
  const OpDef* op_def = nullptr;
  absl::Status s = b_->opts().op_registry()->LookUpOpDef(name, &op_def);
  if (!s.ok() || op_def == nullptr) {
    return false;
  }
  return HasAttr(op_def, attr_name);
}

int32_t GetRunnerThreadpoolSizeFromOpKernelContext(OpKernelContext* ctx) {
  thread::ThreadPool* thread_pool =
      ctx->device()->tensorflow_device_thread_pool();
  if (thread_pool) {
    return thread_pool->NumThreads();
  } else {
    static const int32_t kDefaultRunnerThreadpoolSize = port::MaxParallelism();
    return kDefaultRunnerThreadpoolSize;
  }
}

int64_t MemoryCheckpoint::IdRegistry::Add(const std::string& prefix,
                                          const std::string& key) {
  mutex_lock l(mu_);
  auto pair = std::make_pair(prefix, key);
  if (string_to_int_.contains(pair)) {
    return string_to_int_[pair];
  }
  int64_t id = next_id_++;
  int_to_string_[id] = pair;
  string_to_int_[pair] = id;
  return id;
}

std::vector<int64_t> MemoryCheckpoint::IdRegistry::GetMatchingIds(
    const std::string& prefix_to_match) {
  mutex_lock l(mu_);
  std::vector<int64_t> ids;
  for (const auto& [pair, id] : string_to_int_) {
    auto [prefix, key] = pair;
    if (prefix.compare(0, prefix_to_match.length(), prefix_to_match) == 0) {
      ids.push_back(id);
    }
  }
  return ids;
}

std::pair<std::string, std::string> MemoryCheckpoint::IdRegistry::Get(
    int64_t id) {
  mutex_lock l(mu_);
  auto result = int_to_string_.find(id);
  DCHECK(result != int_to_string_.end())
      << "Failed find id " << id << " in IdRegistry. "
      << "Max id is: " << next_id_ - 1;
  return result->second;
}

void MemoryCheckpoint::IdRegistry::RemoveIds(const std::vector<int64_t>& ids) {
  mutex_lock l(mu_);
  for (const auto& id : ids) {
    string_to_int_.erase(int_to_string_[id]);
    int_to_string_.erase(id);
  }
}

std::string MemoryCheckpoint::DebugString() const {
  std::string result = absl::StrCat("status=", status_.ToString(),
                                    ", "
                                    "root=",
                                    (is_root_ ? "true" : "false"), "\n");
  absl::StrAppend(&result, "number of integers: ", int_values_.size(), "\n");
  for (const auto& [k, v] : int_values_) {
    absl::StrAppend(&result, "  ", id_registry_->Get(k).first, ":",
                    id_registry_->Get(k).second, ": ", v, "\n");
  }
  absl::StrAppend(&result, "number of strings: ", str_values_.size(), "\n");
  for (const auto& [k, v] : str_values_) {
    absl::StrAppend(&result, "  ", id_registry_->Get(k).first, ":",
                    id_registry_->Get(k).second, ": ", v, "\n");
  }
  absl::StrAppend(&result, "number of tensors: ", tensor_values_.size(), "\n");

  absl::StrAppend(
      &result, "number of expired prefixes: ", expired_prefixes_.size(), "\n");
  return result;
}

void MemoryCheckpoint::Merge(MemoryCheckpoint* other) {
  if (!status_.ok()) {
    return;
  }

  if (!other->status_.ok()) {
    status_ = other->status_;
    int_values_.clear();
    str_values_.clear();
    tensor_values_.clear();
  }

  for (const auto& [k, v] : other->int_values_) {
    int_values_[k] = v;
  }
  for (const auto& [k, v] : other->str_values_) {
    str_values_[k] = v;
  }
  for (const auto& [k, v] : other->tensor_values_) {
    tensor_values_[k] = v;
  }

  // Get the expired prefixes from `other`. Since the info only needs to be
  // propagated once downstream, we also clean the `expired_prefixes_` of
  // `other` here.
  for (const auto& prefix : other->expired_prefixes_) {
    Purge(prefix);
  }

  other->expired_prefixes_.clear();
  VLOG(5) << "MemoryCheckpoint::Merge " << DebugString();
}

void MemoryCheckpoint::Purge(const std::string& prefix) {
  std::vector<int64_t> ids = id_registry_->GetMatchingIds(prefix);
  for (const auto& id : ids) {
    int_values_.erase(id);
    str_values_.erase(id);
    tensor_values_.erase(id);
  }
  if (!is_root_) {
    expired_prefixes_.insert(prefix);
  } else {
    // We no longer need the mapping after change has been propagated all the
    // way to root.
    id_registry_->RemoveIds(ids);
  }
}

absl::Status MemoryCheckpoint::Save(IteratorStateWriter* writer) const {
  for (const auto& [id, value] : int_values_) {
    auto [prefix, key] = id_registry_->Get(id);
    TF_RETURN_IF_ERROR(writer->WriteScalar(prefix, key, value));
  }
  for (const auto& [id, value] : str_values_) {
    auto [prefix, key] = id_registry_->Get(id);
    TF_RETURN_IF_ERROR(writer->WriteScalar(prefix, key, value));
  }
  for (const auto& [id, value] : tensor_values_) {
    auto [prefix, key] = id_registry_->Get(id);
    TF_RETURN_IF_ERROR(writer->WriteTensor(prefix, key, value));
  }
  return absl::OkStatus();
}

absl::Status IteratorBase::InitializeBase(IteratorContext* ctx,
                                          const IteratorBase* parent) {
  parent_ = parent;
  id_ = Hash64CombineUnordered(Hash64(prefix()),
                               reinterpret_cast<uint64_t>(this));
  if (parent_) {
    parent_id_ = Hash64CombineUnordered(Hash64(parent_->prefix()),
                                        reinterpret_cast<uint64_t>(parent_));
    // This block of code is executed only when `parent_` is not a `nullptr`
    // because we do not create a `Node` in the `Model` for `RootDataset`.
    if (const auto& model = ctx->model()) {
      auto factory = [ctx, this](model::Node::Args args) {
        return CreateNode(ctx, std::move(args));
      };
      model->AddNode(std::move(factory), prefix(), parent->model_node(),
                     &node_);
      cleanup_fns_.push_back([this, model]() { model->RemoveNode(node_); });
    }
  }
  return absl::OkStatus();
}

absl::Status GetCompressedElementFromVariantTensor(
    const Tensor& tensor, const CompressedElement** out_compressed_element) {
  if (!(tensor.dtype() == DT_VARIANT &&
        TensorShapeUtils::IsScalar(tensor.shape()))) {
    return absl::InvalidArgumentError(
        "`CompressedElement` tensor must be a scalar of dtype `DT_VARIANT`.");
  }
  const Variant& variant = tensor.scalar<Variant>()();
  const CompressedElement* compressed_element =
      variant.get<CompressedElement>();
  if (compressed_element == nullptr) {
    return absl::InvalidArgumentError(
        "Tensor must be a `CompressedElement` object.");
  }
  *out_compressed_element = compressed_element;
  return absl::OkStatus();
}

int64_t GetAllocatedBytes(const std::vector<Tensor>& element) {
  int64_t allocated_bytes = 0;
  for (auto& tensor : element) {
    if (tensor.dtype() == DT_VARIANT) {
      // Special case certain variants where AllocatedBytes() doesn't give an
      // accurate byte count.
      DatasetBase* dataset;
      if (GetDatasetFromVariantTensor(tensor, &dataset).ok()) {
        allocated_bytes += dataset->AllocatedBytes();
        continue;
      }
      const CompressedElement* compressed_element;
      if (GetCompressedElementFromVariantTensor(tensor, &compressed_element)
              .ok()) {
        allocated_bytes += compressed_element->ByteSizeLong();
        continue;
      }
    }
    allocated_bytes += tensor.AllocatedBytes();
  }
  return allocated_bytes;
}

int64_t GetTotalBytes(const std::vector<Tensor>& element) {
  int64_t total_bytes = 0;
  for (auto& tensor : element) {
    if (tensor.dtype() == DT_VARIANT) {
      // Special case certain variants where TotalBytes() doesn't give an
      // accurate byte count.
      DatasetBase* dataset;
      if (GetDatasetFromVariantTensor(tensor, &dataset).ok()) {
        total_bytes += dataset->TotalBytes();
        continue;
      }
      const CompressedElement* compressed_element;
      if (GetCompressedElementFromVariantTensor(tensor, &compressed_element)
              .ok()) {
        total_bytes += compressed_element->ByteSizeLong();
        continue;
      }
    }
    total_bytes += tensor.TotalBytes();
  }
  return total_bytes;
}

std::string FullName(const std::string& prefix, const std::string& name) {
  if (absl::StrContains(name, kColon)) {
    LOG(ERROR) << name << " should not contain " << kColon;
  }

  return strings::StrCat(kFullNameRandomHex, kPipe, prefix, kColon, name);
}

absl::Status ExtractIteratorPrefix(absl::string_view key, std::string* prefix) {
  if (!absl::StartsWith(key, data::kFullNameRandomHex)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Key: ", key, " was not generated using full_name."));
  }
  std::vector<std::string> split_keys = str_util::Split(key, data::kPipe);
  if (split_keys.size() != 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("Key: ", key, " was not generated using full_name."));
  }
  std::string real_key = split_keys[1];
  const int pos = real_key.rfind(kColon);
  *prefix = real_key.substr(0, pos);
  return absl::OkStatus();
}

absl::Status GetDatasetFromVariantTensor(const Tensor& tensor,
                                         DatasetBase** out_dataset) {
  if (!(tensor.dtype() == DT_VARIANT &&
        TensorShapeUtils::IsScalar(tensor.shape()))) {
    return absl::InvalidArgumentError(
        "Dataset tensor must be a scalar of dtype DT_VARIANT.");
  }
  const Variant& variant = tensor.scalar<Variant>()();
  const DatasetVariantWrapper* wrapper = variant.get<DatasetVariantWrapper>();
  if (wrapper == nullptr) {
    return absl::InvalidArgumentError("Tensor must be a Dataset object.");
  }
  *out_dataset = wrapper->get();
  if (*out_dataset == nullptr) {
    return absl::InternalError("Read uninitialized Dataset variant.");
  }
  return absl::OkStatus();
}

absl::Status StoreDatasetInVariantTensor(DatasetBase* dataset, Tensor* tensor) {
  if (!(tensor->dtype() == DT_VARIANT &&
        TensorShapeUtils::IsScalar(tensor->shape()))) {
    return absl::InvalidArgumentError(
        "Dataset tensor must be a scalar of dtype DT_VARIANT.");
  }
  tensor->scalar<Variant>()() = DatasetVariantWrapper(dataset);
  return absl::OkStatus();
}

namespace internal {

#define WARN_PROTO_FIELD_CONFLICT(reflection, field, field_type, src, dst)     \
  {                                                                            \
    auto source_value = reflection->Get##field_type(src, field);               \
    auto destination_value = reflection->Get##field_type(*dst, field);         \
    if (source_value != destination_value) {                                   \
      LOG(WARNING) << "Changing the value of option field " << field->name()   \
                   << " from " << destination_value << " to " << source_value; \
    }                                                                          \
  }

#define WARN_PROTO_ENUM_FIELD_CONFLICT(reflection, field, src, dst) \
  {                                                                 \
    auto source_value = reflection->GetEnum(src, field);            \
    auto destination_value = reflection->GetEnum(*dst, field);      \
    if (source_value != destination_value) {                        \
      LOG(WARNING) << "Changing the value of option enum field "    \
                   << field->name() << " from "                     \
                   << destination_value->full_name() << " to "      \
                   << source_value->full_name();                    \
    }                                                               \
  }

void WarnProtoConflicts(const protobuf::Message& src, protobuf::Message* dst) {
  std::vector<const protobuf::FieldDescriptor*> set_src;
  std::vector<const protobuf::FieldDescriptor*> set_dst;
  const protobuf::Reflection* reflection = src.GetReflection();
  reflection->ListFields(src, &set_src);
  reflection->ListFields(*dst, &set_dst);
  std::sort(set_src.begin(), set_src.end());
  std::sort(set_dst.begin(), set_dst.end());

  std::vector<const protobuf::FieldDescriptor*> in_both;
  std::set_intersection(set_src.begin(), set_src.end(), set_dst.begin(),
                        set_dst.end(), std::back_inserter(in_both));

  for (auto field : in_both) {
    // Used for Job Instrumentation, users should not be warned.
    if (field->name() == "framework_type") {
      continue;
    }
    if (field->type() == protobuf::FieldDescriptor::TYPE_MESSAGE) {
      WarnProtoConflicts(reflection->GetMessage(src, field),
                         reflection->MutableMessage(dst, field));
    } else {
      switch (field->cpp_type()) {
        case protobuf::FieldDescriptor::CPPTYPE_INT32:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, Int32, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_INT64:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, Int64, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_UINT32:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, UInt32, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_UINT64:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, UInt64, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, Double, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_FLOAT:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, Float, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_BOOL:
          WARN_PROTO_FIELD_CONFLICT(reflection, field, Bool, src, dst);
          break;
        case protobuf::FieldDescriptor::CPPTYPE_ENUM:
          WARN_PROTO_ENUM_FIELD_CONFLICT(reflection, field, src, dst);
          break;
        default: {
          LOG(ERROR) << "Unrecognized proto type for field "
                     << field->full_name();
        }
      }
    }
  }
}

#undef WARN_PROTO_ENUM_FIELD_CONFLICT
#undef WARN_PROTO_FIELD_CONFLICT

void MergeOptions(const protobuf::Message& source,
                  protobuf::Message* destination) {
  WarnProtoConflicts(source, destination);
  destination->MergeFrom(source);
}

void MergeOptions(const protobuf::MessageLite& source,
                  protobuf::MessageLite* destination) {
  destination->CheckTypeAndMergeFrom(source);
}

}  // namespace internal

void DatasetBase::Initialize(const Metadata& metadata) {
  absl::Status s = ComputeNumSources();
  if (!s.ok()) {
    LOG_EVERY_N_SEC(ERROR, 10) << s;
  }
  s = MergeOptionsFromInputs();
  if (!s.ok()) {
    LOG_EVERY_N_SEC(ERROR, 10) << s;
  }
  metadata_ = metadata;
  if (metadata_.name() == "") {
    static std::atomic<int64_t> id_counter(0);
    *metadata_.mutable_name() =
        absl::StrCat(type_string(), ":", id_counter.fetch_add(1));
  }
}

absl::Status DatasetBase::ComputeNumSources() {
  std::vector<const DatasetBase*> inputs;
  absl::Status s = InputDatasets(&inputs);
  if (absl::IsUnimplemented(s)) {
    return s;
  }
  if (num_sources_ >= 0) {
    // Already computed.
    return absl::OkStatus();
  }
  num_sources_ = 0;
  if (inputs.empty()) {
    num_sources_ = 1;
    return absl::OkStatus();
  }
  for (const auto& input : inputs) {
    if (input->num_sources() < 0) {
      return absl::FailedPreconditionError(absl::StrCat(
          "Cannot compute input sources for dataset of type ", type_string(),
          ", because sources could not be computed for input dataset of type ",
          input->type_string()));
    }
    num_sources_ += input->num_sources();
  }
  return absl::OkStatus();
}

absl::Status DatasetBase::CheckRandomAccessCompatible(
    const int64_t index) const {
  CardinalityOptions options;
  options.set_compute_level(CardinalityOptions::CARDINALITY_COMPUTE_MODERATE);
  int64_t cardinality = Cardinality(options);
  if (cardinality == kInfiniteCardinality ||
      cardinality == kUnknownCardinality) {
    return absl::FailedPreconditionError(absl::StrCat(
        "Dataset of type ", this->DebugString(), " has ",
        cardinality == kInfiniteCardinality ? "infinite" : "unknown",
        " cardinality, which does not support random access."));
  }
  if (index < 0 || index >= cardinality) {
    return absl::OutOfRangeError(
        absl::StrCat("Index out of range [0, ", cardinality, "):", index));
  }
  return absl::OkStatus();
}

absl::Status DatasetBase::Get(OpKernelContext* ctx, int64_t index,
                              std::vector<Tensor>* out_tensors) const {
  return absl::UnimplementedError(absl::StrCat(
      "Random access is not implemented for dataset ", DebugString()));
}

absl::Status DatasetBase::Get(AnyContext ctx, int64_t index,
                              std::vector<Tensor>* out_tensors) const {
  return absl::UnimplementedError(absl::StrCat(
      "Random access is not implemented for dataset ", DebugString()));
}

absl::StatusOr<DatasetBase*> DatasetBase::Finalize(
    OpKernelContext* ctx,
    std::function<absl::StatusOr<core::RefCountPtr<DatasetBase>>()>
        make_finalized_dataset) const {
  mutex_lock l(mu_);
  if (!finalized_dataset_) {
    TF_ASSIGN_OR_RETURN(finalized_dataset_, make_finalized_dataset());
  }
  return finalized_dataset_.get();
}

absl::Status DatasetBase::MergeOptionsFromInputs() {
  std::vector<const DatasetBase*> inputs;
  absl::Status s = InputDatasets(&inputs);
  if (absl::IsUnimplemented(s)) {
    return s;
  }
  if (inputs.empty()) {
    return absl::OkStatus();
  }
  // Merge options from inputs sequentially before merging options from dataset.
  // Since the last options merged takes precedence, the options that may be set
  // for the current dataset through OptionsDataset takes precedence over those
  // set on the input datasets.
  Options merged_options = inputs[0]->options_;
  for (int i = 1; i < inputs.size(); ++i) {
    internal::MergeOptions(inputs[i]->options_, &merged_options);
  }
  internal::MergeOptions(options_, &merged_options);
  options_ = merged_options;
  return absl::OkStatus();
}

absl::Status DatasetBase::MakeIterator(
    IteratorContext* ctx, const IteratorBase* parent,
    const std::string& output_prefix,
    std::unique_ptr<IteratorBase>* iterator) const {
  if (type_string() == "OptionsDataset" || type_string() == "FinalizeDataset") {
    std::vector<const DatasetBase*> inputs;
    absl::Status s = InputDatasets(&inputs);
    return inputs[0]->MakeIterator(ctx, parent, output_prefix, iterator);
  }
  tsl::profiler::TraceMe traceme(
      [&] {
        return tsl::profiler::TraceMeEncode(
            absl::StrCat("MakeIterator::", type_string()), {});
      },
      tsl::profiler::TraceMeLevel::kInfo);
  *iterator = MakeIteratorInternal(output_prefix);
  absl::Status s = (*iterator)->InitializeBase(ctx, parent);
  if (s.ok()) {
    s.Update((*iterator)->Initialize(ctx));
    ctx->SaveCheckpoint(iterator->get());
  }
  if (!s.ok()) {
    // Reset the iterator to avoid returning an uninitialized iterator.
    iterator->reset();
  }
  return s;
}

absl::Status DatasetBase::MakeSplitProviders(
    std::vector<std::unique_ptr<SplitProvider>>* split_providers) const {
  std::vector<const DatasetBase*> inputs;
  absl::Status s = InputDatasets(&inputs);
  if (absl::IsUnimplemented(s)) {
    return absl::UnimplementedError(absl::StrCat(
        "Cannot create split providers for dataset of type ", type_string(),
        ", because the dataset implements neither `InputDatasets` nor "
        "`MakeSplitProvider`."));
  }
  if (inputs.size() != 1) {
    return absl::UnimplementedError(absl::StrCat(
        "Cannot create split providers for dataset of type ", type_string(),
        ", because the dataset is not unary (instead having arity ",
        inputs.size(),
        "), and no custom implementation of `MakeSplitProvider` is defined."));
  }
  return inputs[0]->MakeSplitProviders(split_providers);
}

std::optional<int64_t> DatasetBase::GetEstimatedElementSize() const {
  const auto& shapes = output_shapes();
  const auto& dtypes = output_dtypes();
  if (shapes.size() != dtypes.size()) {
    LOG(ERROR) << "This should not happen because the sizes of output_shapes() "
                  "and output_dtypes() should always be "
                  "the same.";
    return std::nullopt;
  }

  size_t num_outputs = shapes.size();
  int64_t element_size = 0;
  for (int i = 0; i < num_outputs; ++i) {
    const auto& partial_shape = shapes[i];
    const auto& dtype = dtypes[i];
    auto num_elements = partial_shape.num_elements();
    if (num_elements == -1) {
      return std::nullopt;
    }
    element_size += num_elements * DataTypeSize(dtype);
  }
  return element_size;
}

int64_t DatasetBase::Cardinality() const {
  mutex_lock l(cardinality_mu_);
  if (cardinality_ == kUnknownCardinality) {
    CardinalityOptions options;
    cardinality_ = CardinalityInternal(options);
  }
  return cardinality_;
}

int64_t DatasetBase::Cardinality(CardinalityOptions options) const {
  mutex_lock l(cardinality_mu_);
  if (cardinality_ == kUnknownCardinality) {
    cardinality_ = CardinalityInternal(options);
  }
  return cardinality_;
}

absl::Status DatasetBase::InputDatasets(
    std::vector<const DatasetBase*>* inputs) const {
  return absl::UnimplementedError(absl::StrCat(
      "Cannot compute input sources for dataset of type ", type_string(),
      ", because the dataset does not implement `InputDatasets`. To fix this, "
      "your dataset should override the `InputDatasets` method. If it is a "
      "source dataset, it should return empty inputs."));
}

absl::Status DatasetBase::DatasetGraphDefBuilder::AddInputDataset(
    SerializationContext* ctx, const DatasetBase* dataset, Node** output) {
  absl::Status status = dataset->AsGraphDefInternal(ctx, this, output);
  if (ctx->is_graph_rewrite()) {
    if (status.ok()) {
      // Record cardinality in an unregistered attributes so that rewrites have
      // this information.
      (*output)->AddAttr(kCardinalityAttrForRewrite, dataset->Cardinality());
    } else if (absl::IsUnimplemented(status)) {
      Tensor t(DT_VARIANT, TensorShape({}));
      // `StoreDatasetInVariantTensor` will transfer ownership of `dataset`. We
      // increment the refcount of `dataset` here to retain ownership.
      dataset->Ref();
      TF_RETURN_IF_ERROR(
          StoreDatasetInVariantTensor(const_cast<DatasetBase*>(dataset), &t));
      TF_RETURN_IF_ERROR(AddPlaceholder(t, output));
      DCHECK_NE(ctx->input_list(), nullptr);
      ctx->input_list()->emplace_back((*output)->name(), std::move(t));
      LOG_EVERY_N_SEC(WARNING, 30)
          << "Input of " << dataset->DebugString()
          << " will not be optimized because the dataset does not implement "
             "the "
             "AsGraphDefInternal() method needed to apply optimizations.";
      return absl::OkStatus();
    }
  }
  return status;
}

absl::Status DatasetBase::DatasetGraphDefBuilder::AddDatasetOrTensor(
    SerializationContext* ctx, const Tensor& t, Node** output) {
  if (t.dtype() == DT_VARIANT) {
    // If the input tensor is a variant, it may represent a multi-dimensional
    // array of datasets. We attempt to decode each dataset so that we can use
    // their custom serialization logic and combine the result of their
    // individual serializations using the `Pack` operation.
    //
    // If this fails, we fallback to using its Variant::Encode() based
    // serialization.
    absl::Status s = AddDatasetOrTensorHelper(ctx, t, output);
    if (s.ok()) {
      return s;
    }
  }
  if (t.dtype() == DT_RESOURCE && !ctx->is_graph_rewrite()) {
    absl::Status s = AddResourceHelper(ctx, t, output);
    if (!absl::IsUnimplemented(s)) {
      // Fall through to AddTensor if AsGraphDef is not implemented for this
      // resource.
      return s;
    }
  }
  return AddTensor(t, output);
}

absl::Status DatasetBase::DatasetGraphDefBuilder::AddIdentity(
    SerializationContext* ctx, const std::string& name_prefix, Node** input,
    Node** output) {
  *output =
      ops::UnaryOp("Identity", *input,
                   builder()->opts().WithName(UniqueNodeName(name_prefix)));
  return absl::OkStatus();
}

absl::Status DatasetBase::DatasetGraphDefBuilder::AddDatasetOrTensorHelper(
    SerializationContext* ctx, const Tensor& t, Node** output) {
  if (t.dims() == 0) {
    DatasetBase* dataset;
    TF_RETURN_IF_ERROR(GetDatasetFromVariantTensor(t, &dataset));
    return AddInputDataset(ctx, dataset, output);
  }
  std::vector<NodeBuilder::NodeOut> nodes;
  for (int i = 0; i < t.dim_size(0); ++i) {
    Node* node;
    TF_RETURN_IF_ERROR(AddDatasetOrTensorHelper(ctx, t.SubSlice(i), &node));
    nodes.emplace_back(node);
  }
  auto op_name = "Pack";
  auto opts = builder()->opts();
  NodeBuilder node_builder(opts.GetNameForOp(op_name), op_name,
                           opts.op_registry());
  node_builder.Input(std::move(nodes));
  *output = opts.FinalizeBuilder(&node_builder);
  return absl::OkStatus();
}

absl::Status DatasetBase::DatasetGraphDefBuilder::AddResourceHelper(
    SerializationContext* ctx, const Tensor& t, Node** output) {
  if (t.NumElements() == 0) {
    return absl::InvalidArgumentError("Empty resouce handle");
  }
  const ResourceHandle& handle = t.flat<ResourceHandle>()(0);
  if (ctx->device_name() != handle.device()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Trying to access resource ", handle.name(), " located in device ",
        handle.device(), " from device ", ctx->device_name()));
  }
  ResourceBase* resource;
  TF_RETURN_IF_ERROR(ctx->resource_mgr()->Lookup(handle, &resource));
  core::ScopedUnref unref(resource);
  return resource->AsGraphDef(builder(), output);
}

DatasetBaseIterator::DatasetBaseIterator(const BaseParams& params)
    : params_(params) {
  params_.dataset->Ref();
  VLOG(2) << prefix() << " constructor";
  absl::StrAppend(&traceme_metadata_, "name=", dataset()->metadata().name());
  absl::StrAppend(&traceme_metadata_, ",shapes=");
  auto& shapes = output_shapes();
  for (int i = 0; i < shapes.size(); ++i) {
    if (i > 0) {
      absl::StrAppend(&traceme_metadata_, " ");
    }
    absl::StrAppend(&traceme_metadata_, shapes.at(i).DebugString());
  }
  absl::StrAppend(&traceme_metadata_, ",types=");
  auto& types = output_dtypes();
  for (int i = 0; i < types.size(); ++i) {
    if (i > 0) {
      absl::StrAppend(&traceme_metadata_, " ");
    }
    absl::StrAppend(&traceme_metadata_, DataTypeString(types.at(i)));
  }
}

DatasetBaseIterator::~DatasetBaseIterator() {
  VLOG(2) << prefix() << " destructor";
  params_.dataset->Unref();
}

std::string DatasetBaseIterator::BuildTraceMeName() {
  std::string result =
      strings::StrCat(params_.prefix, "#", traceme_metadata_, ",id=", id_);
  if (parent_) {
    absl::StrAppend(&result, ",parent_id=", parent_id_);
  }
  TraceMeMetadata metadata = GetTraceMeMetadata();
  for (const auto& pair : metadata) {
    absl::StrAppend(&result, ",", pair.first, "=", pair.second);
  }
  if (model_node() != nullptr) {
    if (model_node()->buffered_elements() > 0) {
      absl::StrAppend(
          &result, ",buffered_elements=",
          static_cast<long long>(model_node()->buffered_elements()));
      absl::StrAppend(
          &result, ",buffered_bytes_MB=",
          static_cast<long long>(
              static_cast<double>(model_node()->buffered_bytes()) * 1e-6));
    }
  }
  absl::StrAppend(&result, "#");
  return result;
}

absl::Status DatasetBaseIterator::GetNext(IteratorContext* ctx,
                                          std::vector<Tensor>* out_tensors,
                                          bool* end_of_sequence) {
  activity_watcher::ActivityScope activity_scope([&]() {
    activity_watcher::Activity::Attributes attributes;
    attributes["iterator_prefix"] = prefix();
    return std::make_unique<activity_watcher::Activity>(
        "Iterator::GetNext", activity_watcher::ActivityCategory::kDatasetOp,
        std::move(attributes));
  });
  tsl::profiler::TraceMe activity([&] { return BuildTraceMeName(); },
                                  tsl::profiler::TraceMeLevel::kInfo);
  DVLOG(3) << prefix() << " GetNext enter";
  auto model = ctx->model();
  bool output_was_recording =
      node_ && node_->output() && node_->output()->is_recording();
  if (collect_resource_usage(ctx)) {
    int64_t now_nanos = EnvTime::NowNanos();
    if (output_was_recording) {
      node_->output()->record_stop(now_nanos);
    }
    node_->record_start(now_nanos);
  }
  out_tensors->clear();
  absl::Status s = GetNextInternal(ctx, out_tensors, end_of_sequence);
  ctx->SaveCheckpoint(this);
  if (!SymbolicCheckpointCompatible()) {
    ctx->UpdateCheckpointStatus([this]() {
      return errors::UnimplementedError(
          dataset()->type_string(),
          " does not support symbolic checkpointing.");
    });
  }
  if (TF_PREDICT_TRUE(s.ok())) {
    if (TF_PREDICT_TRUE(!*end_of_sequence)) {
      if (TF_PREDICT_FALSE(out_tensors->size() !=
                           dataset()->output_dtypes().size())) {
        return errors::InternalError(
            "Expected ", dataset()->output_dtypes().size(),
            " components but got ", out_tensors->size(), ".");
      }
      RecordElement(ctx, out_tensors);
    } else {
      out_tensors->clear();
    }
  }
  if (collect_resource_usage(ctx)) {
    int64_t now_nanos = EnvTime::NowNanos();
    node_->record_stop(now_nanos);
    if (output_was_recording) {
      node_->output()->record_start(now_nanos);
    }
  }
  if (TF_PREDICT_FALSE(absl::IsOutOfRange(s))) {
    s = errors::InternalError(
        "Iterator \"", params_.prefix,
        "\" returned `OutOfRange`. This indicates an "
        "implementation error as `OutOfRange` errors are not "
        "expected to be returned here. Original message: ",
        s.message());
    LOG(ERROR) << s;
  }
  DVLOG(3) << prefix() << " GetNext exit";
  return s;
}

absl::Status DatasetBaseIterator::Skip(IteratorContext* ctx, int num_to_skip,
                                       bool* end_of_sequence,
                                       int* num_skipped) {
  tsl::profiler::TraceMe activity([&] { return BuildTraceMeName(); },
                                  tsl::profiler::TraceMeLevel::kInfo);
  DVLOG(3) << prefix() << " Skip enter";
  auto model = ctx->model();
  bool output_was_recording =
      node_ && node_->output() && node_->output()->is_recording();
  if (collect_resource_usage(ctx)) {
    int64_t now_nanos = EnvTime::NowNanos();
    auto output = node_->output();
    if (output_was_recording) {
      output->record_stop(now_nanos);
    }
    node_->record_start(now_nanos);
  }
  absl::Status s = SkipInternal(ctx, num_to_skip, end_of_sequence, num_skipped);
  if (collect_resource_usage(ctx)) {
    int64_t now_nanos = EnvTime::NowNanos();
    node_->record_stop(now_nanos);
    auto output = node_->output();
    if (output_was_recording) {
      output->record_start(now_nanos);
    }
  }
  if (TF_PREDICT_FALSE(absl::IsOutOfRange(s))) {
    s = absl::InternalError(
        absl::StrCat("Iterator \"", params_.prefix,
                     "\" returned `OutOfRange`. This indicates an "
                     "implementation error as `OutOfRange` errors are not "
                     "expected to be returned here. Original message: ",
                     s.message()));
    LOG(ERROR) << s;
  }
  DVLOG(3) << prefix() << " Skip exit";
  return s;
}

absl::Status DatasetBaseIterator::SkipInternal(IteratorContext* ctx,
                                               int num_to_skip,
                                               bool* end_of_sequence,
                                               int* num_skipped) {
  *num_skipped = 0;
  for (int i = 0; i < num_to_skip; ++i) {
    std::vector<Tensor> out_tensors;
    TF_RETURN_IF_ERROR(GetNextInternal(ctx, &out_tensors, end_of_sequence));
    if (*end_of_sequence) {
      return absl::OkStatus();
    }
    // RecordElement is used to count the number of element computed and
    // help calculate the CPU time spent on a given iterator to do the
    // autotuning.
    // Here we only call RecordElement in the default implementation of
    // SkipInternal (which trivially calls GetNextInternal) and assume
    // that the overridden SkipInternal in the derived class will have
    // negligible cost compare to its GetNextInternal.
    RecordElement(ctx, &out_tensors);
    (*num_skipped)++;
  }
  return absl::OkStatus();
}

void DatasetOpKernel::Compute(OpKernelContext* ctx) {
  DatasetBase* dataset = nullptr;
  MakeDataset(ctx, &dataset);
  if (ctx->status().ok()) {
    Tensor* output = nullptr;
    OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}), &output));
    OP_REQUIRES_OK(ctx, StoreDatasetInVariantTensor(dataset, output));
    if (ctx->stack_trace().has_value() && VLOG_IS_ON(4)) {
      VLOG(4) << "Dataset " << dataset->type_string()
              << " created using the following stack trace:";
      for (const auto& stack_frame : ctx->stack_trace()->ToStackFrames(
               {}, {}, /*reverse_traversal=*/false, /*limit=*/-1)) {
        VLOG(4) << stack_frame.file_name << ":" << stack_frame.line_number
                << " in " << stack_frame.function_name << "()";
      }
    }
    dataset->Initialize(metadata_);
  }
}

std::string DatasetOpKernel::TraceString(const OpKernelContext& ctx,
                                         bool verbose) const {
  return tsl::profiler::TraceMeOp(name_view(), type_string_view());
}

// static
bool DatasetOpKernel::IsDatasetOp(const OpDef& op_def) {
  if (op_def.output_arg_size() != 1) return false;
  if (op_def.output_arg(0).type() != DT_VARIANT) return false;
  absl::string_view op_name = op_def.name();

  // When running eager ops as a function, we check if the current op is a
  // Dataset op by unwrapping it. Below are some example op names when running
  // eager ops as a function:
  // 1. __wrapped__MapDataset_Targuments_0_device<...>
  // 2. __wrapped__FlatMapDataset_Targuments_0_device<...>
  // 3. __wrapped__ParallelMapDatasetV2_Targuments_0_device<...>
  //
  // Below are the corresponding unwrapped op names:
  // 1. MapDataset
  // 2. FlatMapDataset
  // 3. ParallelMapDatasetV2

  std::vector<std::string> v1, v2;  // Declared here so that v2 outlives op_name
  if (absl::StartsWith(op_name, "__wrapped__")) {
    v1 = absl::StrSplit(op_name, "__wrapped__", absl::SkipEmpty());
    if (v1.empty()) return false;
    v2 = absl::StrSplit(v1[0], "_", absl::SkipEmpty());
    op_name = v2.empty() ? v1[0] : v2[0];
  }

  if (op_name == "DatasetFromGraph") return true;
  if (absl::EndsWith(op_name, "Dataset")) return true;

  // Check if the suffix matches "DatasetV[0-9]+".
  size_t index = op_name.length() - 1;
  while (index >= 0 && absl::ascii_isdigit(op_name[index])) {
    index--;
  }
  constexpr absl::string_view kDatasetPrefix = "DatasetV";
  constexpr absl::string_view::size_type kPrefixLength = kDatasetPrefix.size();
  if (index < kPrefixLength - 1 || index == op_name.length() - 1) return false;
  return op_name.substr(index - kPrefixLength + 1, kPrefixLength) ==
         kDatasetPrefix;
}

void UnaryDatasetOpKernel::MakeDataset(OpKernelContext* ctx,
                                       DatasetBase** output) {
  DatasetBase* input;
  OP_REQUIRES_OK(ctx, GetDatasetFromVariantTensor(ctx->input(0), &input));
  MakeDataset(ctx, input, output);
}

void BinaryDatasetOpKernel::MakeDataset(OpKernelContext* ctx,
                                        DatasetBase** output) {
  DatasetBase* input;
  OP_REQUIRES_OK(ctx, GetDatasetFromVariantTensor(ctx->input(0), &input));
  DatasetBase* another_input;
  OP_REQUIRES_OK(ctx,
                 GetDatasetFromVariantTensor(ctx->input(1), &another_input));
  MakeDataset(ctx, input, another_input, output);
}

const char DatasetBase::kDatasetGraphKey[] = "_DATASET_GRAPH";
const char DatasetBase::kDatasetGraphOutputNodeKey[] =
    "_DATASET_GRAPH_OUTPUT_NODE";

BackgroundWorker::BackgroundWorker(Env* env, const char* name)
    : env_(env), name_(name) {}

BackgroundWorker::~BackgroundWorker() {
  {
    mutex_lock l(mu_);
    cancelled_ = true;
  }
  cond_var_.notify_one();
  // Block until the background thread has terminated.
  //
  // NOTE(mrry): We explicitly free and join the thread here because
  // `WorkerLoop()` uses other members of this object, and so we must join
  // the thread before destroying them.
  thread_.reset();
}

void BackgroundWorker::Schedule(std::function<void()> work_item) {
  {
    mutex_lock l(mu_);
    if (!thread_) {
      thread_ = absl::WrapUnique(env_->StartThread(
          {} /* thread_options */, name_, [this]() { WorkerLoop(); }));
    }
    work_queue_.push_back(std::move(work_item));
  }
  cond_var_.notify_one();
}

void BackgroundWorker::WorkerLoop() {
  tensorflow::ResourceTagger tag(kTFDataResourceTag, "Background");
  while (true) {
    std::function<void()> work_item = nullptr;
    {
      mutex_lock l(mu_);
      while (!cancelled_ && work_queue_.empty()) {
        cond_var_.wait(l);
      }
      if (cancelled_) {
        return;
      }
      DCHECK(!work_queue_.empty());
      work_item = std::move(work_queue_.front());
      work_queue_.pop_front();
    }
    DCHECK(work_item != nullptr);
    work_item();
  }
}

namespace {
class RunnerImpl : public Runner {
 public:
  void Run(const std::function<void()>& f) override {
    tensorflow::ResourceTagger tag(kTFDataResourceTag, "Runner");
    f();

    // NOTE: We invoke a virtual function to prevent `f` being tail-called, and
    // thus ensure that this function remains on the stack until after `f`
    // returns.
    PreventTailCall();
  }

 private:
  virtual void PreventTailCall() {}
};
}  // namespace

/* static */
Runner* Runner::get() {
  static Runner* singleton = new RunnerImpl;
  return singleton;
}

}  // namespace data
}  // namespace tensorflow

} // export

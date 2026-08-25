module;

/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <optional>
#include <thread>  // NOLINT
#include <utility>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/types/optional.h"
#include "tensorflow/core/framework/cancellation.h"
#include "tensorflow/core/framework/metrics.h"
#include "tensorflow/core/framework/model.pb.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/gtl/cleanup.h"
#include "tensorflow/core/lib/gtl/map_util.h"
#include "tensorflow/core/lib/histogram/histogram.h"
#include "tensorflow/core/lib/random/random.h"
#include "tensorflow/core/platform/cpu_info.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/path.h"
#include "tensorflow/core/platform/statusor.h"
#include "tensorflow/core/platform/strcat.h"
#include "tensorflow/core/platform/stringprintf.h"
#include "tsl/platform/mutex.h"
#include "tsl/platform/thread_annotations.h"
#include <cmath>
#include <queue>
#include "absl/time/clock.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/platform/host_info.h"
#include "tensorflow/core/platform/mem.h"
#include "tsl/platform/protobuf.h"

export module cc_tmp:utils_model;

import std;
import cc_abi;

export {

// TODO(b/114492873): Move this include into core/platform.


namespace tensorflow {
namespace data {
namespace model {

// A constant that can be used to enable auto-tuning.
constexpr int64_t kAutotune = -1;
constexpr char kParallelism[] = "parallelism";
constexpr char kBufferSize[] = "buffer_size";
constexpr char kCycleLength[] = "cycle_length";
constexpr char kDeterministic[] = "deterministic";
constexpr char kMaxBufferedElements[] = "max_buffered_elements";

// A key used to identify the input time of the model.
constexpr char kModelInputTimeKey[] = "model_input_time";

// Default share of available RAM that can be used by model's internal buffers.
constexpr double kRamBudgetShare = 0.5;

// Weight of the latest processing time used in computing the exponential moving
// average of processing time per element.
constexpr double kProcessingTimeEmaWeight = 0.1;

enum class TraversalOrder {
  BFS = 0,
  REVERSE_BFS = 1,
};

// Represents thread-safe state that can be shared between an input pipeline and
// the performance model.
struct SharedState {
 public:
  SharedState(int64_t value, std::shared_ptr<mutex> mu,
              std::shared_ptr<condition_variable> cond_var)
      : value(value),
        mu(std::move(mu)),
        cond_var(std::move(cond_var)),
        tunable(value == kAutotune) {}

  double value;
  const std::shared_ptr<mutex> mu;
  const std::shared_ptr<condition_variable> cond_var;
  const bool tunable;
};

// Represents a parameter.
struct Parameter {
  Parameter(const std::string& name, std::shared_ptr<SharedState> state,
            double min, double max)
      : name(name),
        // Sometimes non-autotune nodes (with `autotune_=false`) may contain
        // parameters (for example inputs of parallel interleave dataset which
        // are not in the current cycle). To avoid unrealistic situation
        // (say `buffer_size=-1` or `parallelism=-1`) in the optimization
        // computation, if the state value is `kAutotune=-1` (just to indicate
        // the `SharedState` is tunable), we initialize the parameter value to
        // be the minimal value of the state.
        value(state == nullptr || state->value == kAutotune ? min
                                                            : state->value),
        min(min),
        max(max),
        state(std::move(state)) {}

  explicit Parameter(const std::shared_ptr<Parameter> parameter)
      : name(parameter->name),
        value(parameter->value),
        min(parameter->min),
        max(parameter->max),
        state(parameter->state) {}

  // Human-readable name of the parameter.
  const std::string name;

  // Identifies the model value of the parameter. This can be different from
  // the actual value (e.g. during optimization search).
  double value;

  // Identifies the minimum value of the parameter.
  const double min;

  // Identifies the maximum value of the parameter.
  const double max;

  // Shared state of the parameter.
  std::shared_ptr<SharedState> state;
};

// Returns a new tunable parameter with the value set to `min`.
std::shared_ptr<Parameter> MakeParameter(const std::string& name,
                                         std::shared_ptr<SharedState> state,
                                         double min, double max);

// Returns a new tunable parameter with the value set to `value` instead
// of `min`.
std::shared_ptr<Parameter> MakeParameter(const std::string& name,
                                         std::shared_ptr<SharedState> state,
                                         double min, double max, double value);

// Returns a new non-tunable parameter.
std::shared_ptr<Parameter> MakeNonTunableParameter(const std::string& name,
                                                   double value);

// Class for managing the ram budget of an iterator. This is necessary for
// coordinating ram usage between the model-based autotuner and the legacy
// prefetch autotuner. Once the legacy autotuner is retired we can remove this
// class and move all ram budget management to the model autotuner.
class RamBudgetManager {
 public:
  explicit RamBudgetManager(int64_t budget) : budget_(budget) {
    if (budget <= 0) {
      LOG(WARNING) << "RAM budget is " << budget
                   << " which could prevent autotuner from properly adjusting "
                      "buffer sizes.";
    }
  }

  // Requests a new total memory allocation for the parts of the dataset
  // tuned by the model.
  //
  // The autotuner is expected to follow a pattern like
  //
  // int64_t budget = ram_budget_manager.AvailableModelRam();
  // NewModel potential_new_params = OptimizeModel(budget);
  // int64_t new_ram_used = potential_new_params.RamUsed();
  // if (ram_budget_manager.RequestModelAllocation(new_ram_used)) {
  //   ApplyModel(potential_new_params);
  // }
  //
  // Returns whether the request succeeded.
  bool RequestModelAllocation(int64_t total_bytes) {
    mutex_lock l(mu_);
    if (total_bytes > budget_ - legacy_prefetch_allocated_) {
      return false;
    }
    model_allocated_ = total_bytes;
    return true;
  }

  // Requests `delta_elements` allocated to the model where each element is of
  // size `element_size` bytes. `delta_elements` can be negative.
  // Returns the actual allocated delta elements.
  int64_t RequestModelBytes(int64_t delta_elements, double element_size) {
    if (delta_elements == 0) {
      return 0;
    }
    int64_t allocated_delta_elements = delta_elements;
    mutex_lock l(mu_);
    // If `delta_elements` is positive, allocate only up to the available
    // memory.
    if (delta_elements > 0) {
      int64_t max_delta_elements = static_cast<int64_t>(
          (budget_ - legacy_prefetch_allocated_ - model_allocated_) /
          element_size);
      if (max_delta_elements < 0) {
        return 0;
      }
      allocated_delta_elements = std::min(max_delta_elements, delta_elements);
    }
    model_allocated_ +=
        static_cast<int64_t>(allocated_delta_elements * element_size);
    return allocated_delta_elements;
  }

  // Requests `bytes` additional bytes for the purpose of legacy prefetch
  // autotuning.
  //
  // Unlike RequestModelAllocation, we use a delta number of bytes, since there
  // can only be one model per iterator but there may be multiple legacy
  // prefetch autotuners.
  //
  // Returns whether there were enough bytes left in the budget to serve the
  // request. If not, no bytes are allocated.
  bool RequestLegacyPrefetchBytes(int64_t delta_bytes) {
    mutex_lock l(mu_);
    if (delta_bytes > budget_ - legacy_prefetch_allocated_ - model_allocated_) {
      return false;
    }
    legacy_prefetch_allocated_ += delta_bytes;
    return true;
  }

  // The total number of bytes that the model could potentially use.
  int64_t AvailableModelRam() const {
    tf_shared_lock l(mu_);
    return budget_ - legacy_prefetch_allocated_;
  }

  void UpdateBudget(int64_t budget) {
    mutex_lock l(mu_);
    budget_ = budget;
    VLOG(2) << "Updated ram budget to " << budget;
  }

  std::string DebugString() {
    mutex_lock l(mu_);
    return absl::StrCat("RamBudgetManager: budget_: ", budget_,
                        " prefetch allocated: ", legacy_prefetch_allocated_,
                        " model allocated: ", model_allocated_);
  }

 private:
  mutable mutex mu_;
  int64_t budget_ TF_GUARDED_BY(mu_) = 0;
  // Number of bytes allocated by legacy prefetch autotuner.
  int64_t legacy_prefetch_allocated_ TF_GUARDED_BY(mu_) = 0;
  // Number of bytes allocated by the model.
  int64_t model_allocated_ TF_GUARDED_BY(mu_) = 0;
};

// Abstract representation of a TensorFlow input pipeline node. It collects
// information about inputs to this node, processing time spent executing the
// node logic, number of elements produced by the node, various other
// information (e.g. batch size or execution parallelism).
//
// Developers of tf.data transformations are not expected to interact with
// this class directly. Boiler plate code for creating the abstract
// representation of the input pipeline and collecting common information has
// been added to the implementation of `DatasetBase` and `DatasetBaseIterator`
// respectively.
//
// In addition, `DatasetBaseIterator` provides wrappers that can be used for
// transformation-specific information collection. The `SetMetadata` wrapper
// can be used to pass arbitrary metadata to the modeling framework, while the
// `StartWork` and `StopWork` wrappers should be used to correctly account for
// processing time of multi-threaded transformation that yield the CPU; such
// transformations should invoke `StartWork()` when a transformation thread
// starts executing (e.g. when created or woken up) and `StopWork()` when a
// transformation thread stops executing (e.g. when returning or waiting).
class Node {
 public:
  // Arguments for `Node` constructor.
  struct Args {
    int64_t id;
    std::string name;
    std::shared_ptr<Node> output;
  };

  using Factory = std::function<std::shared_ptr<Node>(Args)>;
  using NodeVector = std::vector<std::shared_ptr<Node>>;
  using NodePairList =
      std::list<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>>;
  using ModelParameters =
      std::vector<std::pair<std::string, std::shared_ptr<Parameter>>>;
  using NodeValues = absl::flat_hash_map<std::string, double>;
  using ParameterGradients =
      absl::flat_hash_map<std::pair<std::string, std::string>, double>;

  explicit Node(Args args)
      : id_(args.id),
        name_(std::move(args.name)),
        autotune_(true),
        buffered_bytes_(0),
        peak_buffered_bytes_(0),
        buffered_elements_(0),
        buffered_elements_low_(std::numeric_limits<int64_t>::max()),
        buffered_elements_high_(std::numeric_limits<int64_t>::min()),
        bytes_consumed_(0),
        bytes_produced_(0),
        num_elements_(0),
        processing_time_(0),
        record_metrics_(true),
        metrics_(name_),
        output_(args.output.get()),
        output_weak_ptr_(args.output) {}

  virtual ~Node() {
    // Clear the sub-nodes instead of relying on implicit shared pointer
    // destructor to avoid potential stack overflow when the tree is deep.
    std::deque<std::shared_ptr<Node>> queue;
    {
      mutex_lock l(mu_);
      while (!inputs_.empty()) {
        queue.push_back(inputs_.front());
        inputs_.pop_front();
      }
    }
    while (!queue.empty()) {
      auto node = queue.back();
      queue.pop_back();
      {
        mutex_lock l(node->mu_);
        while (!node->inputs_.empty()) {
          queue.push_back(node->inputs_.front());
          node->inputs_.pop_front();
        }
      }
    }

    FlushMetrics();
  }

  // Adds an input.
  void add_input(std::shared_ptr<Node> node) TF_LOCKS_EXCLUDED(mu_) {
    mutex_lock l(mu_);
    inputs_.push_back(node);
  }

  // Increments the aggregate processing time by the given delta.
  void add_processing_time(int64_t delta) TF_LOCKS_EXCLUDED(mu_) {
    processing_time_ += delta;
  }

  // Returns an indication whether autotuning is enabled for this node.
  bool autotune() const TF_LOCKS_EXCLUDED(mu_) { return autotune_; }

  // Returns the number of bytes stored in this node's buffer.
  int64_t buffered_bytes() const TF_LOCKS_EXCLUDED(mu_) {
    return buffered_bytes_;
  }

  // Returns the peak number of bytes stored in this node's buffer.
  int64_t peak_buffered_bytes() const TF_LOCKS_EXCLUDED(mu_) {
    return peak_buffered_bytes_;
  }

  // Returns the number of elements stored in this node's buffer.
  int64_t buffered_elements() const TF_LOCKS_EXCLUDED(mu_) {
    return buffered_elements_;
  }

  // Returns the low watermark of the number of elements stored in this node's
  // buffer. The watermarks are reset at the beginning of the execution time and
  // each time the buffer is upsized or downsized.
  int64_t buffered_elements_low() const TF_LOCKS_EXCLUDED(mu_) {
    return buffered_elements_low_;
  }

  // Returns the high watermark of the number of elements stored in this node's
  // buffer. The watermarks are reset at the beginning of the execution time and
  // each time the buffer is upsized or downsized.
  int64_t buffered_elements_high() const TF_LOCKS_EXCLUDED(mu_) {
    return buffered_elements_high_;
  }

  // Returns the number of bytes consumed by the node.
  int64_t bytes_consumed() const TF_LOCKS_EXCLUDED(mu_) {
    return bytes_consumed_;
  }

  // Returns the number of bytes produced by the node.
  int64_t bytes_produced() const TF_LOCKS_EXCLUDED(mu_) {
    return bytes_produced_;
  }

  // Indicates whether the node has tunable parameters.
  bool has_tunable_parameters() const TF_LOCKS_EXCLUDED(mu_) {
    tf_shared_lock l(mu_);
    for (const auto& pair : parameters_) {
      if (pair.second->state->tunable) return true;
    }
    return false;
  }

  // Returns the unique node ID.
  int64_t id() const TF_LOCKS_EXCLUDED(mu_) { return id_; }

  // Returns the node inputs.
  std::list<std::shared_ptr<Node>> inputs() const TF_LOCKS_EXCLUDED(mu_) {
    tf_shared_lock l(mu_);
    return inputs_;
  }

  // Returns a longer node name that is guaranteed to be unique.
  std::string long_name() const {
    return absl::StrCat(name_, "(id:", id_, ")");
  }

  // Returns the node name.
  const std::string& name() const { return name_; }

  // Returns the number of elements produced by the node.
  int64_t num_elements() const TF_LOCKS_EXCLUDED(mu_) { return num_elements_; }

  // Returns the node output.
  Node* output() const { return output_; }
  std::shared_ptr<Node> output_shared() { return output_weak_ptr_.lock(); }

  // Returns the parameter value.
  double parameter_value(const std::string& name) const TF_LOCKS_EXCLUDED(mu_) {
    tf_shared_lock l(mu_);
    return parameters_.at(name)->state->value;
  }

  // Returns the aggregate processing time.
  int64_t processing_time() const TF_LOCKS_EXCLUDED(mu_) {
    return processing_time_;
  }

  // Records that the node consumed the given number of bytes.
  void record_bytes_consumed(int64_t num_bytes) {
    bytes_consumed_ += num_bytes;
  }

  // Records that the node produced the given number of bytes.
  void record_bytes_produced(int64_t num_bytes) {
    bytes_produced_ += num_bytes;
  }

  // Records the change in this node's buffer.
  void record_buffer_event(int64_t bytes_delta, int64_t elements_delta) {
    buffered_bytes_ += bytes_delta;
    peak_buffered_bytes_.store(std::max(peak_buffered_bytes_, buffered_bytes_));
    buffered_elements_ += elements_delta;
    // There is no need to maintain watermarks for synchronous ops because we
    // will not upsize or downsize the buffers of synchronous ops.
    if (IsAsync()) {
      int64_t low_watermark =
          std::min(buffered_elements_low_, buffered_elements_);
      buffered_elements_low_ = low_watermark;
      int64_t high_watermark =
          std::max(buffered_elements_high_, buffered_elements_);
      buffered_elements_high_ = high_watermark;
    }
  }

  // Records that the node produced an element.
  void record_element() TF_LOCKS_EXCLUDED(mu_) {
    num_elements_++;
    {
      mutex_lock l(mu_);
      UpdateProcessingTimeEma();
    }
  }

  // Records that a node thread has started executing.
  void record_start(int64_t time_nanos) TF_LOCKS_EXCLUDED(mu_) {
    DCHECK_EQ(work_start_, 0);
    work_start_ = time_nanos;
  }

  // Records that a node thread has stopped executing.
  void record_stop(int64_t time_nanos) TF_LOCKS_EXCLUDED(mu_) {
    // TODO(jsimsa): Use DCHECK_NE(work_start_, 0) here.
    if (work_start_ != 0) {
      processing_time_ += time_nanos - work_start_;
      work_start_ = 0;
    } else {
      VLOG(1) << "Encountered a stop event without a matching start event.";
    }
  }

  // Returns whether work is currently being recorded, i.e. whether we are
  // currently between a `record_start` and a `record_stop`.
  bool is_recording() TF_LOCKS_EXCLUDED(mu_) { return work_start_ > 0; }

  // Removes an input.
  void remove_input(std::shared_ptr<Node> input) TF_LOCKS_EXCLUDED(mu_) {
    mutex_lock l(mu_);
    inputs_.remove(input);
  }

  // Sets the value that determines whether autotuning is enabled for this node.
  void set_autotune(bool autotune) TF_LOCKS_EXCLUDED(mu_) {
    autotune_.store(autotune);
  }

  // Resets buffer watermarks to the current buffered elements.
  void ResetBufferWatermarks() {
    if (!IsAsync()) {
      return;
    }
    int64_t current_buffer_size = buffered_elements_;
    buffered_elements_low_ = current_buffer_size;
    buffered_elements_high_ = current_buffer_size;
  }

  // Returns true for asynchronous nodes; false otherwise.
  virtual bool IsAsync() const { return false; }

  // Returns the ratio of the node, which is defined as the number of elements
  // per input needed by the node to produce an element, e.g. batch size of a
  // `Batch`. It can be 0 if the ratio is unknown.
  virtual double Ratio() const { return 1.0; }

  // Computes the self time in nanoseconds of the node to produce one element.
  virtual double ComputeSelfTime() const;

  // Returns the parameter value if it exists, not ok status otherwise.
  absl::StatusOr<double> ParameterValue(const std::string& parameter_name) const
      TF_LOCKS_EXCLUDED(mu_) {
    tf_shared_lock l(mu_);
    if (parameters_.contains(parameter_name)) {
      return parameters_.at(parameter_name)->value;
    }
    return absl::NotFoundError(absl::StrCat("Parameter ", parameter_name,
                                            " was not found in model node ",
                                            long_name()));
  }

  // Given the average time between events when the elements in the buffer are
  // produced (`producer_time`), the average time between events when elements
  // in the buffer are consumed (`consumer_time`) and the buffer size, the
  // method computes the expected time a consumer event will have to wait.
  //
  // The wait time is approximated as the product of the probability the buffer
  // will be empty and the time it takes to produce an element into the buffer.
  //
  // The formula used for computing the probability is derived by modeling the
  // problem as an M/M/1/K queue
  // (https://en.wikipedia.org/wiki/Birth%E2%80%93death_process#M/M/1/K_queue).
  //
  // Collects derivatives of `ComputeWaitTime` w.r.t `producer_time`,
  // `consumer_time' and `buffer_size` if the corresponding pointers are not
  // `nullptr`.
  static double ComputeWaitTime(double producer_time, double consumer_time,
                                double buffer_size,
                                double* producer_time_derivative,
                                double* consumer_time_derivative,
                                double* buffer_size_derivative);

  // Collects tunable parameters in the subtree rooted in this node.
  ModelParameters CollectTunableParameters() const TF_LOCKS_EXCLUDED(mu_);

  // Collects tunable parameters in this node.
  ModelParameters CollectNodeTunableParameters() const TF_LOCKS_EXCLUDED(mu_);

  // Returns a human-readable representation of this node.
  std::string DebugString() const TF_LOCKS_EXCLUDED(mu_);

  // Flushes the metrics recorded by this node.
  void FlushMetrics() TF_LOCKS_EXCLUDED(mu_);

  // Returns the per-element output time for this node and if `gradients` is not
  // `nullptr`, collects the output time gradient w.r.t. tunable parameters of
  // the subtree rooted in this node.
  double OutputTime(NodeValues* input_times,
                    ParameterGradients* gradients) const TF_LOCKS_EXCLUDED(mu_);

  // Returns a copy of this node, making a deep copy of its inputs and a
  // shallow copy of its tunable parameters.
  //
  // The purpose for this method is to allow the model optimization logic to
  // operate over immutable state while allowing concurrent model updates.
  std::shared_ptr<Node> Snapshot() const TF_LOCKS_EXCLUDED(mu_);

  // Returns the per-element processing time in nanoseconds spent in this node.
  double SelfProcessingTime() const TF_LOCKS_EXCLUDED(mu_);

  // Returns the total number of bytes buffered in all nodes in the subtree for
  // which autotuning is enabled.
  double TotalBufferedBytes() const TF_LOCKS_EXCLUDED(mu_);

  // Collects the total buffer limit of all nodes in the subtree for which
  // autotuning is enabled. This number represents the amount of memory that
  // would be used by the subtree nodes if all of their buffers were full.
  double TotalMaximumBufferedBytes() const TF_LOCKS_EXCLUDED(mu_);

  // Returns the per-element CPU time in nanoseconds spent in the subtree rooted
  // in this node. If `processing_times` is not `nullptr`, collects the
  // per-element CPU time spent in each node of the subtree.
  double TotalProcessingTime(NodeValues* processing_times)
      TF_LOCKS_EXCLUDED(mu_);

  // Produces a proto for this node. Does not produce a proto for input nodes.
  virtual absl::Status ToProto(ModelProto::Node* node_proto) const;

  // Restores a node from the proto. Does not restore input nodes.
  static absl::Status FromProto(ModelProto::Node node_proto,
                                std::shared_ptr<Node> output,
                                std::shared_ptr<Node>* node);

  // Returns a vector of nodes of the subtree rooted in this node. The nodes are
  // either in breadth-first search or reverse breadth-first search order
  // depending on the `order` argument. The nodes are collected based on the
  // results of the `collect_node` predicate: if the predicate returns `false`
  // for a given node, then the subtree rooted in this node is excluded. The
  // root node itself is not collected.
  NodeVector CollectNodes(TraversalOrder order,
                          bool collect_node(const std::shared_ptr<Node>)) const
      TF_LOCKS_EXCLUDED(mu_);

  // Downsizes buffer parameters of this node. Returns true if any buffer is
  // downsized.
  bool TryDownsizeBuffer();

  // Collects buffer parameters of this node that should be upsized.
  void CollectBufferParametersToUpsize(
      absl::flat_hash_map<Node*, Parameter*>& node_parameters);

  // Returns the average size of an element buffered in this node.
  double AverageBufferedElementSize() const {
    tf_shared_lock l(mu_);
    return AverageBufferedElementSizeLocked();
  }

  // Copies node's parameter state value to parameter value if the parameter
  // name matches `parameter_name`.
  void SyncStateValuesToParameterValues(const std::string& parameter_name);

  void SetEstimatedElementSize(std::optional<int64_t> estimated_element_size) {
    mutex_lock l(mu_);
    estimated_element_size_ = estimated_element_size;
  }

 protected:
  // Used for (incrementally) recording metrics. The class is thread-safe.
  class Metrics {
   public:
    explicit Metrics(const std::string& name)
        : bytes_consumed_counter_(metrics::GetTFDataBytesConsumedCounter(name)),
          bytes_produced_counter_(metrics::GetTFDataBytesProducedCounter(name)),
          num_elements_counter_(metrics::GetTFDataElementsCounter(name)),
          recorded_bytes_consumed_(0),
          recorded_bytes_produced_(0),
          recorded_num_elements_(0) {}

    // Expects the total number of bytes consumed and records the delta since
    // last invocation.
    void record_bytes_consumed(int64_t total_bytes) {
      int64_t delta =
          total_bytes - recorded_bytes_consumed_.exchange(total_bytes);
      bytes_consumed_counter_->IncrementBy(delta);
    }

    // Expects the total number of bytes produced and records the delta since
    // last invocation.
    void record_bytes_produced(int64_t total_bytes) {
      int64_t delta =
          total_bytes - recorded_bytes_produced_.exchange(total_bytes);
      bytes_produced_counter_->IncrementBy(delta);
    }

    // Expects the total number of elements produced and records the delta since
    // last invocation.
    void record_num_elements(int64_t total_elements) {
      int64_t delta =
          total_elements - recorded_num_elements_.exchange(total_elements);
      num_elements_counter_->IncrementBy(delta);
    }

   private:
    monitoring::CounterCell* const bytes_consumed_counter_;
    monitoring::CounterCell* const bytes_produced_counter_;
    monitoring::CounterCell* const num_elements_counter_;
    std::atomic<int64_t> recorded_bytes_consumed_;
    std::atomic<int64_t> recorded_bytes_produced_;
    std::atomic<int64_t> recorded_num_elements_;
  };

  // Computes the exponential moving average of processing time per element.
  void UpdateProcessingTimeEma() TF_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    if (previous_processing_time_ == 0) {
      if (num_elements_ > 0) {
        processing_time_ema_ =
            static_cast<double>(processing_time_) /
            static_cast<double>(num_elements_ + buffered_elements_);
      } else {
        processing_time_ema_ = static_cast<double>(processing_time_);
      }
    } else {
      processing_time_ema_ =
          (1.0 - kProcessingTimeEmaWeight) * processing_time_ema_ +
          kProcessingTimeEmaWeight *
              static_cast<double>(processing_time_ - previous_processing_time_);
    }
    previous_processing_time_ = processing_time_;
  }

  // Returns the number of inputs.
  int64_t num_inputs() const TF_SHARED_LOCKS_REQUIRED(mu_) {
    int64_t num_inputs = 0;
    for (auto& input : inputs_) {
      // Inputs for which autotuning is disabled are excluded.
      if (input->autotune()) {
        ++num_inputs;
      }
    }
    return num_inputs;
  }

  // Creates a clone of this node.
  virtual std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const
      TF_SHARED_LOCKS_REQUIRED(mu_) = 0;

  // Returns the average size of an element buffered in this node.
  double AverageBufferedElementSizeLocked() const TF_SHARED_LOCKS_REQUIRED(mu_);

  // Returns the sum of per-element output time for the tunable inputs of this
  // node.
  double OutputTimeForInputs(const NodeValues& output_times) const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Returns the sum of output time gradient w.r.t. input time for the tunable
  // inputs of this node.
  double OutputTimeGradientsForInputs(const NodeValues& output_time_gradients)
      const TF_SHARED_LOCKS_REQUIRED(mu_);

  // Computes the input time for this node and stores it in `input_times`.
  virtual void InputTimeLocked(NodeValues* input_times) const
      TF_SHARED_LOCKS_REQUIRED(mu_) = 0;

  // Computes the per-element output time for this node and stores it in
  // `output_times`. If `gradients` is not `nullptr`, computes the output time
  // gradient w.r.t. tunable parameters of the subtree rooted in this node and
  // stores it in `gradients`, also computes the output time gradient w.r.t.
  // input time and stores it in `output_time_gradients`.
  virtual void OutputTimeLocked(const NodeValues& input_times,
                                ParameterGradients* gradients,
                                NodeValues* output_times,
                                NodeValues* output_time_gradients) const
      TF_SHARED_LOCKS_REQUIRED(mu_) = 0;

  // Returns the sum of per-element processing time for the inputs of this node
  // by adding values for input nodes in `total_processing_times`. Processing
  // time for a given input is a weighted combination of a statistic based on
  // history of input processing time and the actual time. This is done to
  // improve accuracy of processing time estimation for newly created inputs.
  //
  // Uniform distribution of per-element processing times across different
  // inputs is assumed.
  double TotalProcessingTimeForInputs(const NodeValues& total_processing_times)
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Returns the per-element processing time spent in this node.
  double SelfProcessingTimeLocked() const TF_SHARED_LOCKS_REQUIRED(mu_);

  // Computes the per-element CPU time spent in the subtree rooted in this node
  // and stores it in `total_processing_times`. If `processing_times` is not
  // `nullptr`, collects the per-element CPU time spent in each node of the
  // subtree.
  virtual void TotalProcessingTimeLocked(NodeValues* processing_times,
                                         NodeValues* total_processing_times)
      TF_SHARED_LOCKS_REQUIRED(mu_) = 0;

  // This is the locked version of the public `CollectNodes`.
  NodeVector CollectNodesLocked(TraversalOrder order,
                                bool collect_node(const std::shared_ptr<Node>))
      const TF_SHARED_LOCKS_REQUIRED(mu_);

  // Collects tunable parameters in the subtree rooted in this node assuming
  // mutex locked.
  ModelParameters CollectTunableParametersLocked() const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Collect tunable parameters on the nodes which have recorded
  // elements.
  void CollectTunableParametersHelper(ModelParameters* parameters) const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Build up debug string for the node and store in the debug strings map.
  void DebugStringHelper(
      absl::flat_hash_map<std::string, std::string>* debug_strings) const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Copy the node and add the (input, copy) pairs to the NodePairList.
  std::shared_ptr<Node> SnapshotHelper(std::shared_ptr<Node> cloned_output,
                                       NodePairList* node_pairs) const;

  // Compute total buffered bytes for the node and store in the total bytes map.
  void TotalBufferedBytesHelper(NodeValues* total_bytes) const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Compute total maximum buffered bytes for the node and store in the total
  // bytes map.
  void TotalMaximumBufferedBytesHelper(NodeValues* total_bytes) const
      TF_SHARED_LOCKS_REQUIRED(mu_);

  // Compute and return the maximum buffered bytes on the node itself. By
  // default non-tunable nodes are assumed not to buffer any bytes, so the
  // tunable nodes as subclasses are expected to override this method to ensure
  // that the optimization algorithm respects the memory budget.
  virtual double MaximumBufferedBytes() const TF_SHARED_LOCKS_REQUIRED(mu_);

  // Restores node from the proto. Note that this is not done recursively, i.e.
  // input nodes are not restored.
  static absl::Status FromProtoHelper(ModelProto::Node node_proto,
                                      std::shared_ptr<Node> node);

  // Stores the time passed to the last call to `Node::record_start()` on the
  // current thread.
  //
  // NOTE: This thread-local variable is shared between all instances of `Node`
  // on which the same thread calls `record_start()` or `record_stop()`. It
  // relies on the invariant that at most one `Node` can be "active" on a
  // particular thread at any time. Therefore if `n->record_start()` is called
  // on thread `t`, then `n->record_stop()` must be called before another call
  // to `Node::record_start()` (for any node).
  static thread_local int64_t work_start_;  // Will be initialized to zero.

  mutable mutex mu_;
  const int64_t id_;
  const std::string name_;

  // Indicates whether the subtree rooted in this node should be included in
  // autotuning. In particular, if this is `false`, then the subtree is excluded
  // from computation of output time and processing time.
  std::atomic<bool> autotune_;
  std::atomic<int64_t> buffered_bytes_;
  std::atomic<int64_t> peak_buffered_bytes_;
  std::atomic<int64_t> buffered_elements_;
  std::atomic<int64_t> buffered_elements_low_;
  std::atomic<int64_t> buffered_elements_high_;
  std::atomic<int64_t> bytes_consumed_;
  std::atomic<int64_t> bytes_produced_;
  std::atomic<int64_t> num_elements_;
  std::atomic<int64_t> processing_time_;
  std::atomic<bool> record_metrics_;
  Metrics metrics_;
  absl::flat_hash_map<std::string, std::shared_ptr<Parameter>> parameters_
      TF_GUARDED_BY(mu_);

  // Statistic of inputs processing time history.
  double input_processing_time_sum_ = 0.0L;
  int64_t input_processing_time_count_ = 0;

  // Holds the previous processing time and the per element processing time
  // exponential moving average.
  int64_t previous_processing_time_ TF_GUARDED_BY(mu_) = 0;
  double processing_time_ema_ TF_GUARDED_BY(mu_) = 0.0;

  // Inputs of this node. These can represent an iterator created from the input
  // dataset but also other input iterators (e.g. created by the user-defined
  // functions of `flat_map` or `interleave`).
  std::list<std::shared_ptr<Node>> inputs_ TF_GUARDED_BY(mu_);

  // The reference to the output node is not owned so that deletion of a
  // node results in recursive deletion of the subtree rooted in the node.
  Node* const output_;
  std::weak_ptr<Node> output_weak_ptr_;
  std::optional<int64_t> estimated_element_size_ TF_GUARDED_BY(mu_) =
      std::nullopt;
};

// InterleaveMany is used to model datasets whose inputs are used to create
// datasets whose elements are then interleaved.
std::shared_ptr<Node> MakeInterleaveManyNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters);

// AsyncInterleaveMany nodes are the asynchronous version of InterleaveMany
// nodes.
std::shared_ptr<Node> MakeAsyncInterleaveManyNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters);

// KnownMany nodes model datasets that synchronously consume known number of
// input element per output element.
std::shared_ptr<Node> MakeKnownRatioNode(Node::Args args, double ratio);

// AsyncKnownRatio nodes are the asynchronous version of KnownRate nodes.
std::shared_ptr<Node> MakeAsyncKnownRatioNode(
    Node::Args args, double ratio, double memory_ratio,
    std::vector<std::shared_ptr<Parameter>> parameters,
    bool is_legacy_prefetch_autotuned = false);

// Makes an AsyncKnownRatioNode. If `estimated_element_size` is provided,
// it will be used during the estimation of maximum buffered bytes.
std::shared_ptr<Node> MakeAsyncKnownRatioNode(
    Node::Args args, double ratio,
    std::vector<std::shared_ptr<Parameter>> parameters,
    bool is_legacy_prefetch_autotuned = false,
    std::optional<int64_t> estimated_element_size = std::nullopt);

// Source nodes represent data sources.
std::shared_ptr<Node> MakeSourceNode(Node::Args args);

// UnknownMany nodes represent datasets that synchronously consume an
// unknown number of input elements per output.
//
// Unlike KnownRatio nodes which expect the ratio between inputs and outputs is
// specified as a parameter, UnknownRatio estimates the ratio empirically.
std::shared_ptr<Node> MakeUnknownRatioNode(Node::Args args);

// AsyncUnknownRatio nodes are the asynchronous version of unknown ratio nodes.
std::shared_ptr<Node> MakeAsyncUnknownRatioNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters);

// Unknown nodes represent datasets for which we do not have a model. It acts
// as pass-through between inputs and output.
std::shared_ptr<Node> MakeUnknownNode(Node::Args args);

// Abstract representation of a TensorFlow input pipeline that can be used
// for collecting runtime information and optimizing performance. It collects
// runtime information about execution of the input pipeline that is used to
// create a performance model, which is in turn used to identify optimal values
// of tunable parameters.
//
// Developers of tf.data transformations are not expected to interact with this
// class directly. Boiler plate code for creating the abstract representation of
// the input pipeline and collecting runtime information has been added to the
// implementation of `DatasetBase` and `DatasetBaseIterator` respectively.
//
// The order of locks acquired is SharedState lock, Model lock, Node lock.
// SharedState lock is acquired first because it shares the same lock as the
// dataset iterator that contains it.
class Model {
 public:
  using OptimizationParams = ModelProto::OptimizationParams;
  using ModelParameters = Node::ModelParameters;
  using NodeValues = Node::NodeValues;
  using ParameterGradients = Node::ParameterGradients;

  explicit Model(std::optional<std::string> dataset_name);
  explicit Model() : Model(std::nullopt) {}
  ~Model();

  // Returns a pointer to the model's output node.
  std::shared_ptr<Node> output() const {
    mutex_lock l(mu_);
    return output_;
  }

  // Set the experiment that this job is part of.
  void AddExperiment(const std::string& experiment) {
    experiments_.insert(experiment);
  }

  // Adds a node with the given name and given parent.
  void AddNode(Node::Factory factory, const std::string& name,
               std::shared_ptr<Node> parent, std::shared_ptr<Node>* out_node)
      TF_LOCKS_EXCLUDED(mu_);

  // Returns a human-readable string representation of the model. This method
  // can be invoked automatically by monitoring gauges and to avoid frequent
  // recomputation, the implementation caches the result.
  std::string DebugString();

  // Uses the given algorithm and resource budgets to periodically perform the
  // autotuning optimization.
  //
  // `cpu_budget_func` can be used to provide the optimizer with up-to-date
  // values in cases where CPUs budgets may be changed by the runtime
  // dynamically.
  //
  // `ram_budget_func` is similar to `cpu_budget_func`. This lambda takes a
  // parameter that is the total number of bytes currently buffered by the
  // model.
  //
  // To terminate the execution of the optimization loop, the caller needs to
  // invoke `cancellation_mgr->StartCancel()`.
  absl::Status OptimizeLoop(AutotuneAlgorithm algorithm,
                            std::function<int64_t()> cpu_budget_func,
                            double ram_budget_share,
                            std::optional<int64_t> fixed_ram_budget,
                            RamBudgetManager& ram_budget_manager,
                            CancellationManager* cancellation_manager);

  // Uses the given algorithm and resource budgets to perform the autotuning
  // optimization.
  void Optimize(AutotuneAlgorithm algorithm,
                std::function<int64_t()> cpu_budget_func,
                double ram_budget_share,
                std::optional<int64_t> fixed_ram_budget,
                double model_input_time, RamBudgetManager& ram_budget_manager,
                CancellationManager* cancellation_manager);

  // Optimizes buffers in the pipeline rooted at `snapshot`. It downsizes
  // buffers that are too large and upsizes buffers that are too small while
  // respecting the ram budget. If any node is downsized or upsized, the
  // watermarks of all nodes are reset to the buffered elements.
  void OptimizeBuffers(std::shared_ptr<Node> snapshot, int64_t ram_budget);

  // Collects the output time and if `gradients` is not `nullptr`, the output
  // time gradient w.r.t. tunable parameters of the subtree rooted in the given
  // node.
  double OutputTime(std::shared_ptr<Node> node, double model_input_time,
                    ParameterGradients* gradients);

  // Removes the given node.
  void RemoveNode(std::shared_ptr<Node> node) TF_LOCKS_EXCLUDED(mu_);

  // Produces a proto for this model.
  absl::Status ToProto(ModelProto* model_proto);

  // Restores a model from the proto.
  static absl::Status FromProto(ModelProto model_proto,
                                std::unique_ptr<Model>* model);

  // Saves this model with a given snapshot and its optimization parameters to a
  // file. Note that the file directory must already exist.
  absl::Status Save(const std::string& fname, std::shared_ptr<Node> snapshot,
                    const OptimizationParams& optimization_params);

  // Loads a model and its optimization parameters from a file with the given
  // name.
  static absl::Status Load(const std::string& fname,
                           std::unique_ptr<Model>* model,
                           OptimizationParams* optimization_params);

  // Records gap time between consecutive `GetNext()` calls.
  void RecordIteratorGapTime(uint64_t duration_usec);

  // Computes the target time in nsecs to use for `STAGE_BASED` autotune
  // algorithm. Returns 0 if there if there are not sufficient recorded iterator
  // gap times to produce a good estimate.
  double ComputeTargetTimeNsec();

  // Computes the target time in nsecs to use for estimating input bottlenecks.
  // Returns 0 if there are not sufficient recorded iterator gap times to
  // produce a good estimate.
  double ComputeExperimentalTargetTimeNsec();

  // Returns the time in nanoseconds it takes the pipeline to produce an
  // element, according to the latest model snapshot obtained from optimization.
  // Returns 0 if the model snapshot is empty or null. This may be caused by not
  // having executed an optimization round before.
  double ComputeSnapshotProcessingTimeNsec() const;

 private:
  // Determines whether optimization should stop given total processing time,
  // estimated output time, and estimated number of buffers bytes.
  using StopPredicate =
      std::function<bool(const ModelParameters&, double, double, double)>;

  static constexpr int64_t kOptimizationPeriodMinMs = 10;
  static constexpr int64_t kOptimizationPeriodMaxMs =
      60 * EnvTime::kSecondsToMillis;

  // Collects tunable parameters in the tree rooted in the given node, returning
  // a vector which contains pairs of node names and tunable parameters.
  ModelParameters CollectTunableParameters(std::shared_ptr<Node> node);

  // Copy parameter state values to parameter values if necessary.For some
  // nodes, the parameter state values are not tuned by Autotune and hence the
  // parameter values can be stale. We do not sync all parameters because it may
  // increase mutex contention with `GetNext()`.
  void MaybeSyncStateValuesToValues(std::shared_ptr<Node> snapshot);

  // Downsizes buffers that are too large for all nodes rooted at `snapshot`.
  // Returns true if any buffer is downsized.
  bool DownsizeBuffers(std::shared_ptr<Node> snapshot);

  // Upsizes buffers that are too small for all nodes rooted at `snapshot` while
  // respecting the ram budget. Returns true if any buffer is upsized.
  bool UpsizeBuffers(std::shared_ptr<Node> snapshot, int64_t ram_budget);

  // Reset buffer watermarks of all asynchronous nodes to their buffered
  // elements.
  void ResetBufferWatermarks();

  // Collects buffer parameters of all nodes in the model that should be
  // upsized.
  absl::flat_hash_map<Node*, Parameter*> CollectBufferParametersToUpsize(
      std::shared_ptr<Node> snapshot);

  // Flushes metrics recorded by the model.
  void FlushMetrics() TF_LOCKS_EXCLUDED(mu_);

  // This optimization algorithm starts by setting all tunable parallelism
  // parameters to the minimum value. It then improves current parameters by
  // making a step in the direction opposite to the gradient of `OutputTime` and
  // projecting resulting values on the feasible intervals. Improvement step is
  // repeated until either the output time improvement is smaller than threshold
  // value or the output time is less than the processing time needed to produce
  // an element divided by CPU budget.
  void OptimizeGradientDescent(std::shared_ptr<Node> snapshot,
                               const OptimizationParams& optimization_params,
                               CancellationManager* cancellation_manager);

  // Helper method for implementing hill-climb optimization that can be
  // parametrized by a predicate to use for stopping the optimization.
  void OptimizeHillClimbHelper(std::shared_ptr<Node> snapshot,
                               const OptimizationParams& optimization_params,
                               CancellationManager* cancellation_manager,
                               int64_t ram_budget,
                               RamBudgetManager& ram_budget_manager,
                               StopPredicate should_stop);

  // This optimization algorithm starts by setting all tunable parallelism
  // parameters to the minimum value. It then repeatedly identifies the
  // parameter whose increase in parallelism decreases the output time the most.
  // This process is repeated until all parameters reach their maximum values or
  // the projected output time is less than or equal to the processing time
  // needed to produce an element divided by CPU budget.
  void OptimizeHillClimb(std::shared_ptr<Node> snapshot,
                         const OptimizationParams& optimization_params,
                         CancellationManager* cancellation_manager,
                         RamBudgetManager& ram_budget_manager);

  // This optimization behaves similarly to the hill climb optimization but uses
  // a relaxed stoping condition, allowing the optimization to oversubscribe
  // CPU.
  void OptimizeMaxParallelism(std::shared_ptr<Node> snapshot,
                              const OptimizationParams& optimization_params,
                              CancellationManager* cancellation_manager,
                              RamBudgetManager& ram_budget_manager);

  // This optimization starts by setting all tunable parallelism parameters to
  // their minimum values. It then repeatedly increases the parallelism
  // parameter of the longest stage by 1 until either the longest stage is
  // faster than the target time or the memory or CPU budget is fully utilized.
  // TODO(b/226910071): The second part of this algorithm optimizes the buffer
  // sizes of parallel ops.
  void OptimizeStageBased(std::shared_ptr<Node> snapshot,
                          const OptimizationParams& optimization_params,
                          CancellationManager* cancellation_manager,
                          RamBudgetManager& ram_budget_manager);

  // This is the first part of the stage-based optimization that optimizes
  // tunable parallelism parameters for async interleave many nodes only. We
  // separately optimize async interleave many nodes more aggressively because
  // the variance of IO is difficult to predict.
  void OptimizeStageBasedAsyncInterleaveManyNodes(
      std::shared_ptr<Node> snapshot,
      const OptimizationParams& optimization_params,
      CancellationManager* cancellation_manager,
      RamBudgetManager& ram_budget_manager);

  // This is the second part of the stage-based optimization that optimizes
  // tunable parallelism parameters for all nodes other than async interleave
  // many nodes.
  void OptimizeStageBasedNonAsyncInterleaveManyNodes(
      std::shared_ptr<Node> snapshot, double target_time_nsec,
      const OptimizationParams& optimization_params,
      CancellationManager* cancellation_manager,
      RamBudgetManager& ram_budget_manager);

  // Determines if we should stop the gradient descent optimization iterations
  // based on number of increasable parameters, CPU budget, RAM budget and
  // current resource usage.
  bool ShouldStop(int64_t cpu_budget, int64_t ram_budget,
                  const ModelParameters& parameters,
                  const ModelParameters& parallelism_parameters,
                  const ModelParameters& buffer_size_parameters,
                  std::shared_ptr<Node> snapshot, bool* cpu_budget_reached);

  // Collects the processing time for the given node.
  double TotalProcessingTime(std::shared_ptr<Node> node);

  // Collects the total number of bytes buffered in all nodes in the subtree
  // rooted in the given node for which autotuning is enabled.
  double TotalBufferedBytes(std::shared_ptr<Node> node);

  // Collects the total buffer limit of all nodes in the subtree rooted in the
  // given node for which autotuning is enabled. This number represents the
  // amount of memory that would be used by the subtree nodes if all of their
  // buffers were full.
  double TotalMaximumBufferedBytes(std::shared_ptr<Node> node);

  std::optional<std::string> dataset_name_;
  // Used for coordination between different input pipeline threads. Exclusive
  // access is required only when adding or removing nodes. Concurrent access to
  // existing nodes is protected by a node mutex.
  mutable mutex mu_;
  // Used for coordinating the optimization loop and model modifications.
  condition_variable optimize_cond_var_;
  int64_t id_counter_ TF_GUARDED_BY(mu_) = 1;
  std::shared_ptr<Node> output_ TF_GUARDED_BY(mu_) = nullptr;

  // Determines the time the optimization loop should wait between
  // running optimizations.
  int64_t optimization_period_ms_ TF_GUARDED_BY(mu_);

  // Gauge cell that can be used to collect the state of the model.
  monitoring::GaugeCell<std::function<std::string()>>* model_gauge_cell_ =
      nullptr;
  // Used to synchronize metrics collection attempts against the model's
  // destruction.
  struct GuardedBool {
    explicit GuardedBool(bool val) : val(val) {}
    bool val TF_GUARDED_BY(mu);
    mutex mu;
  };
  std::shared_ptr<GuardedBool> safe_to_collect_metrics_;

  // Time use for rate limiting the recomputation of human-readable string
  // representation of the model.
  absl::Time cache_until_ = absl::InfinitePast();
  // Cached result of the `DebugString()` invocation used to implement rate
  // limiting of the computation.
  std::string cached_debug_string_ = "";
  // Used to coordinate gap time updates between different threads. Gap time is
  // the time between the completion of the previous `GetNext()` and the start
  // of the next `GetNext()`.
  mutable mutex gap_mu_;
  // Stores the latest gap times between consecutive `GetNext()`.
  std::deque<uint64_t> gap_times_usec_ TF_GUARDED_BY(gap_mu_);
  // The experiment that this job is part of.
  absl::flat_hash_set<std::string> experiments_;
  // Stores the optimization snapshot of the Model.
  std::shared_ptr<Node> snapshot_ TF_GUARDED_BY(mu_);
  // Stores the optimization parameters used by autotune.
  OptimizationParams optimization_params_ TF_GUARDED_BY(mu_);
  // Stores the model id in the string format
  std::string model_id_;
};

// Class to compute timing information for a model.
class ModelTiming {
 public:
  struct NodeTiming {
    // Pipeline ratio is the number of elements this node needs to produce in
    // order to produce an element at the root of the pipeline.
    double pipeline_ratio = 0.0;
    // The self time it takes this node to produce the elements needed to
    // produce one element of the root of the pipeline.
    double self_time_nsec = 0.0;
    // The total time it takes this node and the subtree rooted at this node to
    // produce the elements needed to produce one element at the root of the
    // pipeline.
    double total_time_nsec = 0.0;
  };

  explicit ModelTiming(std::shared_ptr<Node> root);

  // Returns the timing data for `node`.
  const NodeTiming* GetTiming(const Node* node) const;

  // Returns the root nodes of all stages.
  std::vector<std::shared_ptr<Node>> GetStageRoots() const;

  // Returns all the nodes of a stage given the stage root.
  std::vector<std::shared_ptr<Node>> GetStageNodes(
      std::shared_ptr<Node> stage_root) const;

  // Computes the total time for a node.
  void ComputeNodeTotalTime(const Node& node);

 private:
  // Computes the pipeline ratios of all nodes.
  void ComputePipelineRatios(const Node::NodeVector& bfs_nodes);

  // Computes the total time for all nodes. The `reverse_bfs_nodes` are assumed
  // to be a vector of model nodes in reversed BFS manner.
  void ComputeTotalTimes(const Node::NodeVector& reverse_bfs_nodes);

  // Computes the first input total time of an interleave node.
  double ComputeInterleaveManyFirstInputTotalTime(const Node& node);

  // Computes the total time of a node of any type other than async interleave.
  void ComputeNonAsyncInterleaveManyTotalTime(const Node& node);

  // Computes the total time of an async interleave node.
  void ComputeAsyncInterleaveManyTotalTime(const Node& node);
  // Computes the interleaved inputs' total time of an async interleave node.
  double ComputeAsyncInterleaveManyInterleavedInputsTotalTime(const Node& node);

  // Returns a vector of all nodes in the model. The nodes are either in
  // breadth-first search or reverse breadth-first search order depending on the
  // `order` argument. The nodes are collected based on the results of the
  // `collect_node` predicate: if the predicate returns `false` for a given
  // node, then the subtree rooted in this node is excluded. The root node
  // itself is not collected.
  Node::NodeVector CollectNodes(
      std::shared_ptr<Node> root, TraversalOrder order,
      bool collect_node(const std::shared_ptr<Node>)) const;

  // Stores a pointer to the root of a model.
  std::shared_ptr<Node> root_;

  // Holds a mapping from node to its timing node.
  absl::flat_hash_map<const Node*, NodeTiming> timing_nodes_;
};

}  // namespace model
}  // namespace data
}  // namespace tensorflow

// ==================================================================
// Implementation: model.cc
// ==================================================================

namespace tensorflow {
namespace data {
namespace model {

constexpr int64_t Model::kOptimizationPeriodMinMs;
constexpr int64_t Model::kOptimizationPeriodMaxMs;

namespace {

// This is the number of the latest gap times used to compute the target time
// for stage based optimization.
constexpr int32_t kGapTimeWindow = 100;
// Gap time upper threshold: any gap time over this duration will be dropped.
constexpr absl::Duration kGapDurationUpperThreshold = absl::Seconds(10);
// In outlier computation, points that are larger than `kOutlierSigmas` standard
// deviations are considered outliers.
constexpr double kOutlierSigmas = 2.0;
// In target time computation, compute the target time as `kTargetTimeSigmas`
// from the mean of the gap time distribution to account for variance in
// processing time. For example, a value of 1 would mean that the target time is
// faster than 84% of the gap times.
constexpr double kTargetTimeSigmas = 1.0;
// The scaling factors to apply to buffer optimization when upsizing or
// downsizing a buffer.
constexpr double kBufferUpsizeMultiplier = 2.0;
constexpr double kBufferDownsizeMultipliter = 0.9;
// Threshold of low buffer watermark before a buffer is a candidate for
// upsizing.
constexpr int64_t kBufferLowWatermarkThreshold = 2;

constexpr char kDataService[] = "DataService";
constexpr char kFlatMap[] = "FlatMap";
constexpr char kInterleave[] = "Interleave";
constexpr char kParallelInterleave[] = "ParallelInterleave";

// A class to prune outliers given a set of points. To use it, instantiate an
// object and call the `GetCleanPoints()` method.
class TargetTimeCalculator {
 public:
  explicit TargetTimeCalculator(const std::vector<uint64_t>& points_usec,
                                double outlier_sigmas,
                                double target_time_sigmas)
      : points_usec_(points_usec.begin(), points_usec.end()),
        outlier_sigmas_(outlier_sigmas),
        target_time_sigmas_(target_time_sigmas) {}

  double GetTargetTimeUsec() const {
    if (points_usec_.empty()) {
      return 0.0;
    }
    double mean;
    double standard_deviation;
    ComputeMeanAndStandardDeviation(points_usec_, &mean, &standard_deviation);
    // Remove outliers.
    std::vector<uint64_t> clean_points_usec =
        GetCleanPoints(points_usec_, mean, standard_deviation);
    if (clean_points_usec.empty()) {
      return 0.0;
    }
    // Compute mean and standard deviation after outliers are removed.
    ComputeMeanAndStandardDeviation(clean_points_usec, &mean,
                                    &standard_deviation);
    // Compute target time.
    return std::max(0.0, mean - standard_deviation * target_time_sigmas_);
  }

 private:
  // Returns the remaining points after removing outliers from the original set
  // of points.
  std::vector<uint64_t> GetCleanPoints(const std::vector<uint64_t>& points_usec,
                                       double mean,
                                       double standard_deviation) const {
    double threshold = mean + standard_deviation * outlier_sigmas_;
    std::vector<uint64_t> clean_points_usec;
    for (auto point : points_usec) {
      if (static_cast<double>(point) > threshold) {
        continue;
      }
      clean_points_usec.push_back(point);
    }
    return clean_points_usec;
  }

  void ComputeMeanAndStandardDeviation(const std::vector<uint64_t>& points_usec,
                                       double* mean,
                                       double* standard_deviation) const {
    uint64_t sum = std::accumulate(points_usec.begin(), points_usec.end(), 0);
    *mean = static_cast<double>(sum) / static_cast<double>(points_usec.size());
    double accum = 0.0;
    for (auto point : points_usec) {
      accum += (static_cast<double>(point) - *mean) *
               (static_cast<double>(point) - *mean);
    }
    *standard_deviation = std::sqrt(accum / (points_usec.size() - 1));
  }

  // Points to cluster.
  std::vector<uint64_t> points_usec_;
  double outlier_sigmas_;
  double target_time_sigmas_;
};

// A priority queue that holds stage roots where the top of the priority queue
// is the node with the largest total time.
class ModelTimingPriorityQueue {
 public:
  explicit ModelTimingPriorityQueue(ModelTiming& model_timing) {
    std::vector<std::shared_ptr<Node>> stage_roots =
        model_timing.GetStageRoots();
    if (stage_roots.empty()) {
      return;
    }
    for (auto& root : stage_roots) {
      DCHECK(model_timing.GetTiming(root.get()) != nullptr);
      const ModelTiming::NodeTiming* root_timing =
          model_timing.GetTiming(root.get());
      Push(root.get(), *root_timing);
    }
  }

  // Pops the top item from the queue, i.e. node with the largest total time.
  absl::StatusOr<std::pair<double, Node*>> PopSlowestStageRoot() {
    if (stage_roots_queue_.empty()) {
      return absl::InternalError(
          "Model timing priority queue is empty during stage-based "
          "optimization");
    }
    std::pair<double, Node*> top_item = stage_roots_queue_.top();
    stage_roots_queue_.pop();
    return top_item;
  }

  // Push a node together with its total time onto the queue.
  void Push(Node* node, const ModelTiming::NodeTiming& node_timing) {
    stage_roots_queue_.emplace(
        node_timing.total_time_nsec * node_timing.pipeline_ratio, node);
  }

 private:
  std::priority_queue<std::pair<double, Node*>> stage_roots_queue_;
};

// A cache that looks up the `parallelism` parameters of nodes the first time
// they are requested and saves them for subsequent requests.
class NodeParallelismParameters {
 public:
  NodeParallelismParameters() = default;

  // Returns the `parallelism` parameter given a node.
  Parameter* Get(const Node* node) {
    if (node_parallelism_.contains(node)) {
      // Look for the `parallelism` parameter of this node in the cache.
      return node_parallelism_.at(node);
    }
    // Find the `parallelism` parameter of this node and cache it.
    Node::ModelParameters parameters = node->CollectNodeTunableParameters();
    Node::ModelParameters::iterator parameter_pair = std::find_if(
        parameters.begin(), parameters.end(),
        [](const std::pair<std::string, std::shared_ptr<Parameter>>&
               parameter) { return parameter.second->name == kParallelism; });
    if (parameter_pair == parameters.end()) {
      return nullptr;
    }
    node_parallelism_[node] = parameter_pair->second.get();
    return parameter_pair->second.get();
  }

 private:
  absl::flat_hash_map<const Node*, Parameter*> node_parallelism_;
};

// Replaces `\[[0-9].+\]` with `\[\]`.
std::string RemoveArrayIndices(absl::string_view s) {
  absl::string_view::size_type start_pos = 0;
  absl::string_view::size_type pos;
  std::string res;
  do {
    pos = s.find('[', start_pos);
    if (pos == absl::string_view::npos) {
      break;
    }
    res.append(s.data() + start_pos, pos - start_pos + 1);
    start_pos = pos + 1;
    pos = s.find(']', start_pos);
    if (pos == absl::string_view::npos) {
      break;
    }
    res.append(s.data() + pos, 1);
    start_pos = pos + 1;
  } while (true);
  res.append(s.data() + start_pos, s.length() - start_pos);
  return res;
}

// Returns true if all parameters have reached their max values.
bool AreAllParametersMax(const Model::ModelParameters& parameters) {
  for (const auto& pair : parameters) {
    if (pair.second->value < pair.second->max) {
      return false;
    }
  }
  return true;
}

// Records the ram usage of hill climbing algorithm.
void RecordAutotuneRamUsage(int64_t ram_budget, double max_buffered_bytes) {
  if (ram_budget == 0) {
    return;
  }
  const auto memory_info = port::GetMemoryInfo();
  // Records ratio of memory used since RootDataset was created over the ram
  // budget.
  const auto original_free_memory = ram_budget / kRamBudgetShare;
  const auto current_free_memory = memory_info.free;
  metrics::RecordTFDataAutotuneUsedRamBudgetRatio(
      (original_free_memory - current_free_memory) / ram_budget);
  // Records ratio of maximum buffer bytes tf.data could use over the ram
  // budget.
  metrics::RecordTFDataAutotuneMaxBufferBudgetRatio(
      max_buffered_bytes / static_cast<double>(ram_budget));
}

// Helper function for node traversal that doesn't skip any nodes.
inline bool IsAnyNode(const std::shared_ptr<Node> node) { return true; }

// Helper function for node traversal that filters out nodes for which
// autotuning is disabled.
inline bool IsAutotuneNode(const std::shared_ptr<Node> node) {
  return node->autotune();
}

// Helper function for node traversal that returns only synchronous nodes.
inline bool IsSyncNode(const std::shared_ptr<Node> node) {
  return !node->IsAsync();
}

// Helper function for node traversal that returns only asynchronous interleave
// many nodes.
inline bool IsAsyncInterleaveManyNode(const std::shared_ptr<Node> node) {
  return absl::StartsWith(node->name(), kParallelInterleave);
}
inline bool IsAsyncInterleaveManyNode(const Node* node) {
  return absl::StartsWith(node->name(), kParallelInterleave);
}

// Wrapper for the square function to reduce verbosity.
inline double Square(double x) { return x * x; }

// Collects "essential" parallelism parameters and buffer size parameters in the
// tree rooted in the given node. Which parallelism parameters are essential is
// determined by the relative processing time spent in the corresponding
// transformation. The collected parameters are returned via maps that map node
// names to their respective parameters.
inline void CollectParameters(std::shared_ptr<Node> node,
                              const Node::ModelParameters& parameters,
                              Node::ModelParameters* parallelism_parameters,
                              Node::ModelParameters* buffer_size_parameters) {
  // Parallelism parameter is considered to be essential if the corresponding
  // transformations's processing time is greater than essential rate times the
  // average transformation self processing time.
  constexpr double kEssentialRate = 0.3L;

  Node::NodeValues processing_times;
  double processing_time = node->TotalProcessingTime(&processing_times);
  double uniform_share =
      processing_time / static_cast<double>(processing_times.size());
  for (auto& pair : parameters) {
    if (pair.second->name == kParallelism &&
        processing_times[pair.first] > kEssentialRate * uniform_share) {
      parallelism_parameters->push_back(pair);
    } else if (pair.second->name == kBufferSize) {
      buffer_size_parameters->push_back(pair);
    }
  }
}

// Applies the gradient descent method once and updates the parameter values. If
// the new value is out of the range, bound it within the range between the
// minimal and maximum values.
inline void UpdateParameterValues(const Node::ParameterGradients& gradients,
                                  Node::ModelParameters* parameters) {
  // Gradient descent step size.
  constexpr double kDescentStep = 0.1L;
  double new_value;

  double max_abs_derivative = 1.0;
  for (auto& pair : *parameters) {
    if (std::round(pair.second->value) != pair.second->max) {
      auto* gradient = gtl::FindOrNull(
          gradients, std::make_pair(pair.first, pair.second->name));
      if (gradient) {
        max_abs_derivative = std::max(max_abs_derivative, std::abs(*gradient));
      }
    }
  }
  for (auto& pair : *parameters) {
    auto* gradient = gtl::FindOrNull(
        gradients, std::make_pair(pair.first, pair.second->name));
    if (gradient) {
      new_value =
          pair.second->value - kDescentStep * (*gradient) / max_abs_derivative;
      // Projection on a feasible interval.
      if (new_value > pair.second->max) {
        pair.second->value = pair.second->max;
      } else if (new_value < pair.second->min) {
        pair.second->value = pair.second->min;
      } else {
        pair.second->value = new_value;
      }
    }
  }
}

// Copies the parameter values (which are for optimization tuning) and updates
// the state values (which are for the input pipeline to follow).
inline void UpdateStateValues(Node::ModelParameters* parameters) {
  for (auto& pair : *parameters) {
    auto& parameter = pair.second;
    VLOG(2) << "Setting tunable parameter " << pair.first
            << ":: " << parameter->name << " to " << parameter->value;
    mutex_lock l(*parameter->state->mu);
    parameter->state->value = parameter->value;
    parameter->state->cond_var->notify_all();
  }
}

// Recursively produces protos for nodes in a subtree of `output` node and
// appends them to nodes of the given model.
absl::Status ModelToProtoHelper(std::shared_ptr<Node> output,
                                ModelProto* model) {
  model->set_output(output->id());
  std::list<std::shared_ptr<Node>> to_serialize = {output};
  auto& nodes = *model->mutable_nodes();
  while (!to_serialize.empty()) {
    const std::shared_ptr<Node> node = to_serialize.front();
    to_serialize.pop_front();
    TF_RETURN_IF_ERROR(node->ToProto(&(nodes[node->id()])));
    for (const auto& input : node->inputs()) {
      to_serialize.push_back(input);
    }
  }
  return absl::OkStatus();
}

// Recursively produces node tree rooted in `output` from the given model proto.
absl::Status ModelFromProtoHelper(ModelProto model,
                                  std::shared_ptr<Node>* output) {
  if (model.nodes().empty()) {
    return absl::InternalError(
        "Cannot restore model from proto because it has no nodes.");
  }
  TF_RETURN_IF_ERROR(Node::FromProto(model.nodes().at(model.output()),
                                     /*output=*/nullptr, output));
  std::list<std::shared_ptr<Node>> to_restore_inputs = {*output};
  while (!to_restore_inputs.empty()) {
    std::shared_ptr<Node> node = to_restore_inputs.front();
    to_restore_inputs.pop_front();
    for (int64_t input_id : model.nodes().at(node->id()).inputs()) {
      std::shared_ptr<Node> input;
      TF_RETURN_IF_ERROR(
          Node::FromProto(model.nodes().at(input_id), node, &input));
      node->add_input(input);
      to_restore_inputs.push_back(input);
    }
  }
  return absl::OkStatus();
}

// The first input of InterleaveMany corresponds to the input dataset whose
// elements are used to create the (derived) input datasets whose elements are
// interleaved as output.
//
// TODO(jsimsa): model the first input
class InterleaveMany : public Node {
 public:
  using Node::Node;

  InterleaveMany(Node::Args args,
                 std::vector<std::shared_ptr<Parameter>> parameters)
      : Node(args) {
    for (auto& parameter : parameters) {
      parameters_[parameter->name] = std::move(parameter);
    }
  }

  ~InterleaveMany() override = default;

  // The ratio of an InterleaveMany node is `1/cycle_length`. If cycle length is
  // not available, we approximate it by `1/input_size`. The input size does not
  // include the original input dataset that generates other input datasets of
  // interleave nodes.
  double Ratio() const override {
    auto* cycle_length = gtl::FindOrNull(parameters_, kCycleLength);
    if (cycle_length != nullptr) {
      return 1.0 / (*cycle_length)->value;
    }
    // After cl/436244658, `cycle_length` can not be `nullptr`. The remaining
    // part of this function is used to approximate `Ratio()` of this node for
    // model proto that was created before the CL.

    // Cycle length is not available, use 1/input_size as the ratio.
    std::size_t input_size = 1;
    {
      mutex_lock l(mu_);
      if (inputs_.size() >= 2) {
        auto first_input = inputs_.begin();
        auto second_input = std::next(first_input);
        // Some interleave datasets have 2 different inputs: the original input
        // dataset and the generated input datasets when interleave is iterated,
        // and some do not.
        if ((*first_input)->name() == (*second_input)->name()) {
          input_size = std::max(inputs_.size(), input_size);
        } else {
          input_size = std::max(inputs_.size() - 1, input_size);
        }
      }
    }
    if (input_size == 0) {
      return 1.0;
    }
    return 1.0 / static_cast<double>(input_size);
  }

 protected:
  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    return std::make_shared<InterleaveMany>(
        Args{id_, name_, std::move(output)});
  }

  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }

    if (num_inputs() <= 1) {
      (*input_times)[long_name()] = inherited_input_time;
      return;
    }
    // Here `inherited_input_time + SelfProcessingTimeLocked()` is the average
    // input time for InterleaveMany node to call one of the `(num_inputs() -
    // 1)` input nodes (except first input) to return an element. Regardless of
    // the `block_length` parameter of InterleaveMany node, the average input
    // time for any of the `(num_inputs() - 1)` input nodes to be called is
    // computed as:
    double input_time = (inherited_input_time + SelfProcessingTimeLocked()) *
                        static_cast<double>(num_inputs() - 1);
    (*input_times)[long_name()] = input_time;
  }

  // The output time is the sum of the self processing time and the average
  // output time of inputs comprising the interleave "cycle".
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (num_inputs() <= 1) {
      (*output_times)[long_name()] = self_processing_time;
      if (gradients) {
        for (const auto& pair : CollectTunableParametersLocked()) {
          gradients->erase(std::make_pair(pair.first, pair.second->name));
        }
      }
      return;
    }

    double inputs_output_time =
        (OutputTimeForInputs(*output_times) -
         (*output_times)[inputs_.front()->long_name()]) /
        static_cast<double>(num_inputs() - 1);
    if (gradients) {
      for (const auto& pair : CollectTunableParametersLocked()) {
        auto* gradient = gtl::FindOrNull(
            *gradients, std::make_pair(pair.first, pair.second->name));
        if (gradient) {
          *gradient /= static_cast<double>(num_inputs() - 1);
        }
      }

      (*output_time_gradients)[long_name()] =
          OutputTimeGradientsForInputs(*output_time_gradients) -
          (*output_time_gradients)[inputs_.front()->long_name()];

      // Set derivatives w.r.t. tunable parameters of the subtree rooted in the
      // first input equal to 0 since its output time is excluded from
      // computations.
      for (auto& pair : inputs_.front()->CollectTunableParameters()) {
        (*gradients)[std::make_pair(pair.first, pair.second->name)] = 0.0L;
      }
    }
    (*output_times)[long_name()] = self_processing_time + inputs_output_time;
  }

  // The processing time is the sum of the self processing time and the average
  // processing time of inputs comprising the interleave "cycle".
  void TotalProcessingTimeLocked(NodeValues* processing_times,
                                 NodeValues* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (processing_times) {
      (*processing_times)[long_name()] = self_processing_time;
    }
    if (num_inputs() <= 1) {
      (*total_processing_times)[long_name()] = self_processing_time;
      return;
    }
    double inputs_processing_time =
        (TotalProcessingTimeForInputs(*total_processing_times) -
         (*total_processing_times)[inputs_.front()->long_name()]) /
        static_cast<double>(num_inputs() - 1);
    (*total_processing_times)[long_name()] =
        self_processing_time + inputs_processing_time;
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::INTERLEAVE_MANY);
    return absl::OkStatus();
  }
};

// The first input of AsyncInterleaveMany corresponds to the input dataset whose
// elements are used to create the (derived) input datasets whose elements are
// interleaved as output.
//
// TODO(jsimsa): model the first input
class AsyncInterleaveMany : public Node {
 public:
  AsyncInterleaveMany(Node::Args args,
                      std::vector<std::shared_ptr<Parameter>> parameters)
      : Node(args) {
    for (auto& parameter : parameters) {
      parameters_[parameter->name] = std::move(parameter);
    }
  }

  ~AsyncInterleaveMany() override = default;

  bool IsAsync() const override { return true; }

  // The ratio of an AsyncInterleaveMany node is 1/`cycle_length`. If cycle
  // length is not available, we use 1/parallelism.
  double Ratio() const override {
    auto* cycle_length = gtl::FindOrNull(parameters_, kCycleLength);
    if (cycle_length != nullptr) {
      return 1.0 / (*cycle_length)->value;
    }
    // After cl/436244658, `cycle_length` can not be `nullptr`. The remaining
    // part of this function is used to approximate `Ratio()` of this node for
    // model proto that was created before the CL.

    // Cycle length is not available, use 1/min(input_size, parallelism) as the
    // ratio.
    double parallelism = 1.0;
    {
      mutex_lock l(mu_);
      if (inputs_.size() >= 2) {
        auto first_input = inputs_.begin();
        auto second_input = std::next(first_input);
        // Some interleave datasets have 2 different inputs: the original input
        // dataset and the generated input datasets when interleave is iterated,
        // and some do not.
        if ((*first_input)->name() == (*second_input)->name()) {
          parallelism = std::max(inputs_.size(), size_t{1});
        } else {
          parallelism = std::max(inputs_.size() - 1, size_t{1});
        }
      }
    }
    auto* parameter = gtl::FindOrNull(parameters_, kParallelism);
    if (parameter) {
      parallelism = std::min(parallelism, (*parameter)->value);
    }
    return 1.0 / parallelism;
  }

  double ComputeSelfTime() const override {
    double parallelism = 1.0;
    auto* parallelism_parameter = gtl::FindOrNull(parameters_, kParallelism);
    if (parallelism_parameter) {
      parallelism = (*parallelism_parameter)->value;
    }
    if (num_elements_ == 0) {
      return 0;
    }
    {
      tf_shared_lock l(mu_);
      return processing_time_ema_ / parallelism;
    }
  }

 protected:
  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    std::vector<std::shared_ptr<Parameter>> parameters;
    for (auto& pair : parameters_) {
      parameters.push_back(pair.second);
    }
    return std::make_shared<AsyncInterleaveMany>(
        Args{id_, name_, std::move(output)}, parameters);
  }

  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }

    if (num_inputs() <= 1) {
      (*input_times)[long_name()] = inherited_input_time;
      return;
    }
    // Here `inherited_input_time + SelfProcessingTimeLocked()` is the average
    // input time for AsyncInterleaveMany node to call one of the `(num_inputs()
    // - 1)` input nodes (except first input) to return an element. Regardless
    // of the `block_length` parameter of AsyncInterleaveMany node, the average
    // input time for any of the `(num_inputs() - 1)` input nodes to be called
    // is computed as:
    double input_time = (inherited_input_time + SelfProcessingTimeLocked()) *
                        static_cast<double>(num_inputs() - 1);
    (*input_times)[long_name()] = input_time;
  }

  // The output time is the sum of self processing time and expected wait time
  // from the buffer model estimated using `ComputeWaitTime(producer_time,
  // consumer_time, parallelism, ...)`, where `producer_time` is the average
  // output time of inputs comprising the interleave "cycle" divided by
  // `parallelism`, `consumer_time` is the `input_time` specified through
  // `input_times` divided by `num_inputs() - 1`, and if the node has
  // parallelism parameter, then `buffer_size` is derived from `parallelism`.
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (num_inputs() <= 1) {
      (*output_times)[long_name()] = self_processing_time;
      if (gradients) {
        for (const auto& pair : CollectTunableParametersLocked()) {
          gradients->erase(std::make_pair(pair.first, pair.second->name));
        }
      }
      return;
    }

    double output_time, wait_time, consumer_time, producer_time;
    double input_time = input_times.at(long_name());
    consumer_time = input_time / static_cast<double>(num_inputs() - 1);
    double parallelism = num_inputs() - 1;  // default to cycle length
    auto* parameter = gtl::FindOrNull(parameters_, kParallelism);
    if (parameter) {
      parallelism = std::min(parallelism, (*parameter)->value);
    }
    double output_time_for_inputs =
        OutputTimeForInputs(*output_times) -
        (*output_times)[inputs_.front()->long_name()];
    producer_time = output_time_for_inputs /
                    static_cast<double>(num_inputs() - 1) / parallelism;

    if (gradients) {
      double producer_time_der = 0.0L;
      double consumer_time_der = 0.0L;
      double buffer_size_der = 0.0L;
      wait_time = ComputeWaitTime(producer_time, consumer_time, parallelism,
                                  &producer_time_der, &consumer_time_der,
                                  &buffer_size_der);
      double inputs_time_der_sum =
          OutputTimeGradientsForInputs(*output_time_gradients);
      (*output_time_gradients)[long_name()] =
          consumer_time_der +
          producer_time_der * inputs_time_der_sum / parallelism;

      for (const auto& pair : CollectTunableParametersLocked()) {
        auto* gradient = gtl::FindOrNull(
            *gradients, std::make_pair(pair.first, pair.second->name));
        if (gradient) {
          *gradient *= (producer_time_der /
                        static_cast<double>(num_inputs() - 1) / parallelism);
        }
      }

      // Set derivatives w.r.t. tunable parameters of the subtree rooted in the
      // first input equal to 0 since its output time is excluded from
      // computations.
      for (auto& pair : inputs_.front()->CollectTunableParameters()) {
        (*gradients)[std::make_pair(pair.first, pair.second->name)] = 0.0L;
      }
      // Add derivative w.r.t. own parallelism parameter.
      if (parameter && (*parameter)->state->tunable) {
        (*gradients)[std::make_pair(long_name(), (*parameter)->name)] =
            buffer_size_der - producer_time_der * producer_time / parallelism;
      }
    } else {
      wait_time = ComputeWaitTime(producer_time, consumer_time, parallelism,
                                  /*producer_time_derivative=*/nullptr,
                                  /*consumer_time_derivative=*/nullptr,
                                  /*buffer_size_derivative=*/nullptr);
    }
    output_time = self_processing_time + wait_time;
    (*output_times)[long_name()] = output_time;
  }

  // The processing time is the sum of the self processing time and the average
  // processing time of inputs comprising the interleave "cycle".
  void TotalProcessingTimeLocked(NodeValues* processing_times,
                                 NodeValues* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (processing_times) {
      (*processing_times)[long_name()] = self_processing_time;
    }
    if (num_inputs() <= 1) {
      (*total_processing_times)[long_name()] = self_processing_time;
      return;
    }
    double inputs_processing_time =
        (TotalProcessingTimeForInputs(*total_processing_times) -
         (*total_processing_times)[inputs_.front()->long_name()]) /
        static_cast<double>(num_inputs() - 1);
    (*total_processing_times)[long_name()] =
        self_processing_time + inputs_processing_time;
  }

  double MaximumBufferedBytes() const override TF_SHARED_LOCKS_REQUIRED(mu_) {
    auto* parameter = gtl::FindOrNull(parameters_, kMaxBufferedElements);
    if (parameter == nullptr) {
      parameter = gtl::FindOrNull(parameters_, kParallelism);
      if (parameter == nullptr) {
        return 0.0;
      }
    }
    return (*parameter)->value * AverageBufferedElementSizeLocked();
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::ASYNC_INTERLEAVE_MANY);
    return absl::OkStatus();
  }
};

class KnownRatio : public Node {
 public:
  KnownRatio(Node::Args args, double ratio) : Node(args), ratio_(ratio) {}

  ~KnownRatio() override = default;

  double Ratio() const override { return ratio_; }

 protected:
  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    return std::make_shared<KnownRatio>(Args{id_, name_, std::move(output)},
                                        ratio_);
  }

  // The input time is the sum of inherited input time and self processing time,
  // divided by `ratio_`.
  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }

    if (ratio_ == 0) {
      (*input_times)[long_name()] = inherited_input_time;
      return;
    }
    double input_time =
        (inherited_input_time + SelfProcessingTimeLocked()) / ratio_;
    (*input_times)[long_name()] = input_time;
  }

  // The output time is the sum of the self processing time and the product of
  // `ratio_` and the sum of output times of inputs.
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (ratio_ == 0) {
      (*output_times)[long_name()] = self_processing_time;
      if (gradients) {
        for (const auto& pair : CollectTunableParametersLocked()) {
          gradients->erase(std::make_pair(pair.first, pair.second->name));
        }
      }
      return;
    }
    if (gradients) {
      for (const auto& pair : CollectTunableParametersLocked()) {
        auto* gradient = gtl::FindOrNull(
            *gradients, std::make_pair(pair.first, pair.second->name));
        if (gradient) {
          *gradient *= ratio_;
        }
      }
      (*output_time_gradients)[long_name()] =
          OutputTimeGradientsForInputs(*output_time_gradients);
    }
    double inputs_output_time = ratio_ * OutputTimeForInputs(*output_times);
    (*output_times)[long_name()] = self_processing_time + inputs_output_time;
  }

  // The processing time is the sum of the self processing time and the product
  // of `ratio_` and the sum of processing times of inputs.
  void TotalProcessingTimeLocked(NodeValues* processing_times,
                                 NodeValues* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (processing_times) {
      (*processing_times)[long_name()] = self_processing_time;
    }
    if (ratio_ == 0) {
      (*total_processing_times)[long_name()] = self_processing_time;
      return;
    }
    double inputs_processing_time =
        ratio_ * TotalProcessingTimeForInputs(*total_processing_times);
    (*total_processing_times)[long_name()] =
        self_processing_time + inputs_processing_time;
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::KNOWN_RATIO);
    node_proto->set_ratio(ratio_);
    return absl::OkStatus();
  }

 private:
  const double ratio_;
};

class AsyncRatio : public Node {
 public:
  AsyncRatio(Node::Args args, double ratio, double memory_ratio,
             std::vector<std::shared_ptr<Parameter>> parameters,
             bool is_legacy_prefetch_autotuned = false)
      : Node(args),
        is_legacy_prefetch_autotuned_(is_legacy_prefetch_autotuned),
        ratio_(ratio),
        memory_ratio_(memory_ratio) {
    for (auto& parameter : parameters) {
      parameters_[parameter->name] = std::move(parameter);
    }
  }

  ~AsyncRatio() override = default;

  bool IsAsync() const override { return true; }

  double Ratio() const override { return ratio_; }

  double ComputeSelfTime() const override {
    double parallelism = 1.0;
    auto* parallelism_parameter = gtl::FindOrNull(parameters_, kParallelism);
    if (parallelism_parameter) {
      parallelism = (*parallelism_parameter)->value;
    }
    if (num_elements_ == 0) {
      return 0;
    }
    {
      tf_shared_lock l(mu_);
      return processing_time_ema_ / parallelism;
    }
  }

 protected:
  virtual double RatioLocked() const TF_SHARED_LOCKS_REQUIRED(mu_) {
    return ratio_;
  }

  double MemoryRatio() const { return memory_ratio_; }

  // The input time is the sum of inherited input time and parallelism adjusted
  // self processing time, divided by `Ratio()`.
  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }
    double parallelism = 1.0;
    auto* parallelism_parameter = gtl::FindOrNull(parameters_, kParallelism);
    if (parallelism_parameter) {
      parallelism = (*parallelism_parameter)->value;
    }

    auto ratio = RatioLocked();
    if (ratio == 0.0) {
      (*input_times)[long_name()] =
          inherited_input_time + SelfProcessingTimeLocked() / parallelism;
      return;
    }
    double input_time =
        (inherited_input_time + SelfProcessingTimeLocked() / parallelism) /
        ratio;
    (*input_times)[long_name()] = input_time;
  }

  // The output time is the sum of parallelism adjusted self processing time and
  // expected wait time from the buffer model estimated using
  // `ComputeWaitTime(producer_time, consumer_time, parallelism, ...)`, where
  // `producer_time` is the product of `Ratio()` and the sum of output times of
  // inputs, `consumer_time` is the product of `Ratio()` and the `input_time`
  // specified through `input_times` (since for each element stored in the
  // buffer, the inputs need to be called `Ratio()` times), and if the node has
  // parallelism parameter, then `buffer_size` is derived from `parallelism`.
  //
  // Current implementation assumes that there is at most 1 parameter per node.
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    auto ratio = RatioLocked();
    double parallelism = 1.0;
    double buffer_size = 0.0;
    auto* parallelism_parameter = gtl::FindOrNull(parameters_, kParallelism);
    auto* buffer_size_parameter = gtl::FindOrNull(parameters_, kBufferSize);
    if (parallelism_parameter) {
      parallelism = (*parallelism_parameter)->value;
      if (ratio == 0.0) {
        buffer_size = parallelism;
      } else {
        // Currently, MapAndBatch is the only transformation creates
        // AsyncKnownRatio nodes with ratio >= 1. For MapAndBatch, we create
        // `parallelism` threads to apply the function on elements from input
        // dataset, while one element in the buffer actually corresponds to
        // `Ratio()` elements from input dataset. So we adjust the `buffer_size`
        // by dividing `Ratio()`.
        buffer_size = parallelism / ratio;
      }
    } else if (buffer_size_parameter) {
      buffer_size = (*buffer_size_parameter)->value;
    }
    double self_processing_time = SelfProcessingTimeLocked();
    double output_time, wait_time, consumer_time, producer_time;
    double input_time = input_times.at(long_name());

    if (ratio == 0.0) {
      consumer_time = input_time;
      producer_time = 0.0L;
      if (gradients) {
        for (const auto& pair : CollectTunableParametersLocked()) {
          gradients->erase(std::make_pair(pair.first, pair.second->name));
        }

        double producer_time_der = 0.0L;
        double consumer_time_der = 0.0L;
        double buffer_size_der = 0.0L;
        wait_time = ComputeWaitTime(producer_time, consumer_time, buffer_size,
                                    &producer_time_der, &consumer_time_der,
                                    &buffer_size_der);
        (*output_time_gradients)[long_name()] = consumer_time_der;
        if (parallelism_parameter && (*parallelism_parameter)->state->tunable) {
          (*gradients)[std::make_pair(long_name(),
                                      (*parallelism_parameter)->name)] =
              -(1.0L + consumer_time_der) * self_processing_time /
                  Square(parallelism) +
              buffer_size_der;
        } else if (buffer_size_parameter &&
                   (*buffer_size_parameter)->state->tunable) {
          (*gradients)[std::make_pair(
              long_name(), (*buffer_size_parameter)->name)] = buffer_size_der;
        }
      } else {
        wait_time = ComputeWaitTime(producer_time, consumer_time, buffer_size,
                                    /*producer_time_derivative=*/nullptr,
                                    /*consumer_time_derivative=*/nullptr,
                                    /*buffer_size_derivative=*/nullptr);
      }
      output_time = self_processing_time / parallelism + wait_time;
      (*output_times)[long_name()] = output_time;
      return;
    }

    consumer_time = input_time * ratio;
    producer_time = ratio * OutputTimeForInputs(*output_times);
    if (gradients) {
      double producer_time_der = 0.0L;
      double consumer_time_der = 0.0L;
      double buffer_size_der = 0.0L;
      wait_time = ComputeWaitTime(producer_time, consumer_time, buffer_size,
                                  &producer_time_der, &consumer_time_der,
                                  &buffer_size_der);
      double inputs_time_der_sum =
          OutputTimeGradientsForInputs(*output_time_gradients);
      (*output_time_gradients)[long_name()] =
          consumer_time_der + producer_time_der * inputs_time_der_sum;

      for (const auto& pair : CollectTunableParametersLocked()) {
        auto* gradient = gtl::FindOrNull(
            *gradients, std::make_pair(pair.first, pair.second->name));
        if (gradient) {
          *gradient *= (ratio * producer_time_der);
        }
      }

      // Add derivative w.r.t. own parameter if it's tunable.
      if (parallelism_parameter && (*parallelism_parameter)->state->tunable) {
        (*gradients)[std::make_pair(long_name(),
                                    (*parallelism_parameter)->name)] =
            buffer_size_der / ratio -
            (1.0L + consumer_time_der +
             producer_time_der * inputs_time_der_sum) *
                self_processing_time / Square(parallelism);
      } else if (buffer_size_parameter &&
                 (*buffer_size_parameter)->state->tunable) {
        (*gradients)[std::make_pair(
            long_name(), (*buffer_size_parameter)->name)] = buffer_size_der;
      }
    } else {
      wait_time = ComputeWaitTime(producer_time, consumer_time, buffer_size,
                                  /*producer_time_derivative=*/nullptr,
                                  /*consumer_time_derivative=*/nullptr,
                                  /*buffer_size_derivative=*/nullptr);
    }
    output_time = self_processing_time / parallelism + wait_time;
    (*output_times)[long_name()] = output_time;
  }

  // The processing time is the sum of the self processing time and the product
  // of `Ratio()` and the sum of processing times of inputs.
  void TotalProcessingTimeLocked(NodeValues* processing_times,
                                 NodeValues* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (processing_times) {
      (*processing_times)[long_name()] = self_processing_time;
    }
    auto ratio = RatioLocked();
    if (ratio == 0) {
      (*total_processing_times)[long_name()] = self_processing_time;
      return;
    }
    double inputs_processing_time =
        ratio * TotalProcessingTimeForInputs(*total_processing_times);
    (*total_processing_times)[long_name()] =
        self_processing_time + inputs_processing_time;
  }

  double MaximumBufferedBytes() const override TF_SHARED_LOCKS_REQUIRED(mu_) {
    if (is_legacy_prefetch_autotuned_) {
      // This node's memory is managed by a legacy prefetch autotuner instead of
      // the model.
      return 0;
    }
    double result = 0;
    auto* parameter = gtl::FindOrNull(parameters_, kBufferSize);
    if (!parameter) {
      parameter = gtl::FindOrNull(parameters_, kParallelism);
    }

    if (parameter) {
      if (memory_ratio_ == 0) {
        result += (*parameter)->value * AverageBufferedElementSizeLocked();
      } else {
        // The estimation is currently not accurate for MapAndBatchDataset for
        // the maximum buffer size does not match `num_parallel_calls`
        // parameter.
        result += (*parameter)->value * AverageBufferedElementSizeLocked() /
                  memory_ratio_;
      }
    }
    return result;
  }

  // Whether this node represents a prefetch node tuned by the legacy prefetch
  // autotuner, rather than the model.
  const bool is_legacy_prefetch_autotuned_;

 private:
  // Identifies how many input elements need to be created to construct an
  // element for the dataset.
  //
  // Currently the value is 1 for PrefetchDataset and ParallelMapDataset,
  // batch_size for MapAndBatchDataset and ParallelBatchDataset.
  const double ratio_;
  // For parallelism nodes, identifies how many parallelism calls are introduced
  // by one buffered element. The value is defined to correctly estimate RAM
  // budget bound with given num_parallel_calls (or buffer_size) combined with
  // the estimated average size of buffered elements.
  const double memory_ratio_;
};

class UnknownRatio : public Node {
 public:
  using Node::Node;

  ~UnknownRatio() override = default;

  double Ratio() const override {
    tf_shared_lock l(mu_);
    return RatioLocked();
  }

 protected:
  double RatioLocked() const TF_SHARED_LOCKS_REQUIRED(mu_) {
    // TODO(wilsin): Consistent with UnknownRatio, current implementation
    // assumes that the number of input elements consumed per output is the same
    // across all inputs.
    if (num_elements_ == 0 || inputs_.empty() ||
        inputs_.front()->num_elements() == 0) {
      return 0.0;
    }
    return static_cast<double>(inputs_.front()->num_elements()) /
           static_cast<double>(num_elements_);
  }

  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    return std::make_shared<UnknownRatio>(Args{id_, name_, std::move(output)});
  }

  // The input time is the sum of inherited input time and self processing time,
  // divided by the ratio estimate.
  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }

    if (num_elements_ == 0 || inputs_.empty() ||
        inputs_.front()->num_elements() == 0) {
      (*input_times)[long_name()] = inherited_input_time;
      return;
    }
    std::shared_ptr<Node> input = inputs_.front();
    double ratio = static_cast<double>(input->num_elements()) /
                   static_cast<double>(num_elements_);
    double input_time =
        (inherited_input_time + SelfProcessingTimeLocked()) / ratio;
    (*input_times)[long_name()] = input_time;
  }

  // The output time is the sum of the self processing time and the product of
  // the ratio estimate and the sum of output times of inputs.
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (num_elements_ == 0 || inputs_.empty() ||
        inputs_.front()->num_elements() == 0) {
      (*output_times)[long_name()] = self_processing_time;
      if (gradients) {
        for (const auto& pair : CollectTunableParametersLocked()) {
          gradients->erase(std::make_pair(pair.first, pair.second->name));
        }
      }
      return;
    }
    // TODO(jsimsa): The current implementation assumes that the number of input
    // elements consumed per output is the same across all inputs.
    double ratio = static_cast<double>(inputs_.front()->num_elements()) /
                   static_cast<double>(num_elements_);
    if (gradients) {
      for (const auto& pair : CollectTunableParametersLocked()) {
        auto* gradient = gtl::FindOrNull(
            *gradients, std::make_pair(pair.first, pair.second->name));
        if (gradient) {
          *gradient *= ratio;
        }
      }
      (*output_time_gradients)[long_name()] =
          OutputTimeGradientsForInputs(*output_time_gradients);
    }
    double inputs_output_time = ratio * OutputTimeForInputs(*output_times);
    (*output_times)[long_name()] = self_processing_time + inputs_output_time;
  }

  // The processing time is the sum of the self processing time and the product
  // of the ratio estimate and the sum of processing times of inputs.
  void TotalProcessingTimeLocked(
      absl::flat_hash_map<std::string, double>* processing_times,
      absl::flat_hash_map<std::string, double>* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double self_processing_time = SelfProcessingTimeLocked();
    if (processing_times) {
      (*processing_times)[long_name()] = self_processing_time;
    }
    if (inputs_.empty() || num_elements_ == 0) {
      (*total_processing_times)[long_name()] = self_processing_time;
      return;
    }
    // TODO(jsimsa): The current implementation assumes that the number of input
    // elements consumed per output is the same across all inputs.
    std::shared_ptr<Node> input = inputs_.front();
    double ratio = static_cast<double>(input->num_elements()) /
                   static_cast<double>(num_elements_);
    double inputs_processing_time =
        ratio * TotalProcessingTimeForInputs(*total_processing_times);
    (*total_processing_times)[long_name()] =
        self_processing_time + inputs_processing_time;
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::UNKNOWN_RATIO);
    return absl::OkStatus();
  }
};

class Unknown : public Node {
 public:
  using Node::Node;

  ~Unknown() override = default;

 protected:
  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    return std::make_shared<Unknown>(Args{id_, name_, std::move(output)});
  }

  // The input time is the inherited input time.
  void InputTimeLocked(NodeValues* input_times) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    double inherited_input_time;
    if (output_) {
      inherited_input_time = (*input_times)[output_->long_name()];
    } else {
      inherited_input_time = (*input_times)[kModelInputTimeKey];
    }
    (*input_times)[long_name()] = inherited_input_time;
  }

  // The output time is the sum of output times of inputs.
  void OutputTimeLocked(const NodeValues& input_times,
                        ParameterGradients* gradients, NodeValues* output_times,
                        NodeValues* output_time_gradients) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    (*output_times)[long_name()] = OutputTimeForInputs(*output_times);
    if (gradients) {
      (*output_time_gradients)[long_name()] =
          OutputTimeGradientsForInputs(*output_time_gradients);
    }
  }

  // The processing time is the sum of processing times of inputs.
  void TotalProcessingTimeLocked(NodeValues* processing_times,
                                 NodeValues* total_processing_times) override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    if (processing_times) {
      (*processing_times)[long_name()] = SelfProcessingTimeLocked();
    }
    (*total_processing_times)[long_name()] =
        TotalProcessingTimeForInputs(*total_processing_times);
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::UNKNOWN);
    return absl::OkStatus();
  }
};

class AsyncKnownRatio : public AsyncRatio {
 public:
  AsyncKnownRatio(Node::Args args, double ratio, double memory_ratio,
                  std::vector<std::shared_ptr<Parameter>> parameters,
                  bool is_legacy_prefetch_autotuned = false)
      : AsyncRatio(args, ratio, memory_ratio, parameters,
                   is_legacy_prefetch_autotuned) {}

  ~AsyncKnownRatio() override = default;

 protected:
  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    std::vector<std::shared_ptr<Parameter>> parameters;
    for (auto& pair : parameters_) {
      parameters.push_back(pair.second);
    }
    return std::make_shared<AsyncKnownRatio>(
        Args{id_, name_, std::move(output)}, Ratio(), MemoryRatio(), parameters,
        is_legacy_prefetch_autotuned_);
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::ASYNC_KNOWN_RATIO);
    node_proto->set_ratio(Ratio());
    node_proto->set_memory_ratio(MemoryRatio());
    if (is_legacy_prefetch_autotuned_) {
      // Update buffer_size parameter to make sense from a user perspective.
      if (node_proto->parameters_size() != 1) {
        return absl::InternalError(absl::StrCat(
            "Expected prefetch node to have one parameter, but it has ",
            node_proto->parameters_size()));
      }
      auto* parameter = node_proto->mutable_parameters(0);
      // Legacy autotuner only modifies the state_value, not the model value.
      parameter->set_value(parameter->state_value());
      parameter->set_tunable(true);
    }
    return absl::OkStatus();
  }
};

class AsyncUnknownRatio : public AsyncRatio {
 public:
  AsyncUnknownRatio(Node::Args args,
                    std::vector<std::shared_ptr<Parameter>> parameters)
      : AsyncRatio(args, /*ratio=*/0.0, /*memory_ratio=*/0.0, parameters) {}

  ~AsyncUnknownRatio() override = default;

  double Ratio() const override {
    tf_shared_lock l(mu_);
    return RatioLocked();
  }

 protected:
  double RatioLocked() const TF_SHARED_LOCKS_REQUIRED(mu_) override {
    // TODO(wilsin): Consistent with UnknownRatio, current implementation
    // assumes that the number of input elements consumed per output is the same
    // across all inputs.
    if (num_elements_ == 0 || inputs_.empty() ||
        inputs_.front()->num_elements() == 0) {
      return 0.0;
    }
    return static_cast<double>(inputs_.front()->num_elements()) /
           static_cast<double>(num_elements_);
  }

  std::shared_ptr<Node> Clone(std::shared_ptr<Node> output) const override
      TF_SHARED_LOCKS_REQUIRED(mu_) {
    std::vector<std::shared_ptr<Parameter>> parameters;
    for (auto& pair : parameters_) {
      parameters.push_back(pair.second);
    }
    return std::make_shared<AsyncUnknownRatio>(
        Args{id_, name_, std::move(output)}, parameters);
  }

  absl::Status ToProto(ModelProto::Node* node_proto) const override {
    TF_RETURN_IF_ERROR(Node::ToProto(node_proto));
    node_proto->set_node_class(NodeClass::ASYNC_UNKNOWN_RATIO);
    return absl::OkStatus();
  }
};

}  // namespace

thread_local int64_t Node::work_start_;

std::shared_ptr<Parameter> MakeParameter(const std::string& name,
                                         std::shared_ptr<SharedState> state,
                                         double min, double max) {
  return std::make_shared<Parameter>(name, state, min, max);
}

std::shared_ptr<Parameter> MakeParameter(const std::string& name,
                                         std::shared_ptr<SharedState> state,
                                         double min, double max, double value) {
  std::shared_ptr<Parameter> parameter =
      std::make_shared<Parameter>(name, state, min, max);
  parameter->value = value;
  return parameter;
}

std::shared_ptr<Parameter> MakeNonTunableParameter(const std::string& name,
                                                   double value) {
  return std::make_shared<Parameter>(name, nullptr, /*min=*/value,
                                     /*max=*/value);
}

std::shared_ptr<Node> MakeInterleaveManyNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters) {
  DCHECK(absl::c_any_of(parameters,
                        [](const std::shared_ptr<Parameter>& parameter) {
                          return parameter->name == kCycleLength;
                        }));
  return std::make_shared<InterleaveMany>(std::move(args),
                                          std::move(parameters));
}

std::shared_ptr<Node> MakeAsyncInterleaveManyNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters) {
  DCHECK(absl::c_any_of(parameters,
                        [](const std::shared_ptr<Parameter>& parameter) {
                          return parameter->name == kCycleLength;
                        }));
  return std::make_shared<AsyncInterleaveMany>(std::move(args),
                                               std::move(parameters));
}

std::shared_ptr<Node> MakeKnownRatioNode(Node::Args args, double ratio) {
  return std::make_shared<KnownRatio>(std::move(args), ratio);
}

std::shared_ptr<Node> MakeAsyncKnownRatioNode(
    Node::Args args, double ratio, double memory_ratio,
    std::vector<std::shared_ptr<Parameter>> parameters,
    bool is_legacy_prefetch_autotuned) {
  return std::make_shared<AsyncKnownRatio>(std::move(args), ratio, memory_ratio,
                                           std::move(parameters),
                                           is_legacy_prefetch_autotuned);
}

std::shared_ptr<Node> MakeAsyncKnownRatioNode(
    Node::Args args, double ratio,
    std::vector<std::shared_ptr<Parameter>> parameters,
    bool is_legacy_prefetch_autotuned,
    std::optional<int64_t> estimated_element_size) {
  auto node =
      MakeAsyncKnownRatioNode(std::move(args), /*ratio=*/ratio,
                              /*memory_ratio=*/ratio, std::move(parameters),
                              is_legacy_prefetch_autotuned);
  node->SetEstimatedElementSize(estimated_element_size);
  return node;
}

std::shared_ptr<Node> MakeSourceNode(Node::Args args) {
  return MakeKnownRatioNode(std::move(args), 0);
}

std::shared_ptr<Node> MakeUnknownRatioNode(Node::Args args) {
  return std::make_shared<UnknownRatio>(std::move(args));
}

std::shared_ptr<Node> MakeAsyncUnknownRatioNode(
    Node::Args args, std::vector<std::shared_ptr<Parameter>> parameters) {
  return std::make_shared<AsyncUnknownRatio>(std::move(args),
                                             std::move(parameters));
}

std::shared_ptr<Node> MakeUnknownNode(Node::Args args) {
  return std::make_shared<Unknown>(std::move(args));
}

double Node::ComputeWaitTime(const double producer_time,
                             const double consumer_time,
                             const double buffer_size,
                             double* producer_time_derivative,
                             double* consumer_time_derivative,
                             double* buffer_size_derivative) {
  // If we set x=`consumer_time`, y=`producer_time`, n=`buffer_size`,
  // p=`p_buffer_empty`, T=`wait_time`, then we have:
  // if y = 0, then p = 0;
  // elif x = 0, then p = 1;
  // elif x = y, then p = 1 / (n+1);
  // else p = [1 - x/y] / [1 - power(x/y, n+1)].
  //
  // We also have T = p * y, and derivatives of T w.r.t. x, y, n are computed:
  // dT/dx = dp/dx * y,
  // dT/dy = p + dp/dy * y,
  // dT/dn = dp/dn * y.
  // Then the remaining work is to compute dp/dx, dp/dy, dp/dn by considering
  // different cases and substitute the values into above formulas.

  // Case 1: if producer is infinitely fast. The buffer will always be full.
  // Wait time will always be 0.
  if (producer_time == 0) {
    if (producer_time_derivative) {
      // Note a common error is `*producer_time_derivative = 0` since p=0 on the
      // line y=0 doesn't imply dp/dy = 0 there. Actually to compute dp/dy at
      // (x,0), we need to consider lim_{dy->0+} [p(x,dy)-p(x,0)] / dy, where
      // p(x,0)=0 and p(x,dy) = [1 - x/dy] / [1 - power(x/dy, n+1)].
      if (buffer_size == 0 || consumer_time == 0) {
        *producer_time_derivative = 1.0L;
      } else {
        *producer_time_derivative = 0.0L;
      }
    }
    if (consumer_time_derivative) {
      *consumer_time_derivative = 0.0L;
    }
    if (buffer_size_derivative) {
      *buffer_size_derivative = 0.0L;
    }
    return 0.0L;
  }

  // Case 2: if consumer is infinitely fast. Wait time is always the time to
  // produce an output.
  if (consumer_time == 0) {
    if (producer_time_derivative) {
      *producer_time_derivative = 1.0L;
    }
    if (consumer_time_derivative) {
      // Note a common error is `*consumer_time_derivative = 0` since p=1 on the
      // line x=0 doesn't imply dp/dx = 0 there. Actually to compute dp/dx at
      // (0,y), we need to consider lim_{dx->0+} [p(dx,y)-p(0,y)] / dx, where
      // p(0,y)=1, p(dx,y) = [1 - dx/y] / [1 - power(dx/y, n+1)] if y!=0.
      if (buffer_size == 0) {
        *consumer_time_derivative = 0.0L;
      } else {
        *consumer_time_derivative = -1.0L;
      }
    }
    if (buffer_size_derivative) {
      *buffer_size_derivative = 0.0L;
    }
    return producer_time;
  }

  // Case 3: the consumer and the producer are equally fast. Expected wait time
  // decreases linearly with the size of the buffer.
  if (consumer_time == producer_time) {
    const double p_buffer_empty = 1.0L / (buffer_size + 1.0L);
    const double p_buffer_empty_der =
        -buffer_size / (2.0L * buffer_size + 2.0L);
    if (producer_time_derivative) {
      // Note a common error is `*producer_time_derivative = p_buffer_empty`
      // since p=1/(n+1) on the line x=y doesn't imply dp/dy = 0 there. Actually
      // to compute dp/dy at (y,y), we need to consider lim_{dy->0}
      // [p(y,y+dy)-p(y,y)] / dy, where p(y,y)=1/(n+1), p(y,y+dy) = [1 -
      // y/(y+dy)] / [1 - power(y/(y+dy), n+1)].
      *producer_time_derivative = p_buffer_empty - p_buffer_empty_der;
    }
    if (consumer_time_derivative) {
      // Note a common error is `*consumer_time_derivative = 0` since p=1/(n+1)
      // on the line x=y doesn't imply dp/dx = 0 there. Actually to compute
      // dp/dx at (x,x), we need to consider lim_{dx->0} [p(x+dx,x)-p(x,x)] /
      // dx, where p(x,x)=1/(n+1), p(x+dx,x) = [1 - (x+dx)/x] / [1 -
      // power((x+dx)/x, n+1)].
      *consumer_time_derivative = p_buffer_empty_der;
    }
    if (buffer_size_derivative) {
      *buffer_size_derivative = -producer_time / Square(buffer_size + 1.0L);
    }
    return p_buffer_empty * producer_time;
  }

  // Case 4: the consumer is slower than the producer and neither is infinitely
  // fast. Case 4 and Case 5 actually follow same formula. Separate them for
  // numerical computation reasons.
  if (consumer_time > producer_time) {
    const double ratio = producer_time / consumer_time;
    const double ratio_pow = std::pow(ratio, buffer_size);
    const double p_buffer_empty =
        ratio_pow * (1.0L - ratio) / (1.0L - ratio * ratio_pow);
    const double p_buffer_empty_der =
        (buffer_size - (buffer_size + 1.0L) * ratio + ratio_pow * ratio) *
        ratio_pow / ratio / Square(1.0L - ratio_pow * ratio);
    if (producer_time_derivative) {
      *producer_time_derivative = p_buffer_empty + p_buffer_empty_der * ratio;
    }
    if (consumer_time_derivative) {
      *consumer_time_derivative = -p_buffer_empty_der * Square(ratio);
    }
    if (buffer_size_derivative) {
      *buffer_size_derivative = p_buffer_empty / (1.0L - ratio_pow * ratio) *
                                std::log(ratio) * producer_time;
    }
    return p_buffer_empty * producer_time;
  }

  // Case 5: the producer is slower than the consumer and neither is infinitely
  // fast.
  const double ratio = consumer_time / producer_time;
  const double ratio_pow = std::pow(ratio, buffer_size);
  const double p_buffer_empty = (1.0L - ratio) / (1.0L - ratio_pow * ratio);
  const double p_buffer_empty_der =
      ((buffer_size + 1.0L - buffer_size * ratio) * ratio_pow - 1.0L) /
      Square(1.0L - ratio_pow * ratio);
  if (producer_time_derivative) {
    *producer_time_derivative = p_buffer_empty - p_buffer_empty_der * ratio;
  }
  if (consumer_time_derivative) {
    *consumer_time_derivative = p_buffer_empty_der;
  }
  if (buffer_size_derivative) {
    *buffer_size_derivative = p_buffer_empty / (1.0L - ratio_pow * ratio) *
                              ratio_pow * ratio * std::log(ratio) *
                              producer_time;
  }
  return p_buffer_empty * producer_time;
}

Node::ModelParameters Node::CollectTunableParametersLocked() const {
  Node::ModelParameters parameters;
  // Collect tunable parameters from the leaves of the nodes tree to the root.
  for (const auto& node :
       CollectNodesLocked(TraversalOrder::REVERSE_BFS, IsAutotuneNode)) {
    tf_shared_lock l(node->mu_);
    node->CollectTunableParametersHelper(&parameters);
  }
  CollectTunableParametersHelper(&parameters);
  return parameters;
}

Node::ModelParameters Node::CollectTunableParameters() const {
  tf_shared_lock l(mu_);
  return CollectTunableParametersLocked();
}

Node::ModelParameters Node::CollectNodeTunableParameters() const {
  tf_shared_lock l(mu_);
  Node::ModelParameters parameters;
  CollectTunableParametersHelper(&parameters);
  return parameters;
}

std::string Node::DebugString() const {
  absl::flat_hash_map<std::string, std::string> debug_strings;
  tf_shared_lock l(mu_);
  // Build up the debug string from the leaves of the nodes tree to the root.
  for (const auto& node :
       CollectNodesLocked(TraversalOrder::REVERSE_BFS, IsAnyNode)) {
    tf_shared_lock l(node->mu_);
    node->DebugStringHelper(&debug_strings);
  }
  DebugStringHelper(&debug_strings);

  return debug_strings[long_name()];
}

void Node::FlushMetrics() {
  if (!record_metrics_) {
    return;
  }
  metrics_.record_bytes_consumed(bytes_consumed_);
  metrics_.record_bytes_produced(bytes_produced_);
  metrics_.record_num_elements(num_elements_);
}

double Node::OutputTime(Node::NodeValues* input_times,
                        Node::ParameterGradients* gradients) const {
  // To store the output time gradient w.r.t. input time (if `gradients` is not
  // `nullptr`) and the output time for each node.
  Node::NodeValues output_time_gradients, output_times;
  tf_shared_lock l(mu_);
  auto nodes = CollectNodesLocked(TraversalOrder::BFS, IsAutotuneNode);

  // Computes and stores input time for each node from the root to leaves of the
  // nodes tree.
  InputTimeLocked(input_times);
  for (const auto& node : nodes) {
    tf_shared_lock l(node->mu_);
    node->InputTimeLocked(input_times);
  }

  std::reverse(nodes.begin(), nodes.end());
  // Computes and stores the output time and output time gradient w.r.t. input
  // time (if `gradients` is not `nullptr`) for each node from leaves of the
  // nodes tree to the root.
  for (const auto& node : nodes) {
    tf_shared_lock l(node->mu_);
    node->OutputTimeLocked(*input_times, gradients, &output_times,
                           &output_time_gradients);
  }
  OutputTimeLocked(*input_times, gradients, &output_times,
                   &output_time_gradients);

  return output_times[long_name()];
}

double Node::ComputeSelfTime() const {
  if (num_elements_ == 0) {
    return 0;
  }
  tf_shared_lock l(mu_);
  return processing_time_ema_;
}

std::shared_ptr<Node> Node::Snapshot() const {
  NodePairList node_pairs;
  auto result = SnapshotHelper(nullptr, &node_pairs);

  while (!node_pairs.empty()) {
    auto node_pair = node_pairs.front();
    node_pairs.pop_front();
    std::shared_ptr<Node> current = node_pair.first,
                          cloned_output = node_pair.second;
    cloned_output->add_input(
        current->SnapshotHelper(cloned_output, &node_pairs));
  }
  return result;
}

double Node::SelfProcessingTime() const {
  tf_shared_lock l(mu_);
  return SelfProcessingTimeLocked();
}

double Node::TotalBufferedBytes() const {
  Node::NodeValues total_bytes;
  tf_shared_lock l(mu_);
  // Compute total buffered bytes from the leaves of the nodes tree to the root.
  for (const auto& node :
       CollectNodesLocked(TraversalOrder::REVERSE_BFS, IsAnyNode)) {
    tf_shared_lock l(node->mu_);
    node->TotalBufferedBytesHelper(&total_bytes);
  }
  TotalBufferedBytesHelper(&total_bytes);

  return total_bytes[long_name()];
}

double Node::TotalMaximumBufferedBytes() const {
  Node::NodeValues total_bytes;
  tf_shared_lock l(mu_);
  // Compute total maximum buffered bytes from the leaves of the nodes tree to
  // the root.
  for (const auto& node :
       CollectNodesLocked(TraversalOrder::REVERSE_BFS, IsAnyNode)) {
    tf_shared_lock l(node->mu_);
    node->TotalMaximumBufferedBytesHelper(&total_bytes);
  }
  TotalMaximumBufferedBytesHelper(&total_bytes);

  return total_bytes[long_name()];
}

double Node::TotalProcessingTime(Node::NodeValues* processing_times) {
  // Create a hash map to store the per-element CPU time spent in the subtree
  // rooted in each node.
  Node::NodeValues total_processing_times;
  tf_shared_lock l(mu_);

  // Computes per-element CPU time spent in the subtree rooted in the node from
  // the leaves of the nodes tree to the root.
  for (const auto& node :
       CollectNodesLocked(TraversalOrder::REVERSE_BFS, IsAutotuneNode)) {
    tf_shared_lock l(node->mu_);
    node->TotalProcessingTimeLocked(processing_times, &total_processing_times);
  }
  TotalProcessingTimeLocked(processing_times, &total_processing_times);

  return total_processing_times[long_name()];
}

double Node::AverageBufferedElementSizeLocked() const {
  DCHECK_GE(num_elements_, 0);
  DCHECK_GE(buffered_elements_, 0);
  if (num_elements_ <= 0) {
    if (buffered_elements_ <= 0) {
      if (estimated_element_size_) {
        return *estimated_element_size_;
      }
      // If there are no produced elements or buffered elements recorded, return
      // 0.
      return 0;
    }
    // If there are no produced elements but some buffered elements, return the
    // average size of all buffered elements.
    return static_cast<double>(buffered_bytes_) /
           static_cast<double>(buffered_elements_);
  }

  if (buffered_elements_ <= 0) {
    // If there are no buffered elements but some produced elements, return the
    // average size of all produced elements.
    return static_cast<double>(bytes_produced_) /
           static_cast<double>(num_elements_);
  }

  // Otherwise, return the mean value of average size of all produced elements
  // and average size of all buffered elements.
  return (static_cast<double>(bytes_produced_) /
              static_cast<double>(num_elements_) +
          static_cast<double>(buffered_bytes_) /
              static_cast<double>(buffered_elements_)) /
         2.0;
}

double Node::OutputTimeForInputs(const Node::NodeValues& output_times) const {
  double sum = 0;
  for (auto& input : inputs_) {
    // Inputs for which autotuning is disabled are excluded.
    if (input->autotune()) {
      sum += output_times.at(input->long_name());
    }
  }
  return sum;
}

double Node::OutputTimeGradientsForInputs(
    const Node::NodeValues& output_time_gradients) const {
  double sum = 0;
  for (auto& input : inputs_) {
    // Inputs for which autotuning is disabled are excluded.
    if (input->autotune()) {
      sum +=
          gtl::FindWithDefault(output_time_gradients, input->long_name(), 0.0L);
    }
  }
  return sum;
}

double Node::TotalProcessingTimeForInputs(
    const Node::NodeValues& total_processing_times) {
  // If the number of elements produced by an input is smaller than this
  // constant, then its processing time is estimated using a weighted average of
  // the empirical processing time and processing time history.
  constexpr int kNumElementsThreshold = 30;

  // Identifies the minimum number of input processing times to collect before
  // the processing time history is used as a prior.
  constexpr int kCountThreshold = 30;

  double sum = 0;
  for (auto& input : inputs_) {
    // Inputs for which autotuning is disabled are excluded.
    if (input->autotune()) {
      double input_processing_time =
          total_processing_times.at(input->long_name());
      int64_t num_elements = input->num_elements();
      if (num_elements < kNumElementsThreshold) {
        if (input_processing_time_count_ < kCountThreshold) {
          sum += input_processing_time;
        } else {
          // The fewer elements the input has produced so far, the more weight
          // is assigned to the prior to reduce volatility.
          double prior_weight = 1.0L / static_cast<double>(2 << num_elements);
          double prior =
              input_processing_time_sum_ / input_processing_time_count_;
          sum += (1.0L - prior_weight) * input_processing_time +
                 prior_weight * prior;
        }
      } else {
        sum += input_processing_time;
        input_processing_time_count_++;
        input_processing_time_sum_ += input_processing_time;
      }
    }
  }
  return sum;
}

double Node::SelfProcessingTimeLocked() const {
  if (num_elements_ == 0) {
    return 0;
  }
  return static_cast<double>(processing_time_) /
         static_cast<double>(num_elements_);
}

Node::NodeVector Node::CollectNodes(
    TraversalOrder order,
    bool collect_node(const std::shared_ptr<Node>)) const {
  tf_shared_lock l(mu_);
  return CollectNodesLocked(order, collect_node);
}

bool Node::TryDownsizeBuffer() {
  if (!IsAsync()) {
    return false;
  }
  Node::ModelParameters tunable_parameters;
  {
    tf_shared_lock l(mu_);
    if (buffered_elements_low_ > buffered_elements_high_) {
      // No element is stored in the buffer yet. Do nothing.
      return false;
    }
    CollectTunableParametersHelper(&tunable_parameters);
  }
  Node::ModelParameters buffer_size_parameters;
  for (auto& parameter : tunable_parameters) {
    if (parameter.second->name != kBufferSize) {
      continue;
    }
    buffer_size_parameters.push_back(std::move(parameter));
  }
  bool downsized = false;
  // Sync buffer state values to parameter values
  for (auto& [node_name, parameter] : buffer_size_parameters) {
    tf_shared_lock l(*parameter->state->mu);
    parameter->value = parameter->state->value;
  }
  {
    // Downsize buffers
    tf_shared_lock l(mu_);
    for (auto& [node_name, parameter] : buffer_size_parameters) {
      if (buffered_elements_low_ > 0 &&
          (buffered_elements_high_ - buffered_elements_low_ + 1) <
              parameter->value) {
        double old_value = parameter->value;
        // By default, we increase the buffer sizes by `kBufferUpsizeMultiplier`
        // if there is enough RAM in upsize. We cap the downsize by
        // `kBufferDownsizeMultipliter` of the current size.
        parameter->value = std::max(
            buffered_elements_high_ - buffered_elements_low_ + 1,
            static_cast<int64_t>(old_value * kBufferDownsizeMultipliter));
        if (old_value != parameter->value) {
          VLOG(2) << "Downsize buffer " << long_name()
                  << "::" << parameter->name << " from " << old_value << " to "
                  << parameter->value;
          downsized = true;
        }
      }
    }
  }
  // Since SharedState locks are the same as the Ops iterator locks, locking of
  // the SharedState locks should be minimized in the optimization thread.
  if (downsized) {
    UpdateStateValues(&buffer_size_parameters);
  }
  return downsized;
}

void Node::CollectBufferParametersToUpsize(
    absl::flat_hash_map<Node*, Parameter*>& node_parameters) {
  {
    tf_shared_lock l(mu_);
    for (auto& [node_name, parameter] : parameters_) {
      if ((parameter->name != kBufferSize) ||
          (parameter->state == nullptr || !parameter->state->tunable)) {
        continue;
      }
      if (buffered_elements_low_ <= kBufferLowWatermarkThreshold &&
          buffered_elements_high_ >= parameter->value) {
        parameter->value = parameter->state->value;
        node_parameters[this] = parameter.get();
      }
    }
  }
  for (auto& [node, parameter] : node_parameters) {
    tf_shared_lock l(*parameter->state->mu);
    parameter->value = parameter->state->value;
  }
}

void Node::SyncStateValuesToParameterValues(const std::string& parameter_name) {
  // We need to first collect the parameters because `parameter->state->mu` must
  // be locked before the node mutex `mu_`;
  std::vector<Parameter*> parameters;
  {
    tf_shared_lock l(mu_);
    for (auto& [node_name, parameter] : parameters_) {
      if ((parameter->state == nullptr) ||
          (parameter->name != parameter_name)) {
        continue;
      }
      parameters.push_back(parameter.get());
    }
  }
  for (auto& parameter : parameters) {
    tf_shared_lock l(*parameter->state->mu);
    parameter->value = parameter->state->value;
  }
}

Node::NodeVector Node::CollectNodesLocked(
    TraversalOrder order, bool collect_node(const std::shared_ptr<Node>)) const
    TF_SHARED_LOCKS_REQUIRED(mu_) {
  NodeVector node_vector;
  std::list<std::shared_ptr<Node>> temp_list;

  for (auto& input : inputs_) {
    if (collect_node(input)) {
      node_vector.push_back(input);
      temp_list.push_back(input);
    }
  }

  while (!temp_list.empty()) {
    auto cur_node = temp_list.front();
    temp_list.pop_front();
    tf_shared_lock l(cur_node->mu_);
    for (auto& input : cur_node->inputs_) {
      if (collect_node(input)) {
        node_vector.push_back(input);
        temp_list.push_back(input);
      }
    }
  }

  if (order == TraversalOrder::REVERSE_BFS) {
    std::reverse(node_vector.begin(), node_vector.end());
  }
  return node_vector;
}

void Node::CollectTunableParametersHelper(
    Node::ModelParameters* parameters) const TF_SHARED_LOCKS_REQUIRED(mu_) {
  // If autotune is turned off or there are no elements recorded, we don't
  // collect the parameters on the node.
  if (!autotune_ || num_elements_ <= 0) {
    return;
  }
  for (auto& pair : parameters_) {
    if (pair.second->state != nullptr && pair.second->state->tunable) {
      parameters->push_back(std::make_pair(long_name(), pair.second));
    }
  }
}

void Node::DebugStringHelper(
    absl::flat_hash_map<std::string, std::string>* debug_strings) const
    TF_SHARED_LOCKS_REQUIRED(mu_) {
  std::string result;
  absl::StrAppend(&result, long_name(), ":\n");
  absl::StrAppend(&result, "  autotune=", autotune_.load(), "\n");
  absl::StrAppend(&result, "  buffered_bytes=", buffered_bytes_.load(), "\n");
  absl::StrAppend(&result, "  buffered_elements=", buffered_elements_.load(),
                  "\n");
  absl::StrAppend(&result, "  bytes_consumed=", bytes_consumed_.load(), "\n");
  absl::StrAppend(&result, "  bytes_produced=", bytes_produced_.load(), "\n");
  absl::StrAppend(&result, "  processing_time=", processing_time_.load(), "\n");
  absl::StrAppend(&result, "  num_elements=", num_elements_.load(), "\n");
  std::string inputs;
  for (auto& input : inputs_) {
    absl::StrAppend(&inputs, input->long_name(), ",");
  }
  absl::StrAppend(&result, "  inputs={", inputs, "}\n");
  for (auto& input : inputs_) {
    absl::StrAppend(&result, debug_strings->at(input->long_name()));
  }
  debug_strings->insert(std::make_pair(long_name(), result));
}

std::shared_ptr<Node> Node::SnapshotHelper(
    std::shared_ptr<Node> cloned_output, Node::NodePairList* node_pairs) const {
  tf_shared_lock l(mu_);

  // Clone current node(`this`), also set clone of its output node
  // (`cloned_output`) to be the output node of the cloned node
  // (`cloned_current`).
  std::shared_ptr<Node> cloned_current = Clone(cloned_output);
  {
    cloned_current->autotune_.store(autotune_);
    cloned_current->buffered_bytes_.store(buffered_bytes_);
    cloned_current->buffered_elements_.store(buffered_elements_);
    cloned_current->buffered_elements_low_.store(buffered_elements_low_);
    cloned_current->buffered_elements_high_.store(buffered_elements_high_);
    cloned_current->bytes_consumed_.store(bytes_consumed_);
    cloned_current->bytes_produced_.store(bytes_produced_);
    cloned_current->num_elements_.store(num_elements_);
    cloned_current->record_metrics_.store(false);
    cloned_current->processing_time_.store(processing_time_);
    {
      mutex_lock l2(cloned_current->mu_);
      cloned_current->parameters_ =
          absl::flat_hash_map<std::string, std::shared_ptr<Parameter>>();
      for (const auto& [parameter_name, parameter_ptr] : parameters_) {
        cloned_current->parameters_[parameter_name] =
            std::make_shared<Parameter>(parameter_ptr);
      }
      cloned_current->estimated_element_size_ = estimated_element_size_;
      cloned_current->previous_processing_time_ = previous_processing_time_;
      cloned_current->processing_time_ema_ = processing_time_ema_;
    }
  }

  for (auto& input : inputs_) {
    node_pairs->push_back(std::make_pair(input, cloned_current));
  }
  return cloned_current;
}

void Node::TotalBufferedBytesHelper(Node::NodeValues* total_bytes) const
    TF_SHARED_LOCKS_REQUIRED(mu_) {
  if (!autotune_) {
    total_bytes->insert(std::make_pair(long_name(), 0));
    return;
  }

  double result = 0;
  auto* parameter = gtl::FindOrNull(parameters_, kBufferSize);
  if (!parameter) {
    parameter = gtl::FindOrNull(parameters_, kParallelism);
  }
  if (parameter) {
    result = buffered_bytes_;
  }
  for (auto& input : inputs_) {
    result += total_bytes->at(input->long_name());
  }
  total_bytes->insert(std::make_pair(long_name(), result));
}

void Node::TotalMaximumBufferedBytesHelper(Node::NodeValues* total_bytes) const
    TF_SHARED_LOCKS_REQUIRED(mu_) {
  if (!autotune_) {
    total_bytes->insert(std::make_pair(long_name(), 0));
    return;
  }

  double result = MaximumBufferedBytes();
  for (auto& input : inputs_) {
    result += total_bytes->at(input->long_name());
  }
  total_bytes->insert(std::make_pair(long_name(), result));
}

double Node::MaximumBufferedBytes() const TF_SHARED_LOCKS_REQUIRED(mu_) {
  return 0;
}

absl::Status Node::ToProto(ModelProto::Node* node_proto) const {
  tf_shared_lock l(mu_);
  node_proto->set_id(id_);
  node_proto->set_name(name_);
  node_proto->set_autotune(autotune_);
  node_proto->set_buffered_bytes(buffered_bytes_);
  node_proto->set_buffered_elements(buffered_elements_);
  node_proto->set_bytes_consumed(bytes_consumed_);
  node_proto->set_bytes_produced(bytes_produced_);
  node_proto->set_num_elements(num_elements_);
  node_proto->set_processing_time(processing_time_);
  node_proto->set_record_metrics(record_metrics_);

  // Produce protos for all parameters.
  for (auto const& parameter : parameters_) {
    ModelProto::Node::Parameter* parameter_proto = node_proto->add_parameters();
    parameter_proto->set_name(parameter.first);
    parameter_proto->set_value(parameter.second->value);
    parameter_proto->set_min(parameter.second->min);
    parameter_proto->set_max(parameter.second->max);
    if (parameter.second->state != nullptr) {
      parameter_proto->set_state_value(parameter.second->state->value);
      parameter_proto->set_tunable(parameter.second->state->tunable);
    }
  }

  // Add input node ids.
  for (auto const& input : inputs_) {
    node_proto->add_inputs(input->id());
  }
  return absl::OkStatus();
}

absl::Status Node::FromProtoHelper(ModelProto::Node node_proto,
                                   std::shared_ptr<Node> node) {
  {
    tf_shared_lock l(node->mu_);
    node->autotune_.store(node_proto.autotune());
    node->buffered_bytes_.store(node_proto.buffered_bytes());
    node->buffered_elements_.store(node_proto.buffered_elements());
    if (node_proto.buffered_elements() == 0) {
      node->buffered_elements_low_.store(std::numeric_limits<int64_t>::max());
      node->buffered_elements_high_.store(std::numeric_limits<int64_t>::min());
    } else {
      node->buffered_elements_low_.store(node_proto.buffered_elements());
      node->buffered_elements_high_.store(node_proto.buffered_elements());
    }
    node->bytes_consumed_.store(node_proto.bytes_consumed());
    node->bytes_produced_.store(node_proto.bytes_produced());
    node->num_elements_.store(node_proto.num_elements());
    node->processing_time_.store(node_proto.processing_time());
    node->record_metrics_.store(node_proto.record_metrics());

    // Restore parameters.
    int64_t num_parameters = node_proto.parameters_size();
    for (int i = 0; i < num_parameters; i++) {
      const ModelProto::Node::Parameter& parameter_proto =
          node_proto.parameters(i);
      std::shared_ptr<SharedState> state;
      if (parameter_proto.tunable()) {
        state = std::make_shared<SharedState>(
            kAutotune, std::make_shared<mutex>(),
            std::make_shared<condition_variable>());
        state->value = parameter_proto.state_value();
      } else {
        state = std::make_shared<SharedState>(
            parameter_proto.state_value(), std::make_shared<mutex>(),
            std::make_shared<condition_variable>());
      }
      node->parameters_[parameter_proto.name()] =
          MakeParameter(parameter_proto.name(), state, parameter_proto.min(),
                        parameter_proto.max());
      node->parameters_[parameter_proto.name()]->value =
          std::max(parameter_proto.min(), parameter_proto.value());
    }
  }
  {
    mutex_lock l(node->mu_);
    node->UpdateProcessingTimeEma();
  }
  return absl::OkStatus();
}

absl::Status Node::FromProto(ModelProto::Node node_proto,
                             std::shared_ptr<Node> output,
                             std::shared_ptr<Node>* node) {
  // Note that parameters are restored in `FromProtoHelper`.
  Args args = {node_proto.id(), node_proto.name(), std::move(output)};
  switch (node_proto.node_class()) {
    case NodeClass::INTERLEAVE_MANY:
      *node = std::make_shared<InterleaveMany>(args);
      break;
    case NodeClass::ASYNC_INTERLEAVE_MANY:
      *node = std::make_shared<AsyncInterleaveMany>(
          args, /*parameters=*/std::vector<std::shared_ptr<Parameter>>());
      break;
    case NodeClass::KNOWN_RATIO:
      *node = std::make_shared<KnownRatio>(args, node_proto.ratio());
      break;
    case NodeClass::ASYNC_KNOWN_RATIO:
      *node = std::make_shared<AsyncKnownRatio>(
          args, node_proto.ratio(), node_proto.memory_ratio(),
          /*parameters=*/std::vector<std::shared_ptr<Parameter>>());
      break;
    case NodeClass::UNKNOWN_RATIO:
      *node = std::make_shared<UnknownRatio>(args);
      break;
    case NodeClass::ASYNC_UNKNOWN_RATIO:
      *node = std::make_shared<AsyncUnknownRatio>(
          args, /*parameters=*/std::vector<std::shared_ptr<Parameter>>());
      break;
    default:
      *node = std::make_shared<Unknown>(args);
  }
  return FromProtoHelper(node_proto, *node);
}

Model::Model(std::optional<std::string> dataset_name)
    : dataset_name_(std::move(dataset_name)),
      optimization_period_ms_(kOptimizationPeriodMinMs),
      safe_to_collect_metrics_(std::make_shared<GuardedBool>(true)) {
  model_id_ = absl::StrCat(reinterpret_cast<uint64_t>(this));
  model_gauge_cell_ = metrics::GetTFDataModelGauge(model_id_);

  // Capture `safe_to_collect_metrics_` by value to avoid use-after-free issues
  // when the callback is invoked after the model has been destroyed.
  model_gauge_cell_->Set(
      [this, my_safe_to_collect_metrics = this->safe_to_collect_metrics_]() {
        mutex_lock l(my_safe_to_collect_metrics->mu);
        if (!my_safe_to_collect_metrics->val) {
          return std::string();
        }
        {
          tf_shared_lock snapshot_lock(mu_);
          if (snapshot_ != nullptr) {
            ModelProto model_proto;
            absl::Status s = ModelToProtoHelper(snapshot_, &model_proto);
            if (s.ok()) {
              *model_proto.mutable_optimization_params() = optimization_params_;
              tf_shared_lock l(gap_mu_);
              *model_proto.mutable_gap_times() = {gap_times_usec_.begin(),
                                                  gap_times_usec_.end()};
              if (dataset_name_.has_value()) {
                model_proto.set_dataset_name(dataset_name_.value());
              }
              return tsl::LegacyUnredactedDebugString(model_proto);
            }
            LOG(WARNING) << s.message();
          }
        }
        return DebugString();
      });
}

Model::~Model() {
  mutex_lock l(safe_to_collect_metrics_->mu);
  safe_to_collect_metrics_->val = false;
  // Reset the pipeline processing time to 0
  metrics::RecordPipelineProcessingTime(model_id_, 0);
}

void Model::AddNode(Node::Factory factory, const std::string& name,
                    std::shared_ptr<Node> parent,
                    std::shared_ptr<Node>* out_node) {
  // The name captures the sequence of iterators joined by `::`. We only use the
  // last element of the sequence as the name node.
  auto node_name = str_util::Split(name, ':', str_util::SkipEmpty()).back();
  mutex_lock l(mu_);
  std::shared_ptr<Node> node = factory({id_counter_++, node_name, parent});
  if (!output_) {
    output_ = node;
  }
  if (parent) {
    VLOG(3) << "Adding " << node->long_name() << " as input for "
            << parent->long_name();
    parent->add_input(node);
  } else {
    VLOG(3) << "Adding " << node->long_name();
  }
  *out_node = std::move(node);
  // TODO(jsimsa): Reset the optimization period when a node is added so that
  // autotuning adapts to changes to the input pipeline faster. Initial attempt
  // to enable this functionality caused a regression (see b/179812091).
}

void Model::FlushMetrics() {
  std::deque<std::shared_ptr<Node>> queue;
  {
    tf_shared_lock l(mu_);
    if (output_) queue.push_back(output_);
  }
  while (!queue.empty()) {
    auto node = queue.front();
    queue.pop_front();
    node->FlushMetrics();
    for (const auto& input : node->inputs()) {
      queue.push_back(input);
    }
  }
}

void Model::Optimize(AutotuneAlgorithm algorithm,
                     std::function<int64_t()> cpu_budget_func,
                     double ram_budget_share,
                     std::optional<int64_t> fixed_ram_budget,
                     double model_input_time,
                     RamBudgetManager& ram_budget_manager,
                     CancellationManager* cancellation_manager) {
  std::shared_ptr<Node> snapshot;
  {
    tf_shared_lock l(mu_);
    snapshot = output_->Snapshot();
  }
  MaybeSyncStateValuesToValues(snapshot);
  int64_t total_ram_budget;
  if (fixed_ram_budget.has_value()) {
    total_ram_budget = fixed_ram_budget.value();
  } else {
    // Because the snapshot will take up some memory as time goes,
    // we need to add the total buffered bytes back
    // In other words, if we assume the system overall memory does not change:
    // Assume the port::AvailableRam() returns 1000 before the model starts
    // to tune the parameters of the dataset ops.
    // After some time, the dataset ops has buffered some bytes, say `x` bytes
    // In this case, when we call AvailableRam(), it will return 1000 - `x`.
    // But we want the `total_ram_budget` to be fixed.
    // So we need to add `x` bytes back, i.e. AvailableRam() + x == 1000 - x + x
    total_ram_budget = ram_budget_share *
                       (port::AvailableRam() + TotalBufferedBytes(snapshot));
  }

  ram_budget_manager.UpdateBudget(total_ram_budget);
  int64_t model_ram_budget = ram_budget_manager.AvailableModelRam();
  int64_t original_model_bytes = TotalMaximumBufferedBytes(snapshot);
  if (!port::JobName().empty()) {
    RecordAutotuneRamUsage(model_ram_budget, original_model_bytes);
  }
  OptimizationParams optimization_params;
  optimization_params.set_algorithm(algorithm);
  optimization_params.set_cpu_budget(cpu_budget_func());
  optimization_params.set_ram_budget(model_ram_budget);
  optimization_params.set_model_input_time(model_input_time);
  switch (algorithm) {
    case AutotuneAlgorithm::DEFAULT:
    case AutotuneAlgorithm::MAX_PARALLELISM:
      OptimizeMaxParallelism(snapshot, optimization_params,
                             cancellation_manager, ram_budget_manager);
      break;
    case AutotuneAlgorithm::HILL_CLIMB:
      OptimizeHillClimb(snapshot, optimization_params, cancellation_manager,
                        ram_budget_manager);
      break;
    case AutotuneAlgorithm::GRADIENT_DESCENT:
      LOG(WARNING) << "AutotuneAlgorithm::GRADIENT_DESCENT and prefetch "
                      "legacy autotuner should not be turned on together"
                   << "as OptimizeGradientDescent will update state values "
                      "without consulting ram_budget_manager first."
                   << "This might cause out-of-memory (OOM)";
      OptimizeGradientDescent(snapshot, optimization_params,
                              cancellation_manager);
      break;
    case AutotuneAlgorithm::STAGE_BASED:
      OptimizeStageBased(snapshot, optimization_params, cancellation_manager,
                         ram_budget_manager);
      break;
    default:
      VLOG(2) << "Autotuning algorithm was not recognized. Aborting "
                 "optimization.";
      return;
  }
  if (experiments_.contains("autotune_buffer_optimization")) {
    OptimizeBuffers(snapshot, optimization_params.ram_budget());
  }
  {
    // Save the snapshot of the model proto including the parameters used by
    // autotune. This will be used as the model proto returned in `tfstreamz`.
    mutex_lock l(mu_);
    snapshot_ = snapshot;
    optimization_params_ = optimization_params;

    if (snapshot_) {
      double pipeline_processing_usec = 0;
      ModelTiming model_timing(snapshot_);
      auto bfs_stage_roots = model_timing.GetStageRoots();
      for (const auto& root : bfs_stage_roots) {
        auto* root_timing = model_timing.GetTiming(root.get());
        if (root_timing == nullptr) {
          constexpr int TEN_MINUTES = 60 * 10;
          LOG_EVERY_N_SEC(ERROR, TEN_MINUTES)
              << "Encounter an error when computing the pipeline processing "
                 "time for "
                 "/tensorflow/data/pipeline_processing_time";
          pipeline_processing_usec = 0;
          break;
        }

        double root_total_time_usec = root_timing->total_time_nsec *
                                      root_timing->pipeline_ratio /
                                      EnvTime::kMicrosToNanos;

        pipeline_processing_usec =
            std::max(pipeline_processing_usec, root_total_time_usec);
      }
      // Only updates the pipeline processing time when it is greater than 0.
      // If it is zero, we assume the pipeline processing time is the same
      // as the previous one and do not update it.
      if (pipeline_processing_usec > 0) {
        metrics::RecordPipelineProcessingTime(model_id_,
                                              pipeline_processing_usec);
      }
    }
  }
  VLOG(2) << ram_budget_manager.DebugString();
}

void Model::RemoveNode(std::shared_ptr<Node> node) {
  if (node) {
    if (node->output()) {
      std::shared_ptr<Node> output_shared = node->output_shared();
      if (output_shared) {
        output_shared->remove_input(node);
      }
    }
    VLOG(3) << "Removing " << node->long_name();
  }
}

Model::ModelParameters Model::CollectTunableParameters(
    std::shared_ptr<Node> node) {
  return node->CollectTunableParameters();
}

void Model::MaybeSyncStateValuesToValues(std::shared_ptr<Node> snapshot) {
  auto subtree_nodes = snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
  for (const auto& node : subtree_nodes) {
    if (!absl::StartsWith(node->name(), kDataService)) {
      continue;
    }
    node->SyncStateValuesToParameterValues(kBufferSize);
  }
}

bool Model::DownsizeBuffers(std::shared_ptr<Node> snapshot) {
  Node::NodeVector nodes =
      snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
  nodes.push_back(snapshot);
  bool downsized = false;
  for (auto& node : nodes) {
    if (!node->IsAsync()) {
      continue;
    }
    if (node->TryDownsizeBuffer()) {
      downsized = true;
    }
  }
  return downsized;
}

absl::flat_hash_map<Node*, Parameter*> Model::CollectBufferParametersToUpsize(
    std::shared_ptr<Node> snapshot) {
  Node::NodeVector nodes =
      snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
  absl::flat_hash_map<Node*, Parameter*> node_parameters;
  if (snapshot->IsAsync()) {
    snapshot->CollectBufferParametersToUpsize(node_parameters);
  }
  for (auto& node : nodes) {
    if (!node->IsAsync()) {
      continue;
    }
    node->CollectBufferParametersToUpsize(node_parameters);
  }
  return node_parameters;
}

bool Model::ShouldStop(int64_t cpu_budget, int64_t ram_budget,
                       const Model::ModelParameters& parameters,
                       const Model::ModelParameters& parallelism_parameters,
                       const Model::ModelParameters& buffer_size_parameters,
                       std::shared_ptr<Node> snapshot,
                       bool* cpu_budget_reached) {
  if (!(*cpu_budget_reached)) {
    // If those essential transformations' parallelism reaches the CPU budget,
    // we will only tune the buffer size parameters in future iterations.
    int64_t model_parallelism = 0;
    for (auto& pair : parallelism_parameters) {
      model_parallelism += std::round(pair.second->value);
    }
    *cpu_budget_reached = (model_parallelism > cpu_budget);
  }

  bool all_max = AreAllParametersMax(
      *cpu_budget_reached ? buffer_size_parameters : parameters);

  // If all parameters have reached their maximum values or RAM budget is
  // reached, we stop the iterations.
  return all_max || TotalMaximumBufferedBytes(snapshot) > ram_budget;
}

// TODO(jsimsa): Add support for tracking and using the model input time.
absl::Status Model::OptimizeLoop(AutotuneAlgorithm algorithm,
                                 std::function<int64_t()> cpu_budget_func,
                                 double ram_budget_share,
                                 std::optional<int64_t> fixed_ram_budget,
                                 RamBudgetManager& ram_budget_manager,
                                 CancellationManager* cancellation_manager) {
  std::function<void()> unused;
  TF_RETURN_IF_ERROR(RegisterCancellationCallback(
      cancellation_manager,
      [this]() {
        mutex_lock l(mu_);
        optimize_cond_var_.notify_all();
      },
      /*deregister_fn=*/&unused));

  int64_t last_optimization_ms = 0;
  int64_t current_time_ms = EnvTime::NowMicros() / EnvTime::kMillisToMicros;
  while (true) {
    {
      mutex_lock l(mu_);
      while (!cancellation_manager->IsCancelled() &&
             last_optimization_ms + optimization_period_ms_ > current_time_ms) {
        auto wait_ms =
            last_optimization_ms + optimization_period_ms_ - current_time_ms;
        VLOG(2) << "Waiting for " << wait_ms << " ms.";
        optimize_cond_var_.wait_for(l, std::chrono::milliseconds(wait_ms));
        current_time_ms = EnvTime::NowMicros() / EnvTime::kMillisToMicros;
      }
      if (cancellation_manager->IsCancelled()) {
        return absl::OkStatus();
      }
    }

    int64_t start_ms = EnvTime::NowMicros() / EnvTime::kMillisToMicros;
    double model_input_time = 0.0;
    // Model input time is set to 0 for all optimization algorithms except for
    // stage-based optimization algorithm for historical reason. In stage-based
    // optimization algorithm, the model input time is used as a target
    // optimization time of all stages in the pipeline.
    if (algorithm == AutotuneAlgorithm::STAGE_BASED) {
      model_input_time = ComputeTargetTimeNsec();
    }
    Optimize(algorithm, cpu_budget_func, ram_budget_share, fixed_ram_budget,
             model_input_time, ram_budget_manager, cancellation_manager);
    int64_t end_ms = EnvTime::NowMicros() / EnvTime::kMillisToMicros;
    VLOG(2) << "Optimized for " << end_ms - start_ms << " ms.";

    // Exponentially increase the period of running the optimization until a
    // threshold is reached.
    {
      mutex_lock l(mu_);
      optimization_period_ms_ =
          std::min(optimization_period_ms_ << 1, kOptimizationPeriodMaxMs);
    }
    current_time_ms = EnvTime::NowMicros() / EnvTime::kMillisToMicros;
    last_optimization_ms = current_time_ms;
    FlushMetrics();
  }
}

void Model::OptimizeGradientDescent(
    std::shared_ptr<Node> snapshot,
    const OptimizationParams& optimization_params,
    CancellationManager* cancellation_manager) {
  VLOG(2) << "Starting optimization of tunable parameters with Gradient "
             "Descent.";
  auto parameters = CollectTunableParameters(snapshot);
  if (parameters.empty()) {
    VLOG(2) << "The Gradient Descent optimization is terminated since no node "
               "with tunable parameters has recorded elements.";
    return;
  }
  VLOG(2) << "Number of tunable parameters: " << parameters.size();

  // The vectors of "essential" parallelism parameters and buffer size
  // parameters.
  Model::ModelParameters parallelism_parameters, buffer_size_parameters;
  CollectParameters(snapshot, parameters, &parallelism_parameters,
                    &buffer_size_parameters);

  // Initialize the parameter values to minimal before tuning.
  for (auto& pair : parameters) {
    pair.second->value = pair.second->min;
  }

  // Optimization is stopped once the `OutputTime` improvement is smaller than
  // this value.
  constexpr double kOptimizationPrecision = 100.0L;

  // Maximum number of iterations for optimization.
  constexpr int64_t kMaxIterations = 1000;

  double output_time = 0;
  double new_output_time;

  // When the CPU budget is reached, the parallelism parameter values are fixed
  // and we only increase the buffer size parameters.
  bool cpu_budget_reached = false;

  for (int i = 0; i < kMaxIterations; ++i) {
    if (cancellation_manager->IsCancelled() ||
        ShouldStop(optimization_params.cpu_budget(),
                   optimization_params.ram_budget(), parameters,
                   parallelism_parameters, buffer_size_parameters, snapshot,
                   &cpu_budget_reached)) {
      break;
    }
    Model::ParameterGradients gradients;
    new_output_time = OutputTime(
        snapshot, optimization_params.model_input_time(), &gradients);
    // We also terminate once the improvement of the output latency is too
    // small.
    if (std::abs(output_time - new_output_time) < kOptimizationPrecision) {
      break;
    }

    UpdateParameterValues(
        gradients, &(cpu_budget_reached ? buffer_size_parameters : parameters));
    output_time = new_output_time;
  }

  for (auto& pair : parameters) {
    pair.second->value = std::round(pair.second->value);
  }
  UpdateStateValues(&parameters);
}

void Model::OptimizeHillClimbHelper(
    std::shared_ptr<Node> snapshot,
    const OptimizationParams& optimization_params,
    CancellationManager* cancellation_manager, int64_t ram_budget,
    RamBudgetManager& ram_budget_manager, StopPredicate should_stop) {
  VLOG(2) << "Starting optimization of tunable parameters with Hill Climb.";
  const double processing_time = TotalProcessingTime(snapshot);
  auto parameters = CollectTunableParameters(snapshot);
  if (parameters.empty()) {
    VLOG(2) << "There are no tunable parameters.";
    return;
  }
  VLOG(2) << "Number of tunable parameters: " << parameters.size();

  // Buffer size parameter will only be incremented if the output latency
  // improvement is greater than this constant.
  constexpr double kBufferSizeMinDelta = 1.0L;

  // Skip buffer size optimization if we are running the new buffering
  // algorithm.
  bool skip_buffer_sizes =
      (experiments_.contains("autotune_buffer_optimization"));
  if (skip_buffer_sizes) {
    constexpr float TEN_MINUTES = 60.0 * 10.0;
    LOG_EVERY_N_SEC(INFO, TEN_MINUTES)
        << "Skipping buffer_size parameters in HillClimb (message logged "
           "every "
           "10 minutes).";
  }
  // Initialize the parameter values to minimal before tuning.
  for (auto& pair : parameters) {
    if (skip_buffer_sizes && (pair.second->name == kBufferSize)) {
      continue;
    }
    pair.second->value = pair.second->min;
  }
  Parameter* best_parameter = nullptr;
  while (!cancellation_manager->IsCancelled()) {
    const double output_time =
        OutputTime(snapshot, optimization_params.model_input_time(),
                   /*gradients=*/nullptr);
    const double new_buffered_bytes = TotalMaximumBufferedBytes(snapshot);
    if (should_stop(parameters, processing_time, output_time,
                    new_buffered_bytes)) {
      if (best_parameter && new_buffered_bytes > ram_budget) {
        // Take a step back of the previous hill climbing attempt
        // so that the final parameters will not exceed the ram budget
        // as the current parameter meets should_stop(...) condition
        best_parameter->value--;
      }
      break;
    }

    double best_delta = -1.0L;
    best_parameter = nullptr;
    for (auto& pair : parameters) {
      if (pair.second->value >= pair.second->max ||
          (skip_buffer_sizes && (pair.second->name == kBufferSize))) {
        continue;
      }
      pair.second->value++;
      double new_output_time =
          OutputTime(snapshot, optimization_params.model_input_time(),
                     /*gradients=*/nullptr);
      double delta = output_time - new_output_time;
      if (delta > best_delta &&
          (delta > kBufferSizeMinDelta || pair.second->name != kBufferSize)) {
        best_delta = delta;
        best_parameter = pair.second.get();
      }
      pair.second->value--;
    }
    if (!best_parameter) {
      metrics::RecordTFDataAutotuneStoppingCriteria("local_maximum_reached");
      VLOG(2) << "Failed to find a tunable parameter that would further "
                 "decrease the output time. This suggests that the hill-climb "
                 "optimization got stuck in a local maximum. The optimization "
                 "attempt will stop now.";
      break;
    }
    // Take a hill-climb step
    best_parameter->value++;
  }
  if (ram_budget_manager.RequestModelAllocation(
          TotalMaximumBufferedBytes(snapshot))) {
    // Note that `ram_budget` is only a snapshot of
    // ram_budget_manager.AvailableModelRam()
    // that is why we still need to invoke RequestModelAllocation
    // as `ram_budget` might be outdated
    UpdateStateValues(&parameters);
  }
}
void Model::RecordIteratorGapTime(uint64_t duration_usec) {
  mutex_lock l(gap_mu_);
  // Drop duration if it is too large.
  if (duration_usec >= absl::ToInt64Microseconds(kGapDurationUpperThreshold)) {
    VLOG(3) << "Dropped tf.data Model gap duration: " << duration_usec;
    return;
  }
  VLOG(3) << "Reported tf.data Model gap duration: " << duration_usec;
  gap_times_usec_.push_back(duration_usec);
  // Keep only the latest `window` gap times. Drop the oldest one.
  while (gap_times_usec_.size() > kGapTimeWindow) {
    gap_times_usec_.pop_front();
  }
}

double Model::ComputeTargetTimeNsec() {
  tf_shared_lock l(gap_mu_);
  if (gap_times_usec_.size() < kGapTimeWindow) {
    return 0.0;
  }
  double target_time_sigmas = 0.0;
  if (experiments_.contains("stage_based_autotune_v2")) {
    target_time_sigmas = kTargetTimeSigmas;
  }
  return TargetTimeCalculator({gap_times_usec_.begin(), gap_times_usec_.end()},
                              kOutlierSigmas, target_time_sigmas)
             .GetTargetTimeUsec() *
         1.0e3;
}

// TODO(armandouv): Evaluate replacing original target time computation for
// this.
double Model::ComputeExperimentalTargetTimeNsec() {
  tf_shared_lock l(gap_mu_);
  if (gap_times_usec_.size() < kGapTimeWindow) {
    return 0.0;
  }

  uint64_t sum =
      std::accumulate(gap_times_usec_.begin(), gap_times_usec_.end(), 0);
  // Return average gap time.
  return (static_cast<double>(sum) /
          static_cast<double>(gap_times_usec_.size())) *
         1.0e3;
}

double Model::ComputeSnapshotProcessingTimeNsec() const {
  std::unique_ptr<ModelTiming> model_timing = nullptr;
  {
    tf_shared_lock l(mu_);
    if (snapshot_ == nullptr) {
      return 0.0;
    }
    model_timing = std::make_unique<ModelTiming>(snapshot_);
  }

  ModelTimingPriorityQueue priority_queue(*model_timing);
  absl::StatusOr<std::pair<double, Node*>> critical_root_status =
      priority_queue.PopSlowestStageRoot();
  if (!critical_root_status.ok()) {
    return 0.0;
  }
  return critical_root_status->first;
}

void Model::OptimizeStageBased(std::shared_ptr<Node> snapshot,
                               const OptimizationParams& optimization_params,
                               CancellationManager* cancellation_manager,
                               RamBudgetManager& ram_budget_manager) {
  VLOG(2) << "Starting optimization of tunable parameters with Stage-Based "
             "optimization with a target time of "
          << optimization_params.model_input_time() << " nanoseconds.";
  if (experiments_.contains("stage_based_autotune_v2")) {
    OptimizeStageBasedAsyncInterleaveManyNodes(snapshot, optimization_params,
                                               cancellation_manager,
                                               ram_budget_manager);
  }
  OptimizeStageBasedNonAsyncInterleaveManyNodes(
      snapshot, optimization_params.model_input_time(), optimization_params,
      cancellation_manager, ram_budget_manager);
}

void Model::OptimizeStageBasedAsyncInterleaveManyNodes(
    std::shared_ptr<Node> snapshot,
    const OptimizationParams& optimization_params,
    CancellationManager* cancellation_manager,
    RamBudgetManager& ram_budget_manager) {
  VLOG(2) << "Optimizing async interleave many nodes.";
  Node::NodeVector interleave_many_nodes =
      snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
  if (IsAsyncInterleaveManyNode(snapshot)) {
    interleave_many_nodes.push_back(snapshot);
  }
  Node::ModelParameters tunable_parameters;
  for (const auto& node : interleave_many_nodes) {
    if (!IsAsyncInterleaveManyNode(node)) {
      continue;
    }
    Node::ModelParameters node_tunable_parameters =
        node->CollectNodeTunableParameters();
    tunable_parameters.insert(tunable_parameters.end(),
                              node_tunable_parameters.begin(),
                              node_tunable_parameters.end());
  }
  ModelTiming model_timing(snapshot);
  ModelTimingPriorityQueue priority_queue(model_timing);
  NodeParallelismParameters node_parallelism;
  while (!cancellation_manager->IsCancelled()) {
    absl::StatusOr<std::pair<double, Node*>> critical_root_status =
        priority_queue.PopSlowestStageRoot();
    if (!critical_root_status.ok()) {
      // All async interleave many nodes have been processed.
      break;
    }
    std::pair<double, Node*> critical_root = critical_root_status.value();
    if (!IsAsyncInterleaveManyNode(critical_root.second)) {
      continue;
    }
    Parameter* parallelism_parameter =
        node_parallelism.Get(critical_root.second);
    if (parallelism_parameter == nullptr ||
        parallelism_parameter->value >= parallelism_parameter->max) {
      continue;
    }
    parallelism_parameter->value += 1.0;
    if (TotalMaximumBufferedBytes(snapshot) >
        optimization_params.ram_budget()) {
      // Increasing the parallelism by 1 exceeded ram budget. Reduce it back and
      // stop optimization because we cannot improve the most critical stage.
      // There is also a decent chance that the current optimization iteration
      // is under-optimized. For that reason, return immediately without
      // updating the parameter state values.
      parallelism_parameter->value -= 1.0;
      // Removes the `<index>` of `[<index>]` to reduce the number of labels.
      metrics::RecordTFDataAutotuneStoppingCriteria(
          absl::StrCat("ram_budget_exceeded:",
                       RemoveArrayIndices(critical_root.second->long_name())));
      return;
    }
    model_timing.ComputeNodeTotalTime(*critical_root.second);
    // This async interleave many node has not reached its max parallelism
    // value. Push it back to the priority queue.
    const ModelTiming::NodeTiming* root_timing =
        model_timing.GetTiming(critical_root.second);
    priority_queue.Push(critical_root.second, *root_timing);
  }
  if (ram_budget_manager.RequestModelAllocation(
          TotalMaximumBufferedBytes(snapshot))) {
    UpdateStateValues(&tunable_parameters);
  }
}

void Model::OptimizeStageBasedNonAsyncInterleaveManyNodes(
    std::shared_ptr<Node> snapshot, double target_time_nsec,
    const OptimizationParams& optimization_params,
    CancellationManager* cancellation_manager,
    RamBudgetManager& ram_budget_manager) {
  VLOG(2) << "Optimizing nodes other than async interleave many nodes.";
  Node::NodeVector all_nodes;
  if (experiments_.contains("stage_based_autotune_v2")) {
    all_nodes = snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
    if (!IsAsyncInterleaveManyNode(snapshot)) {
      all_nodes.push_back(snapshot);
    }
  } else {
    all_nodes = snapshot->CollectNodes(TraversalOrder::BFS, IsAnyNode);
    all_nodes.push_back(snapshot);
  }
  Node::ModelParameters tunable_parameters;
  for (const auto& node : all_nodes) {
    if (IsAsyncInterleaveManyNode(node)) {
      continue;
    }
    Node::ModelParameters node_tunable_parameters =
        node->CollectNodeTunableParameters();
    tunable_parameters.insert(tunable_parameters.end(),
                              node_tunable_parameters.begin(),
                              node_tunable_parameters.end());
  }
  // Initialize the parallelism parameter values to minimal before tuning.
  for (std::pair<std::string, std::shared_ptr<Parameter>>& pair :
       tunable_parameters) {
    if (pair.second->name != kParallelism) {
      continue;
    }
    pair.second->value = pair.second->min;
  }
  ModelTiming model_timing(snapshot);
  ModelTimingPriorityQueue priority_queue(model_timing);
  absl::StatusOr<std::pair<double, Node*>> critical_root_status =
      priority_queue.PopSlowestStageRoot();
  if (!critical_root_status.ok()) {
    metrics::RecordTFDataAutotuneStoppingCriteria("empty_critical_queue");
    return;
  }
  NodeParallelismParameters node_parallelism;
  std::pair<double, Node*> critical_root = critical_root_status.value();
  while (critical_root.first > target_time_nsec) {
    Parameter* parallelism_parameter =
        node_parallelism.Get(critical_root.second);
    // Stop optimization if the critical stage has no `parallelism` parameter or
    // it has reached the max parallelism value.
    if (parallelism_parameter == nullptr) {
      // Removes the `<index>` of `[<index>]` to reduce the number of labels.
      metrics::RecordTFDataAutotuneStoppingCriteria(
          absl::StrCat("no_optimizable_parameter:",
                       RemoveArrayIndices(critical_root.second->long_name())));
      break;
    }
    if (parallelism_parameter->value >= parallelism_parameter->max) {
      // Removes the `<index>` of `[<index>]` to reduce the number of labels.
      metrics::RecordTFDataAutotuneStoppingCriteria(
          absl::StrCat("parameter_max_exceeded:",
                       RemoveArrayIndices(critical_root.second->long_name())));
      break;
    }
    parallelism_parameter->value += 1.0;
    if (cancellation_manager->IsCancelled() ||
        TotalMaximumBufferedBytes(snapshot) >
            optimization_params.ram_budget()) {
      // Either the optimization thread is cancelled or increasing the
      // parallelism by 1 exceeded ram budget. There is a decent chance that the
      // current optimization iteration is under-optimized. For that reason,
      // return immediately without updating the parameter state values after
      // recording the stopping criteria.

      // Removes the `<index>` of `[<index>]` to reduce the number of labels.
      metrics::RecordTFDataAutotuneStoppingCriteria(
          absl::StrCat("ram_budget_exceeded:",
                       RemoveArrayIndices(critical_root.second->long_name())));
      return;
    }
    // Compute the new total time and put the node back in the queue after its
    // parallelism value has been increased by 1.
    model_timing.ComputeNodeTotalTime(*critical_root.second);
    const ModelTiming::NodeTiming* root_timing =
        model_timing.GetTiming(critical_root.second);
    // If timing has not improved, stop optimizing.
    if (critical_root.first <=
        (root_timing->total_time_nsec * root_timing->pipeline_ratio)) {
      parallelism_parameter->value -= 1.0;
      // Removes the `<index>` of `[<index>]` to reduce the number of labels.
      metrics::RecordTFDataAutotuneStoppingCriteria(
          absl::StrCat("total_time_not_improved:",
                       RemoveArrayIndices(critical_root.second->long_name())));
      break;
    }
    // Push it back to the priority queue.
    priority_queue.Push(critical_root.second, *root_timing);
    // Get the next critical stage root.
    critical_root_status = priority_queue.PopSlowestStageRoot();
    if (!critical_root_status.ok()) {
      metrics::RecordTFDataAutotuneStoppingCriteria("empty_critical_queue");
      break;
    }
    critical_root = critical_root_status.value();
  }
  if (ram_budget_manager.RequestModelAllocation(
          TotalMaximumBufferedBytes(snapshot))) {
    UpdateStateValues(&tunable_parameters);
  }
}

void Model::OptimizeBuffers(std::shared_ptr<Node> snapshot,
                            int64_t ram_budget) {
  VLOG(2) << "Starting optimization of buffer_size parameters.";
  constexpr float TEN_MINUTES = 60.0 * 10.0;
  LOG_EVERY_N_SEC(INFO, TEN_MINUTES)
      << "Starting optimization of buffer_size parameters (message logged "
         "every 10 minutes).";
  // Reset node watermarks if any node's buffer is upsized or downsized. We
  // reset the watermarks of not only those nodes whose sizes change but all
  // nodes. The reason is that the optimization algorithm works on a snapshot of
  // nodes. There is no back references from snapshot of nodes to nodes. We
  // could add these back references but it is probably not necessary.
  bool downsized = DownsizeBuffers(snapshot);
  bool upsized = UpsizeBuffers(snapshot, ram_budget);
  if (downsized || upsized) {
    ResetBufferWatermarks();
  }
}

bool Model::UpsizeBuffers(std::shared_ptr<Node> snapshot, int64_t ram_budget) {
  // Find buffers that should be up-sized.
  absl::flat_hash_map<Node*, Parameter*> node_parameters =
      CollectBufferParametersToUpsize(snapshot);

  // Compute available memory.
  double available_ram_bytes =
      static_cast<double>(ram_budget) - TotalMaximumBufferedBytes(snapshot);

  // Compute the max memory used by all buffers that should be upsized.
  double max_buffered_bytes = 0;
  for (auto& [node, parameter] : node_parameters) {
    if (node->buffered_elements() == 0) {
      continue;
    }
    max_buffered_bytes += static_cast<double>(node->buffered_bytes()) /
                          static_cast<double>(node->buffered_elements()) *
                          parameter->value;
  }

  // Compute a uniform scaling factor for all buffers. Cap the factor at 2.
  double scaling_factor = kBufferUpsizeMultiplier;
  if (max_buffered_bytes > 0) {
    scaling_factor =
        1.0 + std::min(1.0, available_ram_bytes / max_buffered_bytes);
  }

  bool upsized = false;
  // Up-size all buffers by the scaling factor.
  for (auto& [node, parameter] : node_parameters) {
    double old_value = parameter->value;
    // Scale the new buffer_size value. Use 1 if it is less than 1.
    double new_value = std::max(1.0, static_cast<double>(static_cast<int64_t>(
                                         parameter->value * scaling_factor)));
    // Cap the new buffer_size value at its max value.
    parameter->value = std::min(parameter->max, new_value);
    VLOG(2) << "Upsize buffer " << node->long_name() << "::" << parameter->name
            << " from " << old_value << " to " << parameter->value;
    {
      mutex_lock l(*parameter->state->mu);
      if (parameter->value != parameter->state->value) {
        parameter->state->value = parameter->value;
        parameter->state->cond_var->notify_all();
        upsized = true;
      }
    }
  }
  return upsized;
}

void Model::ResetBufferWatermarks() {
  Node::NodeVector nodes =
      output()->CollectNodes(TraversalOrder::BFS, IsAnyNode);
  nodes.push_back(output());
  for (auto& node : nodes) {
    if (!node->IsAsync()) {
      continue;
    }
    node->ResetBufferWatermarks();
  }
}

void Model::OptimizeHillClimb(std::shared_ptr<Node> snapshot,
                              const OptimizationParams& optimization_params,
                              CancellationManager* cancellation_manager,
                              RamBudgetManager& ram_budget_manager) {
  auto should_stop = [&optimization_params](const ModelParameters& parameters,
                                            double processing_time,
                                            double output_time,
                                            double buffered_bytes) {
    const bool all_max = AreAllParametersMax(parameters);
    const bool output_time_budget_exceeded =
        output_time < processing_time / optimization_params.cpu_budget();
    const bool ram_budget_exceeded =
        buffered_bytes > optimization_params.ram_budget();
    if (all_max) {
      metrics::RecordTFDataAutotuneStoppingCriteria("all_max");
    }
    if (output_time_budget_exceeded) {
      metrics::RecordTFDataAutotuneStoppingCriteria("output_time");
    }
    if (ram_budget_exceeded) {
      metrics::RecordTFDataAutotuneStoppingCriteria("max_buffered_bytes");
    }
    return all_max || output_time_budget_exceeded || ram_budget_exceeded;
  };
  OptimizeHillClimbHelper(snapshot, optimization_params, cancellation_manager,
                          optimization_params.ram_budget(), ram_budget_manager,
                          should_stop);
}

void Model::OptimizeMaxParallelism(
    std::shared_ptr<Node> snapshot,
    const OptimizationParams& optimization_params,
    CancellationManager* cancellation_manager,
    RamBudgetManager& ram_budget_manager) {
  auto should_stop = [&optimization_params](const ModelParameters& parameters,
                                            double processing_time,
                                            double output_time,
                                            double buffered_bytes) {
    const bool all_max = AreAllParametersMax(parameters);
    const bool ram_budget_exceeded =
        buffered_bytes > optimization_params.ram_budget();
    if (all_max) {
      metrics::RecordTFDataAutotuneStoppingCriteria("all_max");
    }
    if (ram_budget_exceeded) {
      metrics::RecordTFDataAutotuneStoppingCriteria("max_buffered_bytes");
    }
    return all_max || ram_budget_exceeded;
  };
  OptimizeHillClimbHelper(snapshot, optimization_params, cancellation_manager,
                          optimization_params.ram_budget(), ram_budget_manager,
                          should_stop);
}

double Model::OutputTime(std::shared_ptr<Node> node, double model_input_time,
                         Model::ParameterGradients* gradients) {
  // To store the input time for each node.
  Model::NodeValues input_times = {{kModelInputTimeKey, model_input_time}};

  // TODO(jsimsa): Now that we are accounting for buffer size in wait time
  // computation, assuming that the input is infinitely fast will result in
  // inaccurate estimates of the output latency.
  //
  // We should compute the output latency as a fix-point of the following
  // equation: `output_time = node(OutputTime(input_times(1, output_time))`.

  return node->OutputTime(&input_times, gradients);
}

double Model::TotalBufferedBytes(std::shared_ptr<Node> node) {
  return node->TotalBufferedBytes();
}

double Model::TotalMaximumBufferedBytes(std::shared_ptr<Node> node) {
  return node->TotalMaximumBufferedBytes();
}

double Model::TotalProcessingTime(std::shared_ptr<Node> node) {
  return node->TotalProcessingTime(/*processing_times=*/nullptr);
}

absl::Status Model::ToProto(ModelProto* model_proto) {
  tf_shared_lock l(mu_);
  model_proto->set_id_counter(id_counter_);
  TF_RETURN_IF_ERROR(ModelToProtoHelper(output_, model_proto));
  model_proto->set_id_counter(id_counter_);
  *model_proto->mutable_optimization_params() = optimization_params_;
  if (dataset_name_.has_value()) {
    model_proto->set_dataset_name(dataset_name_.value());
  }
  tf_shared_lock gap_lock(gap_mu_);
  *model_proto->mutable_gap_times() = {gap_times_usec_.begin(),
                                       gap_times_usec_.end()};
  return absl::OkStatus();
}

absl::Status Model::FromProto(ModelProto model_proto,
                              std::unique_ptr<Model>* model) {
  std::unique_ptr<Model> restored_model = std::make_unique<Model>();
  mutex_lock l(restored_model->mu_);
  TF_RETURN_IF_ERROR(
      ModelFromProtoHelper(model_proto, &restored_model->output_));
  restored_model->id_counter_ = model_proto.id_counter();
  *model = std::move(restored_model);
  return absl::OkStatus();
}

absl::Status Model::Save(const std::string& fname,
                         std::shared_ptr<Node> snapshot,
                         const OptimizationParams& optimization_params) {
  ModelProto model_proto;
  std::unique_ptr<Model> model_snapshot = std::make_unique<Model>();
  {
    mutex_lock l(model_snapshot->mu_);
    model_snapshot->output_ = std::move(snapshot);
    model_snapshot->id_counter_ = id_counter_;
  }
  TF_RETURN_IF_ERROR(model_snapshot->ToProto(&model_proto));
  OptimizationParams* saved_optimization_params =
      model_proto.mutable_optimization_params();
  *saved_optimization_params = optimization_params;
  return WriteBinaryProto(Env::Default(), fname, model_proto);
}

absl::Status Model::Load(const std::string& fname,
                         std::unique_ptr<Model>* model,
                         OptimizationParams* optimization_params) {
  ModelProto model_proto;
  TF_RETURN_IF_ERROR(
      ReadTextOrBinaryProto(Env::Default(), fname, &model_proto));
  TF_RETURN_IF_ERROR(FromProto(model_proto, model));
  const OptimizationParams restored_optimization_params =
      model_proto.optimization_params();
  *optimization_params = restored_optimization_params;
  return absl::OkStatus();
}

std::string Model::DebugString() {
  constexpr int64_t kMinSecondsBetweenCalls = 30;
  if (absl::Now() < cache_until_) return cached_debug_string_;
  std::shared_ptr<Node> snapshot;
  {
    tf_shared_lock l(mu_);
    if (!output_) return cached_debug_string_;
    snapshot = output_->Snapshot();
  }
  // TODO(jsimsa): Populate OptimizationParams.
  ModelProto model_proto;
  absl::Status s = ModelToProtoHelper(snapshot, &model_proto);
  if (s.ok()) {
    cached_debug_string_ = model_proto.DebugString();
  } else {
    LOG(WARNING) << s.message();
  }
  cache_until_ = absl::Now() + absl::Seconds(kMinSecondsBetweenCalls);
  return cached_debug_string_;
}

ModelTiming::ModelTiming(std::shared_ptr<Node> root) : root_(root) {
  DCHECK(root_.get() != nullptr);
  auto bfs_nodes = CollectNodes(root_, TraversalOrder::BFS, IsAnyNode);
  auto reverse_bfs_nodes = bfs_nodes;
  std::reverse(reverse_bfs_nodes.begin(), reverse_bfs_nodes.end());
  ComputePipelineRatios(bfs_nodes);
  ComputeTotalTimes(reverse_bfs_nodes);
}

Node::NodeVector ModelTiming::CollectNodes(
    std::shared_ptr<Node> root, TraversalOrder order,
    bool collect_node(const std::shared_ptr<Node>)) const {
  if (root == nullptr) {
    return Node::NodeVector({});
  }
  auto subtree_nodes = root->CollectNodes(order, collect_node);
  Node::NodeVector nodes;
  if (order == TraversalOrder::BFS) {
    nodes.push_back(root);
    nodes.insert(nodes.end(), subtree_nodes.begin(), subtree_nodes.end());
  } else {
    nodes.insert(nodes.end(), subtree_nodes.begin(), subtree_nodes.end());
    nodes.push_back(root);
  }
  return nodes;
}

const ModelTiming::NodeTiming* ModelTiming::GetTiming(const Node* node) const {
  if (timing_nodes_.find(node) == timing_nodes_.end()) {
    return nullptr;
  }
  return &(timing_nodes_.at(node));
}

void ModelTiming::ComputePipelineRatios(const Node::NodeVector& bfs_nodes) {
  for (const auto& node : bfs_nodes) {
    auto& node_timing = timing_nodes_[node.get()];
    double parent_pipeline_ratio = 1.0;
    double parent_ratio = 1.0;
    if (node->output() != nullptr || timing_nodes_.contains(node->output())) {
      const auto& output_timing = timing_nodes_[node->output()];
      parent_pipeline_ratio = output_timing.pipeline_ratio;
      auto should_estimate_first_input_ratio = [node]() {
        // Elements of the first input of some transformations like `Interleave`
        // are used to produce "derived" inputs whose elements are then produced
        // as the output of the transformation. For this reason, the ratio of
        // such inputs is not known statically and is instead estimated as the
        // ratio of `num_elements` it produces and `num_elements` its output
        // produces.
        return (absl::StartsWith(node->output()->name(), kFlatMap) ||
                absl::StartsWith(node->output()->name(), kInterleave) ||
                absl::StartsWith(node->output()->name(),
                                 kParallelInterleave)) &&
               node.get() == node->output()->inputs().begin()->get() &&
               node->num_elements() > 0 && node->output()->num_elements() > 0;
      };
      if (should_estimate_first_input_ratio()) {
        parent_ratio = static_cast<double>(node->num_elements()) /
                       static_cast<double>(node->output()->num_elements() +
                                           node->output()->buffered_elements());
      } else {
        parent_ratio = node->output()->Ratio();
      }
      if (parent_ratio <= 0.0) {
        // Parent ratio is unknown, we use 1.0 as a guess.
        parent_ratio = 1.0;
      }
    }
    node_timing.pipeline_ratio = parent_pipeline_ratio * parent_ratio;
  }
}

void ModelTiming::ComputeNonAsyncInterleaveManyTotalTime(const Node& node) {
  DCHECK(timing_nodes_.contains(&node));
  auto inputs = node.inputs();
  auto input = inputs.begin();
  double first_input_total_time = 0.0;
  // The total time of a node is scaled by its ratio to account for how many
  // elements it needs to produce for its output node to produce an element. If
  // this is an interleave node or a flat map node, then the ratio is not known
  // statically and is instead estimated using empirical data.
  if (absl::StartsWith(node.name(), kFlatMap) ||
      absl::StartsWith(node.name(), kInterleave)) {
    first_input_total_time = ComputeInterleaveManyFirstInputTotalTime(node);
    if (input != inputs.end()) {
      ++input;
    }
  }
  double input_total_time_nsec = first_input_total_time;
  for (; input != inputs.end(); ++input) {
    if ((*input)->IsAsync()) {
      continue;
    }
    if (!(*input)->autotune() || (*input)->num_elements() <= 0) {
      continue;
    }
    DCHECK(timing_nodes_.contains((*input).get()))
        << "Input " << (*input)->long_name() << " of node " << node.long_name()
        << " has no timing node.";
    input_total_time_nsec +=
        timing_nodes_[(*input).get()].total_time_nsec * node.Ratio();
  }
  auto& node_timing = timing_nodes_[&node];
  node_timing.total_time_nsec =
      node_timing.self_time_nsec + input_total_time_nsec;
}

void ModelTiming::ComputeAsyncInterleaveManyTotalTime(const Node& node) {
  DCHECK(timing_nodes_.contains(&node));
  auto& node_timing = timing_nodes_[&node];
  node_timing.total_time_nsec =
      node_timing.self_time_nsec +
      ComputeInterleaveManyFirstInputTotalTime(node) +
      ComputeAsyncInterleaveManyInterleavedInputsTotalTime(node);
}

double ModelTiming::ComputeAsyncInterleaveManyInterleavedInputsTotalTime(
    const Node& node) {
  DCHECK(timing_nodes_.contains(&node));
  double max_input_total_time_nsec = 0.0;
  double sum_input_throughput = 0.0;
  auto inputs = node.inputs();
  // `ParallelInterleave` is often used to interleave processing of datasets
  // generated from the first input, e.g. reading from IO where the first input
  // has the list of all filenames. We skip it here to compute the total time of
  // the other inputs.
  auto input = std::next(inputs.begin());
  // `num_active_inputs` holds the number of inputs that the
  // `ParallelInterleave` is reading from, not including those that are warm
  // starting, which can be detected by checking the value of `autotune()`. It
  // also does not count async inputs because they would be in their own
  // stages. This number is typically the same as `cycle_length`. It will be
  // used below to scale the throughput of inputs if `cycle_length` is smaller
  // than `num_active_inputs`.
  int num_active_inputs = 0;
  for (; input != inputs.end(); ++input) {
    if ((*input)->IsAsync()) {
      continue;
    }
    if (!(*input)->autotune() || (*input)->num_elements() <= 0) {
      continue;
    }
    DCHECK(timing_nodes_.contains((*input).get()))
        << "Input " << (*input)->long_name() << " of node " << node.long_name()
        << " has no timing node.";
    auto input_total_time_nsec = timing_nodes_[(*input).get()].total_time_nsec;
    max_input_total_time_nsec =
        std::max(input_total_time_nsec, max_input_total_time_nsec);
    if (input_total_time_nsec > 0.0) {
      sum_input_throughput += 1.0 / input_total_time_nsec;
    }
    ++num_active_inputs;
  }
  auto parallelism_param = node.ParameterValue(kParallelism);
  double parallelism = 1.0;
  if (parallelism_param.ok()) {
    parallelism = parallelism_param.value();
  }
  // After cl/445005635, there should always be `deterministic` parameter for an
  // ASYNC_INTERLEAVE_MANY node. The "not-ok" check is to allow the code to work
  // with protos saved and restored before that CL. Similarly for `cycle_length`
  // with cl/436244658.
  auto deterministic_param = node.ParameterValue(kDeterministic);
  bool deterministic = false;
  if (deterministic_param.ok()) {
    deterministic = deterministic_param.value() == 1.0;
  }
  auto cycle_length_param = node.ParameterValue(kCycleLength);
  double cycle_length = num_active_inputs;
  if (cycle_length_param.ok()) {
    cycle_length = cycle_length_param.value();
  }
  double input_total_time_nsec = 0.0;
  if (deterministic) {
    // If deterministic = true, then the total time is `max input total time /
    // min(parallelism, cycle_length)`.
    input_total_time_nsec =
        max_input_total_time_nsec / std::min(parallelism, cycle_length);
  } else if (sum_input_throughput > 0.0) {
    // If deterministic = false, then the total time is
    // `1/sum_input_throughput`. Scale the throughput according to `parallelism`
    // and `cycle_length` if `cycle_length` or `parallelism` is smaller than
    // active inputs. `cycle_length` and `parallelism` could theoretically be
    // larger than active inputs when some inputs are async and some are sync.
    if (std::min(cycle_length, parallelism) < num_active_inputs) {
      sum_input_throughput *=
          std::min(parallelism, cycle_length) / num_active_inputs;
    }
    input_total_time_nsec = 1.0 / sum_input_throughput;
  }
  return input_total_time_nsec;
}

double ModelTiming::ComputeInterleaveManyFirstInputTotalTime(const Node& node) {
  DCHECK(timing_nodes_.contains(&node));
  // An interleave node is often used to interleave processing of datasets
  // generated from the first input, e.g. reading from IO where the first input
  // has the list of all filenames. The contribution of the first input total
  // time is proportional to the number of elements it produces over the number
  // elements the parallel interleave node produces.
  auto inputs = node.inputs();
  auto first_input = inputs.begin();
  if (first_input == inputs.end() || (*first_input)->IsAsync() ||
      !(*first_input)->autotune() || (*first_input)->num_elements() <= 0) {
    return 0.0;
  }
  DCHECK(timing_nodes_.contains((*first_input).get()))
      << "Input " << (*first_input)->long_name() << " of node "
      << node.long_name() << " has no timing node.";
  return timing_nodes_[(*first_input).get()].total_time_nsec *
         (*first_input)->num_elements() /
         (node.num_elements() + node.buffered_elements());
}

void ModelTiming::ComputeTotalTimes(const Node::NodeVector& reverse_bfs_nodes) {
  for (const auto& node : reverse_bfs_nodes) {
    ComputeNodeTotalTime(*(node));
  }
}

void ModelTiming::ComputeNodeTotalTime(const Node& node) {
  NodeTiming& node_timing = timing_nodes_[&node];
  node_timing.self_time_nsec = node.ComputeSelfTime();
  if (!node.autotune() || node.num_elements() <= 0) {
    return;
  }
  if (absl::StartsWith(node.name(), kParallelInterleave)) {
    ComputeAsyncInterleaveManyTotalTime(node);
    return;
  }
  ComputeNonAsyncInterleaveManyTotalTime(node);
}

std::vector<std::shared_ptr<Node>> ModelTiming::GetStageRoots() const {
  auto bfs_nodes = CollectNodes(root_, TraversalOrder::BFS, IsAnyNode);
  std::vector<std::shared_ptr<Node>> roots;
  if (!bfs_nodes.empty() && !bfs_nodes[0]->IsAsync()) {
    roots.push_back(bfs_nodes[0]);
  }
  for (auto& node : bfs_nodes) {
    if (node->IsAsync()) {
      roots.push_back(node);
    }
  }
  return roots;
}

std::vector<std::shared_ptr<Node>> ModelTiming::GetStageNodes(
    std::shared_ptr<Node> stage_root) const {
  return CollectNodes(stage_root, TraversalOrder::BFS, IsSyncNode);
}

}  // namespace model
}  // namespace data
}  // namespace tensorflow

} // export

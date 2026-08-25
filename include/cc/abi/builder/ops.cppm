module;

#include "c/extern/ops.h"

export module cc_abi_builder:ops;

import cc_abi_builder_intern;
import cc_abi_value;

export namespace ice {

// Owned wrapper for TF_OpDefinitionBuilder — plugin builds up an op definition and registers
// it via register_op(), which transfers ownership to the host's op registry.
class OpsBuilder {
 public:
  OpsBuilder() : m_handle{nullptr} {}
  explicit OpsBuilder(TF_OpDefinitionBuilder* handle) : m_handle{handle} {}
  ~OpsBuilder() { if (m_handle) TF_DeleteOpDefinitionBuilder(m_handle); }

  OpsBuilder(const OpsBuilder&) = delete;
  OpsBuilder& operator=(const OpsBuilder&) = delete;

  OpsBuilder(OpsBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  OpsBuilder& operator=(OpsBuilder&& other) noexcept {
    if (this != &other) {
      if (m_handle) TF_DeleteOpDefinitionBuilder(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  static OpsBuilder alloc(const StringBuilder& op_name) {
    return OpsBuilder{TF_NewOpDefinitionBuilder(op_name.c_str())};
  }

  void add_attr(const StringBuilder& attr_spec) { TF_OpDefinitionBuilderAddAttr(m_handle, attr_spec.c_str()); }
  void add_input(const StringBuilder& input_spec) { TF_OpDefinitionBuilderAddInput(m_handle, input_spec.c_str()); }
  void add_output(const StringBuilder& output_spec) { TF_OpDefinitionBuilderAddOutput(m_handle, output_spec.c_str()); }
  void set_is_commutative(bool is_commutative) { TF_OpDefinitionBuilderSetIsCommutative(m_handle, is_commutative); }
  void set_is_aggregate(bool is_aggregate) { TF_OpDefinitionBuilderSetIsAggregate(m_handle, is_aggregate); }
  void set_is_stateful(bool is_stateful) { TF_OpDefinitionBuilderSetIsStateful(m_handle, is_stateful); }
  void set_allows_uninitialized_input(bool allows) { TF_OpDefinitionBuilderSetAllowsUninitializedInput(m_handle, allows); }
  void deprecated(int version, const StringBuilder& explanation) { TF_OpDefinitionBuilderDeprecated(m_handle, version, explanation.c_str()); }

  using ShapeInferenceFn = TF_ShapeInferenceFn;
  void set_shape_inference_function(ShapeInferenceFn fn) { TF_OpDefinitionBuilderSetShapeInferenceFunction(m_handle, fn); }

  void register_op(TF_Status* status) {
    TF_RegisterOpDefinition(m_handle, status);
    m_handle = nullptr;  // Ownership transferred
  }

  // Underlying handle — pass directly to the C ABI
  TF_OpDefinitionBuilder *get_handle() { return m_handle; }
  const TF_OpDefinitionBuilder *get_handle() const { return m_handle; }

 private:
  TF_OpDefinitionBuilder* m_handle;
};

// ShapeHandle/DimensionHandle/ShapeInferenceContextView — support types consumed *inside* a
// plugin-authored shape-inference callback body (set via OpsBuilder::set_shape_inference_function),
// operating on a TF_ShapeInferenceContext* the host constructs and passes in. They live with the
// builder side because that's whose code actually calls them, not because the plugin owns the
// context itself.

// Owned wrapper for TF_ShapeHandle
class ShapeHandle {
 public:
  ShapeHandle() : m_handle{nullptr} {}
  explicit ShapeHandle(TF_ShapeHandle* handle) : m_handle{handle} {}
  ~ShapeHandle() { if (m_handle) TF_DeleteShapeHandle(m_handle); }

  ShapeHandle(const ShapeHandle&) = delete;
  ShapeHandle& operator=(const ShapeHandle&) = delete;

  ShapeHandle(ShapeHandle&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  ShapeHandle& operator=(ShapeHandle&& other) noexcept {
    if (this != &other) {
      if (m_handle) TF_DeleteShapeHandle(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  static ShapeHandle create() { return ShapeHandle(TF_NewShapeHandle()); }

  // Underlying handle — pass directly to the C ABI
  TF_ShapeHandle *get_handle() { return m_handle; }
  const TF_ShapeHandle *get_handle() const { return m_handle; }

 private:
  TF_ShapeHandle* m_handle;
};

// Owned wrapper for TF_DimensionHandle
class DimensionHandle {
 public:
  DimensionHandle() : m_handle{nullptr} {}
  explicit DimensionHandle(TF_DimensionHandle* handle) : m_handle{handle} {}
  ~DimensionHandle() { if (m_handle) TF_DeleteDimensionHandle(m_handle); }

  DimensionHandle(const DimensionHandle&) = delete;
  DimensionHandle& operator=(const DimensionHandle&) = delete;

  DimensionHandle(DimensionHandle&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  DimensionHandle& operator=(DimensionHandle&& other) noexcept {
    if (this != &other) {
      if (m_handle) TF_DeleteDimensionHandle(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  static DimensionHandle create() { return DimensionHandle(TF_NewDimensionHandle()); }

  bool value_known() const { return TF_DimensionHandleValueKnown(m_handle); }
  int64_t value() const { return TF_DimensionHandleValue(m_handle); }

  // Underlying handle — pass directly to the C ABI
  TF_DimensionHandle *get_handle() { return m_handle; }
  const TF_DimensionHandle *get_handle() const { return m_handle; }

 private:
  TF_DimensionHandle* m_handle;
};

// Borrowed wrapper for TF_ShapeInferenceContext
class ShapeInferenceContextView {
 public:
  explicit ShapeInferenceContextView(TF_ShapeInferenceContext* handle) : m_handle{handle} {}
  ~ShapeInferenceContextView() = default;

  ShapeInferenceContextView(const ShapeInferenceContextView&) = delete;
  ShapeInferenceContextView& operator=(const ShapeInferenceContextView&) = delete;

  ShapeInferenceContextView(ShapeInferenceContextView&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  ShapeInferenceContextView& operator=(ShapeInferenceContextView&& other) noexcept {
    if (this != &other) { m_handle = other.m_handle; other.m_handle = nullptr; }
    return *this;
  }

  int64_t num_inputs() const { return TF_ShapeInferenceContextNumInputs(m_handle); }

  ShapeHandle get_input(int i, TF_Status* status) {
    ShapeHandle handle = ShapeHandle::create();
    TF_ShapeInferenceContextGetInput(m_handle, i, handle.get_handle(), status);
    return handle;
  }

  void set_output(int i, const ShapeHandle& handle, TF_Status* status) {
    TF_ShapeInferenceContextSetOutput(m_handle, i, const_cast<TF_ShapeHandle *>(handle.get_handle()), status);
  }

  ShapeHandle scalar() { return ShapeHandle(TF_ShapeInferenceContextScalar(m_handle)); }
  ShapeHandle vector_from_size(size_t size) { return ShapeHandle(TF_ShapeInferenceContextVectorFromSize(m_handle, size)); }

  void get_attr_type(const StringBuilder& attr_name, DataType* val, TF_Status* status) {
    TF_DataType raw = static_cast<TF_DataType>(0);
    TF_ShapeInferenceContext_GetAttrType(m_handle, attr_name.c_str(), &raw, status);
    *val = static_cast<DataType>(raw);
  }

  int64_t rank(const ShapeHandle& handle) const { return TF_ShapeInferenceContextRank(m_handle, const_cast<TF_ShapeHandle *>(handle.get_handle())); }
  int rank_known(const ShapeHandle& handle) const { return TF_ShapeInferenceContextRankKnown(m_handle, const_cast<TF_ShapeHandle *>(handle.get_handle())); }

  void with_rank(const ShapeHandle& handle, int64_t rank, ShapeHandle* result, TF_Status* status) {
    TF_ShapeInferenceContextWithRank(m_handle, const_cast<TF_ShapeHandle *>(handle.get_handle()), rank, result->get_handle(), status);
  }

  void with_rank_at_least(const ShapeHandle& handle, int64_t rank, ShapeHandle* result, TF_Status* status) {
    TF_ShapeInferenceContextWithRankAtLeast(m_handle, const_cast<TF_ShapeHandle *>(handle.get_handle()), rank, result->get_handle(), status);
  }

  void with_rank_at_most(const ShapeHandle& handle, int64_t rank, ShapeHandle* result, TF_Status* status) {
    TF_ShapeInferenceContextWithRankAtMost(m_handle, const_cast<TF_ShapeHandle *>(handle.get_handle()), rank, result->get_handle(), status);
  }

  void dim(const ShapeHandle& shape_handle, int64_t i, DimensionHandle* result) {
    TF_ShapeInferenceContextDim(m_handle, const_cast<TF_ShapeHandle *>(shape_handle.get_handle()), i, result->get_handle());
  }

  void subshape(const ShapeHandle& shape_handle, int64_t start, int64_t end, ShapeHandle* result, TF_Status* status) {
    TF_ShapeInferenceContextSubshape(m_handle, const_cast<TF_ShapeHandle *>(shape_handle.get_handle()), start, end, result->get_handle(), status);
  }

  void set_unknown_shape(TF_Status* status) { TF_ShapeInferenceContextSetUnknownShape(m_handle, status); }

  bool dimension_value_known(const DimensionHandle& dim_handle) const { return TF_DimensionHandleValueKnown(const_cast<TF_DimensionHandle *>(dim_handle.get_handle())); }
  int64_t dimension_value(const DimensionHandle& dim_handle) const { return TF_DimensionHandleValue(const_cast<TF_DimensionHandle *>(dim_handle.get_handle())); }

  void concatenate_shapes(const ShapeHandle& first, const ShapeHandle& second, ShapeHandle* result, TF_Status* status) {
    TF_ShapeInferenceContextConcatenateShapes(m_handle, const_cast<TF_ShapeHandle *>(first.get_handle()), const_cast<TF_ShapeHandle *>(second.get_handle()), result->get_handle(), status);
  }

  // Underlying handle — pass directly to the C ABI
  TF_ShapeInferenceContext *get_handle() { return m_handle; }
  const TF_ShapeInferenceContext *get_handle() const { return m_handle; }

 private:
  TF_ShapeInferenceContext* m_handle;
};

}  // namespace ice

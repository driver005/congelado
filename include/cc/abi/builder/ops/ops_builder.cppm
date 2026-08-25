module;

#include "c/extern/ops.h"

export module cc_abi_builder:ops_builder;

import cc_abi_builder_intern;

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

} // namespace ice

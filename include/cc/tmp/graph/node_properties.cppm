module;

/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/op_def.pb.h"
#include "tensorflow/core/framework/op_def_builder.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/framework/node_def_util.h"
#include "tensorflow/core/framework/op.h"

export module cc_tmp:graph_node_properties;

import std;
import cc_abi;

export {

namespace tensorflow {

class OpRegistryInterface;

struct NodeProperties {
 public:
  NodeProperties(const OpDef* op_def, NodeDef node_def,
                 const DataTypeSlice inputs, const DataTypeSlice outputs)
      : NodeProperties(op_def, std::move(node_def),
                       DataTypeVector(inputs.begin(), inputs.end()),
                       DataTypeVector(outputs.begin(), outputs.end())) {}

  NodeProperties(const OpDef* _op_def, NodeDef&& _node_def,
                 DataTypeVector inputs, DataTypeVector outputs)
      : op_def(_op_def),
        node_def(std::move(_node_def)),
        input_types(std::move(inputs)),
        input_types_slice(input_types),
        output_types(std::move(outputs)),
        output_types_slice(output_types) {}

  // Resets the 'props' shared pointer to point to a new NodeProperties created
  // from the given NodeDef. 'op_registry' is used to look up the OpDef
  // corresponding to node_def.op(). Returns an error if OpDef lookup or
  // creation failed.
  static Status CreateFromNodeDef(NodeDef node_def,
                                  const OpRegistryInterface* op_registry,
                                  std::shared_ptr<const NodeProperties>* props);

  const OpDef* op_def;  // not owned.
  NodeDef node_def;
  DataTypeVector input_types;
  DataTypeSlice input_types_slice;
  DataTypeVector output_types;
  DataTypeSlice output_types_slice;
};

}  // namespace tensorflow

// ==================================================================
// Implementation: node_properties.cc
// ==================================================================

namespace tensorflow {

// static
absl::Status NodeProperties::CreateFromNodeDef(
    NodeDef node_def, const OpRegistryInterface* op_registry,
    std::shared_ptr<const NodeProperties>* props) {
  const OpDef* op_def;
  TF_RETURN_IF_ERROR(op_registry->LookUpOpDef(node_def.op(), &op_def));
  DataTypeVector input_types;
  DataTypeVector output_types;
  TF_RETURN_IF_ERROR(
      InOutTypesForNode(node_def, *op_def, &input_types, &output_types));
  props->reset(new NodeProperties(op_def, std::move(node_def),
                                  std::move(input_types),
                                  std::move(output_types)));
  return absl::OkStatus();
}

}  // namespace tensorflow

} // export

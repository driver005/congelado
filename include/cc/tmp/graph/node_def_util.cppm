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

#include <string>
#include <unordered_set>
#include <vector>
#include "tensorflow/core/framework/attr_value_util.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/op_def.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/gtl/array_slice.h"
#include "tensorflow/core/lib/gtl/flatmap.h"
#include "tensorflow/core/lib/hash/hash.h"
#include "tensorflow/core/platform/hash.h"
#include "tensorflow/core/platform/protobuf.h"
#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/platform/stringpiece.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/padding.h"
#include <algorithm>
#include <unordered_map>
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/op_def_util.h"
#include "tensorflow/core/framework/tensor.pb.h"
#include "tensorflow/core/framework/tensor_shape.pb.h"
#include "tensorflow/core/lib/gtl/map_util.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/scanner.h"
#include "tensorflow/core/platform/strcat.h"

export module cc_tmp:graph_node_def_util;

import std;
import cc_abi;

export {

namespace tensorflow {

class AttrSlice;
// We forward declare protos so that kernels don't need to depend on them
class OpDef;
class AttrValue;
class NameAttrList;
class TensorProto;
class TensorShapeProto;

// Name of the attribute used to encode node colocation constraints.
//
// Nodes can be co-located on the same device. Desire for explicit co-location
// is described by list(string) attribute containing the name of colocation
// groups.
extern const char* const kColocationAttrName;

// String prefix applied to the operation name for colocation constraints.
extern const char* const kColocationGroupPrefix;

// Constants for host CPU staging op for TPUExecute.
extern const char* const kTpuExecuteStagingOp;
extern const char* const kTpuExecuteStagingNodeName;

// Produce a human-readable version of a Node or NodeDef that is more concise
// than a text-format proto.
//
// The parameter `max_inputs_in_summary` specifies how many inputs at most to
// serialize in the output (in order not to get a string which is overly large).
// The value `-1` specifies that all inputs will be shown.
std::string SummarizeNodeDef(const NodeDef& node_def,
                             int max_inputs_in_summary = -1);
std::string SummarizeAttrs(const NodeDef& node_def);
std::string SummarizeAttrsHelper(AttrSlice attrs, absl::string_view device);

// Produces a formatted string pattern from the node which can uniquely identify
// this node upstream to produce an informative error message. The pattern
// followed is: {{node <node_name>}}
std::string FormatNodeDefForError(const NodeDef& node_def);
std::string FormatNodeDefForError(
    absl::string_view node_name, bool has_experimental_debug_info,
    const NodeDef_ExperimentalDebugInfo& experimental_debug_info);

typedef protobuf::Map<std::string, AttrValue> AttrValueMap;

// Adds an attr with name <name> and value <value> to *node_def.
// The type of the attr is based on the type of value.
void AddNodeAttr(absl::string_view name, const AttrValue& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, AttrValue&& value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::string_view value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const char* value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, int32_t value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, int64_t value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, float value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, double value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, bool value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, DataType value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const PartialTensorShape& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const Tensor& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const TensorProto& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const NameAttrList& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name,
                 absl::Span<const absl::string_view> value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const char* const> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const std::string> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const int32_t> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const int64_t> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const float> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const bool> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, const std::vector<bool>& value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const DataType> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const TensorShape> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name,
                 absl::Span<const PartialTensorShape> value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name,
                 absl::Span<const TensorShapeProto> value, NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const Tensor> value,
                 NodeDef* node_def);
void AddNodeAttr(absl::string_view name, absl::Span<const NameAttrList> value,
                 NodeDef* node_def);

// Version to workaround C++'s "perfect" forwarding not being able to
// forward {...} initialization.
template <class T>
void AddNodeAttr(absl::string_view name, std::initializer_list<T> value,
                 NodeDef* node_def) {
  AddNodeAttr(name, gtl::ArraySlice<T>(value), node_def);
}

// Adds an attr to an attr value map.
void AddAttr(absl::string_view name, const AttrValue& value, AttrValueMap* map);
void AddAttr(absl::string_view name, bool value, AttrValueMap* map);

class AttrSlice {
 public:
  AttrSlice(const NodeDef& node_def);  // NOLINT(runtime/explicit)

  AttrSlice();  // Empty
  explicit AttrSlice(const AttrValueMap* a);

  int size() const { return attrs()->size(); }

  // Returns the attr with attr_name if found.  Otherwise, returns
  // nullptr.
  const AttrValue* Find(absl::string_view attr_name) const;
  const AttrValue* FindByString(const std::string& attr_name) const;

  // Returns the attr_value for attr_name if found. Otherwise, returns a
  // NotFound status.
  absl::Status Find(absl::string_view attr_name,
                    const AttrValue** attr_value) const;
  absl::Status FindByString(const std::string& attr_name,
                            const AttrValue** attr_value) const;

  // Helper class to avoid allocations in EqualAttrs.
  // TODO(irving): Will go away once NodeInfo is used.
  struct Scratch {
    std::string a;
    std::string b;
  };

  // Check if all attrs and attr values match.  Does not take defaults into
  // account.
  //
  // TODO(irving): There is a bug in this routine inherited from its
  // OptimizerCSE::EqualAttrs predecessor.  The same tensor attr can be
  // represented in more than one way as an AttrValue, since TensorProto is
  // not 1-1.  This bug will go away once I replace everything with NodeInfo,
  // which stores a Tensor object directly.  The Scratch object will also go
  // away.
  bool EqualAttrs(AttrSlice other, Scratch* scratch) const;

  // If this AttrSlice has an attached NodeDef, summarize it.  This is for
  // error messages only: we intentionally do not provide direct access to the
  // NodeDef, since it is not always there.
  std::string SummarizeNode() const;

  // Iteration over all attrs
  AttrValueMap::const_iterator begin() const { return attrs()->begin(); }
  AttrValueMap::const_iterator end() const { return attrs()->end(); }

  std::string DebugString() const;

 private:
  const AttrValueMap* attrs() const {
    return ndef_ != nullptr ? &ndef_->attr() : attrs_;
  }

  absl::Status CheckFind(absl::string_view attr_name,
                         const AttrValue* attr_value) const;

  const NodeDef* ndef_;
  const AttrValueMap* attrs_;
};

// Return true if the attr with the name attr_name is defined in node_def.
bool HasNodeAttr(const NodeDef& node_def, absl::string_view attr_name);

// Look up the attr with name attr_name and set *value to its value.  If no
// attr with attr_name is found in node_def, or the attr does not have
// a matching type, a non-ok status will be returned.
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::string* value);  // type: "string"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         tstring* value);  // type: "tstring"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         int64_t* value);  // type: "int"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         int32_t* value);  // type: "int"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         float* value);  // type: "float"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         bool* value);  // type: "bool"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         DataType* value);  // type: "type"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         TensorShapeProto* value);  // type: "shape"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         TensorShape* value);  // type: "shape"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         PartialTensorShape* value);  // type: "shape"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         Tensor* value);  // type: "tensor"
absl::Status GetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<std::string>* value);  // type "list(string)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<tstring>* value);  // type "list(tstring)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<int64_t>* value);  // type "list(int)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<int32_t>* value);  // type "list(int)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<float>* value);  // type "list(float)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<bool>* value);  // type "list(bool)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<DataType>* value);  // type "list(type)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         DataTypeVector* value);  // type "list(type)"
absl::Status GetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<TensorShapeProto>* value);  // type "list(shape)"
absl::Status GetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<TensorShape>* value);  // type "list(shape)"
absl::Status GetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<PartialTensorShape>* value);  // type "list(shape)"
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         std::vector<Tensor>* value);  // type: "list(tensor)"

template <typename T>
StatusOr<T> GetNodeAttr(const NodeDef& ndef, absl::string_view attr_name) {
  T val;
  TF_RETURN_IF_ERROR(GetNodeAttr(ndef, attr_name, &val));
  return val;
}

// This version avoids copying the TensorProto.
// REQUIRES: Must not use *value beyond the lifetime of node_def.
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         const TensorProto** value);  // type: "tensor"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    const TensorProto** value);  // type: "tensor"

// This version avoids copying the NameAttrList.
// REQUIRES: Must not use *value beyond the lifetime of node_def.
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         const NameAttrList** value);  // type: "func"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    const NameAttrList** value);  // type: "func"

// These versions copies the NameAttrList(s).
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         NameAttrList* value);  // type: "func"
absl::Status GetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<NameAttrList>* value);  // type: "list(func)"

// Look up the attr with name attr_name and set *value to its value.  If no
// attr with attr_name is found in node_def, or the attr does not have
// a matching type, false is returned.
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::string* value);  // type: "string"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    int64_t* value);  // type: "int"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<int64_t>* value);  // type: "int"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    int32_t* value);  // type: "int"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    float* value);  // type: "float"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    bool* value);  // type: "bool"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    DataType* value);  // type: "type"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    TensorShape* value);  // type: "shape"

bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<std::string>* value);  // type: "list(string)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<tstring>* value);  // type: "list(tstring)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<int32_t>* value);  // type: "list(int)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<float>* value);  // type: "list(float)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<bool>* value);  // type: "list(bool)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<DataType>* value);  // type: "list(type)"
bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<TensorShape> value);  // type: "shape"

// Overloads of TryGetNodeAttr() that avoid copying the non-POD attribute
// values.
bool TryGetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<const std::string*>* value);  // type: "list(string)"
bool TryGetNodeAttr(
    const AttrSlice& attrs, absl::string_view attr_name,
    std::vector<const TensorShapeProto*>* value);  // type: "list(shape)"

// Look up the attr with name attr_name and return a reference to its value.
// If no attr with attr_name is found in node_def, or the attr does not have
// a matching type, a reference to an empty string is returned.
// REQUIRES: Must not use the returned value beyond the lifetime of node_def.
const std::string& GetNodeAttrString(const AttrSlice& attrs,
                                     absl::string_view attr_name);

// Specialization to parse an attribute directly into a Padding enum.
absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         Padding* value);

// Computes the input type for a specific node input.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status InputTypeForNode(const NodeDef& node_def, const OpDef& op_def,
                              int input_port, DataType* input_type);
// Computes the input types for a specific node.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status InputTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                               DataTypeVector* inputs);
// Computes the output type for a specific node output.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status OutputTypeForNode(const NodeDef& node_def, const OpDef& op_def,
                               int output_port, DataType* output_type);
// Computes the output types for a specific node.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status OutputTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                                DataTypeVector* outputs);
absl::Status OutputTypesForNode(const AttrSlice& attrs, const OpDef& op_def,
                                DataTypeVector* outputs);

// Computes the input and output types for a specific node.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status InOutTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                               DataTypeVector* inputs, DataTypeVector* outputs);
// Computes the number of outputs for a specific node.
// REQUIRES: ValidateOpDef(op_def).ok()
absl::Status NumOutputsForNode(const NodeDef& node_def, const OpDef& op_def,
                               int* num_outputs);

// Map a node/op's input/output port_id to arg_id.
//
// The port_id refers to the n-th tensor of the node, while the arg_id refers to
// the n-th arg of the op. These two can be different if an op's arg is a list
// of tensors.
//
// We return -1 for any invalid port_id (i.e., no corresponding arg_id).
int OpPortIdToArgId(const NodeDef& node,
                    const protobuf::RepeatedPtrField<OpDef::ArgDef>& args,
                    int port_id);

// Validates that the NodeDef:
// * Defines all expected attrs from the OpDef.
// * All attrs satisfies constraints from the OpDef.
// * Has a signature matching SignatureForNode().
// etc.
absl::Status ValidateNodeDef(const NodeDef& node_def, const OpDef& op_def);

// Computes the mapping from input/output argument name to the
// corresponding input/output index range.  For example,
// input "foo" corresponds to input indices
//   [ (*inputs)["foo"].first, (*inputs)["foo"].second ).
// NOTE(mrry): To reduce allocations when the map is used and save
// space, the returned `NameRangeMap` objects borrow the input/output
// argument names from `op_def`. The `op_def` must outlive the
// returned `NameRangeMap` objects.
typedef gtl::FlatMap<absl::string_view, std::pair<int, int>,
                     hash<absl::string_view>>
    NameRangeMap;
absl::Status NameRangesForNode(const AttrSlice& attrs, const OpDef& op_def,
                               NameRangeMap* inputs, NameRangeMap* outputs);
// Adds default values to *node_def for unspecified attrs from op_def.
void AddDefaultsToNodeDef(const OpDef& op_def, NodeDef* node_def);

// Remove attributes from node_def when the value is the default from the
// op_def.
void StripDefaultsFromNodeDef(const OpDef& op_def, NodeDef* node_def);

// Validates the syntax of a NodeDef provided externally.
//
// The following is an EBNF-style syntax for NodeDef objects. Note that
// Node objects are actually specified as tensorflow::NodeDef protocol buffers,
// which contain many other fields that are not (currently) validated.
//
// Node         = NodeName, Inputs
// Inputs       = ( DataInput * ), ( ControlInput * )
// DataInput    = NodeName, ( ":", [1-9], [0-9] * ) ?
// ControlInput = "^", NodeName
// NodeName     = [A-Za-z0-9.], [A-Za-z0-9_./] *
absl::Status ValidateExternalNodeDefSyntax(const NodeDef& node_def);

// Returns "status" with formatted NodeDef attached as additional text
// in the error message. If 'allow_multiple_formatted_node' is false and there
// is already a formatted NodeDef present in 'status', we simply attach the name
// of the NodeDef instead of the formatted string.
absl::Status AttachDef(const absl::Status& status, const NodeDef& node_def,
                       bool allow_multiple_formatted_node = false);
// Appends the given prefix and suffix to the original node name in order to
// make the name unique. If it's an "Enter" node and uniquify_frame_name is
// true, use the same way to reset attribute "frame_name".
absl::Status AddPrefixAndSuffixToNode(absl::string_view prefix,
                                      absl::string_view suffix,
                                      NodeDef* node_def,
                                      bool uniquify_frame_name = true);

// Appends the given prefix to the colocation group name if the name exists
// in `to_match`.
absl::Status MaybeAddPrefixToColocationConstraints(
    const std::unordered_set<std::string>& match, absl::string_view prefix,
    NodeDef* node_def);

// Updates the colocation constraint name with the one provided in the map (if
// it exists in the map) for node_def.
absl::Status MaybeUpdateColocationConstraintsWithMap(
    const std::map<absl::string_view, absl::string_view>& node_name_map,
    NodeDef* node_def);

// For replacing a existing node with a NoOp, change the op and clear full type
// information (since a NoOp has no output). Note that (duplicate control or
// all) inputs, (regular, output or all) attributes and output properperties are
// NOT cleared (and should be cleared if appropriate elsewhere).
void ChangeToNoOp(NodeDef* node_def);

}  // namespace tensorflow

// ==================================================================
// Implementation: node_def_util.cc
// ==================================================================

namespace tensorflow {

const char* const kColocationAttrName = "_class";
const char* const kColocationGroupPrefix = "loc:@";
// For TPU distributed rewrite, TPU args are collected and "staged" on the local
// host using an IdentityN TF op. Some args may result from a remote source.
// When all arg tensors are available, the TPUExecute op can be inovoked. See
// DistributedTPURewritePass for more details.
const char* const kTpuExecuteStagingOp = "IdentityN";
const char* const kTpuExecuteStagingNodeName = "_variable_copy";

AttrSlice::AttrSlice() : ndef_(nullptr) {
  static const AttrValueMap* const kEmptyAttrValueMap = new AttrValueMap;
  attrs_ = kEmptyAttrValueMap;
}

// Do not cache the map field reference because that may be invalidated on
// Clear.
AttrSlice::AttrSlice(const NodeDef& node_def)
    : ndef_(&node_def), attrs_(nullptr) {}

AttrSlice::AttrSlice(const AttrValueMap* a) : ndef_(nullptr), attrs_(a) {}

std::string SummarizeAttrsHelper(AttrSlice attrs, absl::string_view device) {
  std::string ret;

  // We sort the attrs so the output is deterministic.
  std::vector<std::string> attr_names;
  attr_names.reserve(attrs.size());
  for (const auto& attr : attrs) {
    attr_names.push_back(attr.first);
  }
  std::sort(attr_names.begin(), attr_names.end());
  bool first = true;
  for (const std::string& attr_name : attr_names) {
    if (!first) absl::StrAppend(&ret, ", ");
    first = false;
    absl::StrAppend(&ret, attr_name, "=",
                    SummarizeAttrValue(*attrs.Find(attr_name)));
  }

  // Consider the device to be a final attr with name "_device".
  if (!device.empty()) {
    if (!first) absl::StrAppend(&ret, ", ");
    first = false;
    absl::StrAppend(&ret, "_device=\"", device, "\"");
  }
  return ret;
}

std::string AttrSlice::SummarizeNode() const {
  return ndef_
             ? SummarizeNodeDef(*ndef_)
             : absl::StrCat(
                   "[", SummarizeAttrsHelper(*this, absl::string_view()), "]");
}

std::string AttrSlice::DebugString() const {
  std::vector<std::string> attr_key_vals;
  attr_key_vals.reserve(attrs()->size());
  for (const auto& it : *this) {
    const std::string& name = it.first;
    const AttrValue& attr_value = it.second;
    attr_key_vals.push_back(
        absl::StrCat(name, "=", SummarizeAttrValue(attr_value)));
  }
  return absl::StrJoin(attr_key_vals, ", ");
}

std::string SummarizeNodeDef(const NodeDef& node_def,
                             int max_inputs_in_summary) {
  std::string ret =
      absl::StrCat(errors::FormatNodeNameForError(node_def.name()), " = ",
                   node_def.op(), "[");
  absl::StrAppend(&ret, SummarizeAttrsHelper(node_def, node_def.device()));
  absl::StrAppend(&ret, "](");

  // Output inputs, including control inputs, verbatim.
  bool first = true;
  for (const std::string& input : node_def.input()) {
    if (!first) absl::StrAppend(&ret, ", ");
    first = false;
    if (max_inputs_in_summary-- == 0) {
      absl::StrAppend(&ret, "...");
      break;
    }
    absl::StrAppend(&ret, input);
  }
  absl::StrAppend(&ret, ")");
  return ret;
}

std::string SummarizeAttrs(const NodeDef& node_def) {
  return SummarizeAttrsHelper(node_def, node_def.device());
}

std::string FormatNodeDefForError(
    absl::string_view node_name, bool has_experimental_debug_info,
    const NodeDef_ExperimentalDebugInfo& experimental_debug_info) {
  return !has_experimental_debug_info ||
                 experimental_debug_info.original_node_names().empty()
             ? errors::FormatNodeNameForError(node_name)
             : errors::FormatOriginalNodeLocationForError(
                   experimental_debug_info.original_node_names(),
                   experimental_debug_info.original_func_names());
}

std::string FormatNodeDefForError(const NodeDef& node_def) {
  return FormatNodeDefForError(node_def.name(),
                               node_def.has_experimental_debug_info(),
                               node_def.experimental_debug_info());
}

const AttrValue* AttrSlice::Find(absl::string_view attr_name) const {
  // Currently, the collection used for NodeDef::attr() (google::protobuf::Map)
  // requires that the keys used for lookups have type 'const string&'. Because
  // this method takes a StringPiece, it is necessary to allocate a temporary
  // string, copy attr_name to it, and then use that temporary string for the
  // lookup. This causes an excessive number of short-lived allocations, and for
  // large graphs, this can be a significant cost.
  //
  // Because most nodes have a small number of attributes, a simple linear scan
  // is generally more efficient than a hashed lookup.  If google::protobuf::Map
  // changes so that it supports efficient lookups using StringPiece instead of
  // const string&, then this code could be changed to use attrs()->find()
  // again.

  for (const auto& attr : *attrs()) {
    if (attr.first == attr_name) {
      return &attr.second;
    }
  }
  return nullptr;
}

const AttrValue* AttrSlice::FindByString(const std::string& attr_name) const {
  auto iter = attrs()->find(attr_name);
  if (iter != attrs()->end()) {
    return &iter->second;
  } else {
    return nullptr;
  }
}

absl::Status AttrSlice::CheckFind(absl::string_view attr_name,
                                  const AttrValue* attr_value) const {
  if (attr_value != nullptr) {
    return absl::OkStatus();
  }
  absl::Status s = absl::NotFoundError(
      absl::StrCat("No attr named '", attr_name, "' in NodeDef:"));
  // Skip AttachDef for internal attrs since it is a little bit
  // expensive and it is common for them to correctly not be included
  // in a NodeDef.
  if (!absl::StartsWith(attr_name, "_") && ndef_ != nullptr) {
    s = AttachDef(s, *ndef_);
  }
  return s;
}

absl::Status AttrSlice::Find(absl::string_view attr_name,
                             const AttrValue** attr_value) const {
  *attr_value = Find(attr_name);
  return CheckFind(attr_name, *attr_value);
}

absl::Status AttrSlice::FindByString(const std::string& attr_name,
                                     const AttrValue** attr_value) const {
  *attr_value = FindByString(attr_name);
  return CheckFind(attr_name, *attr_value);
}

bool AttrSlice::EqualAttrs(AttrSlice other, Scratch* scratch) const {
  if (size() != other.size()) return false;

  for (const auto& attr : *other.attrs()) {
    auto iter = attrs()->find(attr.first);
    if (iter == attrs()->end()) return false;
    // TODO(irving): Comparing AttrValues by proto is slightly buggy, since
    // TensorProto is a nonunique representation of Tensor.  This bug will go
    // away once AttrSlice switches over to NodeInfo.
    iter->second.SerializeToString(&scratch->a);
    attr.second.SerializeToString(&scratch->b);
    if (scratch->a != scratch->b) return false;
  }
  return true;
}

// The ... is to allow the caller to inject some value validation code.  Use
// just ; if no additional validation code is needed.
#define DEFINE_GET_ATTR(TYPE, FIELD, ATTR_TYPE, APPEND_OP, CAST, ...)         \
  Status GetNodeAttr(const AttrSlice& attrs, StringPiece attr_name,           \
                     TYPE* value) {                                           \
    const AttrValue* attr_value;                                              \
    TF_RETURN_IF_ERROR(attrs.Find(attr_name, &attr_value));                   \
    TF_RETURN_IF_ERROR(AttrValueHasType(*attr_value, ATTR_TYPE));             \
    const auto& v = attr_value->FIELD();                                      \
    __VA_ARGS__;                                                              \
    *value = CAST;                                                            \
    return OkStatus();                                                        \
  }                                                                           \
  Status GetNodeAttr(const AttrSlice& attrs, StringPiece attr_name,           \
                     std::vector<TYPE>* value) {                              \
    const AttrValue* attr_value;                                              \
    TF_RETURN_IF_ERROR(attrs.Find(attr_name, &attr_value));                   \
    TF_RETURN_IF_ERROR(AttrValueHasType(*attr_value, "list(" ATTR_TYPE ")")); \
    value->reserve(attr_value->list().FIELD().size());                        \
    for (const auto& v : attr_value->list().FIELD()) {                        \
      __VA_ARGS__;                                                            \
      value->APPEND_OP(CAST);                                                 \
    }                                                                         \
    return OkStatus();                                                        \
  }

#define DEFINE_TRY_GET_ATTR(TYPE, FIELD, ATTR_TYPE, APPEND_OP, CAST, ...) \
  bool TryGetNodeAttr(const AttrSlice& attrs, StringPiece attr_name,      \
                      TYPE* value) {                                      \
    const AttrValue* attr_value = attrs.Find(attr_name);                  \
    if (attr_value == nullptr) {                                          \
      return false;                                                       \
    }                                                                     \
    Status s = AttrValueHasType(*attr_value, ATTR_TYPE);                  \
    if (!s.ok()) {                                                        \
      return false;                                                       \
    }                                                                     \
    const auto& v = attr_value->FIELD();                                  \
    __VA_ARGS__;                                                          \
    *value = CAST;                                                        \
    return true;                                                          \
  }                                                                       \
  bool TryGetNodeAttr(const AttrSlice& attrs, StringPiece attr_name,      \
                      std::vector<TYPE>* value) {                         \
    const AttrValue* attr_value = attrs.Find(attr_name);                  \
    if (attr_value == nullptr) {                                          \
      return false;                                                       \
    }                                                                     \
    Status s = AttrValueHasType(*attr_value, "list(" ATTR_TYPE ")");      \
    if (!s.ok()) {                                                        \
      return false;                                                       \
    }                                                                     \
    value->reserve(attr_value->list().FIELD().size());                    \
    for (const auto& v : attr_value->list().FIELD()) {                    \
      __VA_ARGS__;                                                        \
      value->APPEND_OP(CAST);                                             \
    }                                                                     \
    return true;                                                          \
  }
DEFINE_GET_ATTR(tstring, s, "string", emplace_back, v, ;)
DEFINE_TRY_GET_ATTR(tstring, s, "string", emplace_back, v, ;)
DEFINE_GET_ATTR(std::string, s, "string", emplace_back, v, ;)
DEFINE_TRY_GET_ATTR(std::string, s, "string", emplace_back, v, ;)
DEFINE_GET_ATTR(int64_t, i, "int", emplace_back, v, ;)
DEFINE_TRY_GET_ATTR(int64_t, i, "int", emplace_back, v, ;)
DEFINE_GET_ATTR(
    int32_t, i, "int", emplace_back, static_cast<int32_t>(v),
    if (static_cast<int64_t>(static_cast<int32_t>(v)) != v) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Attr ", attr_name, " has value ", v, " out of range for an int32"));
    })
DEFINE_TRY_GET_ATTR(
    int32_t, i, "int", emplace_back, static_cast<int32_t>(v),
    if (static_cast<int64_t>(static_cast<int32_t>(v)) != v) {
      static int log_counter = 0;
      if (log_counter < 10) {
        log_counter++;
        LOG(WARNING) << "Attr " << attr_name << " has value " << v
                     << " out of range for an int32";
      }
      return false;
    })
DEFINE_GET_ATTR(float, f, "float", emplace_back, v, ;)
DEFINE_TRY_GET_ATTR(float, f, "float", emplace_back, v, ;)
DEFINE_GET_ATTR(bool, b, "bool", emplace_back, v, ;)
DEFINE_TRY_GET_ATTR(bool, b, "bool", emplace_back, v, ;)
DEFINE_GET_ATTR(DataType, type, "type", emplace_back, static_cast<DataType>(v),
                ;)
DEFINE_TRY_GET_ATTR(DataType, type, "type", emplace_back,
                    static_cast<DataType>(v),
                    ;)
DEFINE_GET_ATTR(TensorShapeProto, shape, "shape", emplace_back, v, ;)
DEFINE_GET_ATTR(TensorShape, shape, "shape", emplace_back, TensorShape(v),
                TF_RETURN_IF_ERROR(TensorShape::IsValidShape(v));)
DEFINE_TRY_GET_ATTR(
    TensorShape, shape, "shape", emplace_back, TensorShape(v),
    if (!TensorShape::IsValidShape(v).ok()) {
      static int log_counter = 0;
      if (log_counter < 10) {
        log_counter++;
        LOG(WARNING) << "Attr " << attr_name << " has invalid shape value "
                     << v.DebugString();
      }
      return false;
    })
DEFINE_GET_ATTR(PartialTensorShape, shape, "shape", emplace_back,
                PartialTensorShape(v),
                TF_RETURN_IF_ERROR(PartialTensorShape::IsValidShape(v));)
DEFINE_GET_ATTR(
    Tensor, tensor, "tensor", emplace_back, t, Tensor t; if (!t.FromProto(v)) {
      return absl::InvalidArgumentError(
          absl::StrCat("Attr ", attr_name, " has value ", v.ShortDebugString(),
                       " that can't be converted to a Tensor"));
    })
DEFINE_GET_ATTR(NameAttrList, func, "func", emplace_back, v, ;);
#undef DEFINE_GET_ATTR

bool HasNodeAttr(const NodeDef& node_def, absl::string_view attr_name) {
  return node_def.attr().find(std::string(attr_name)) != node_def.attr().end();
}

static const std::string& kEmptyString = *new std::string();

const std::string& GetNodeAttrString(const AttrSlice& attrs,
                                     absl::string_view attr_name) {
  const AttrValue* attr_value = attrs.Find(attr_name);
  if (attr_value == nullptr) {
    return kEmptyString;
  }
  absl::Status s = AttrValueHasType(*attr_value, "string");
  if (!s.ok()) {
    return kEmptyString;
  }
  return attr_value->s();
}

bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<const std::string*>* value) {
  const AttrValue* attr_value = attrs.Find(attr_name);
  if (attr_value == nullptr) {
    return false;
  }
  absl::Status s = AttrValueHasType(*attr_value, "list(string)");
  if (!s.ok()) {
    return false;
  }
  value->reserve(attr_value->list().s().size());
  for (const auto& v : attr_value->list().s()) {
    value->push_back(&v);
  }
  return true;
}

bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    std::vector<const TensorShapeProto*>* value) {
  const AttrValue* attr_value = attrs.Find(attr_name);
  if (attr_value == nullptr) {
    return false;
  }
  absl::Status s = AttrValueHasType(*attr_value, "list(shape)");
  if (!s.ok()) {
    return false;
  }
  value->reserve(attr_value->list().shape().size());
  for (const auto& v : attr_value->list().shape()) {
    value->push_back(&v);
  }
  return true;
}

absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         DataTypeVector* value) {
  const AttrValue* attr_value;
  TF_RETURN_IF_ERROR(attrs.Find(attr_name, &attr_value));
  TF_RETURN_IF_ERROR(AttrValueHasType(*attr_value, "list(type)"));
  for (const auto& v : attr_value->list().type()) {
    value->push_back(static_cast<DataType>(v));
  }
  return absl::OkStatus();
}

absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         const TensorProto** value) {
  const AttrValue* attr_value;
  TF_RETURN_IF_ERROR(attrs.Find(attr_name, &attr_value));
  TF_RETURN_IF_ERROR(AttrValueHasType(*attr_value, "tensor"));
  *value = &attr_value->tensor();
  return absl::OkStatus();
}

bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    const TensorProto** value) {
  const AttrValue* attr_value = attrs.Find(attr_name);
  if (attr_value == nullptr) {
    return false;
  }
  absl::Status s = AttrValueHasType(*attr_value, "tensor");
  if (!s.ok()) {
    return false;
  }
  *value = &attr_value->tensor();
  return true;
}

absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         const NameAttrList** value) {
  const AttrValue* attr_value;
  TF_RETURN_IF_ERROR(attrs.Find(attr_name, &attr_value));
  TF_RETURN_IF_ERROR(AttrValueHasType(*attr_value, "func"));
  *value = &attr_value->func();
  return absl::OkStatus();
}

bool TryGetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                    const NameAttrList** value) {
  const AttrValue* attr_value = attrs.Find(attr_name);
  if (attr_value == nullptr) {
    return false;
  }
  absl::Status s = AttrValueHasType(*attr_value, "func");
  if (!s.ok()) {
    return false;
  }
  *value = &attr_value->func();
  return true;
}

absl::Status GetNodeAttr(const AttrSlice& attrs, absl::string_view attr_name,
                         Padding* value) {
  std::string str_value;
  TF_RETURN_IF_ERROR(GetNodeAttr(attrs, attr_name, &str_value));
  return GetPaddingFromString(str_value, value);
}

namespace {  // Helper for InOutTypesForNode().

template <class NodeDefOrAttrSlice>
absl::Status AddArgToSig(const NodeDefOrAttrSlice& node_or_attrs,
                         const OpDef::ArgDef& arg_def, DataTypeVector* sig) {
  const int original_size = sig->size();
  if (!arg_def.number_attr().empty()) {
    // Same type repeated "repeats" times.
    int64_t repeats = -1;
    TF_RETURN_IF_ERROR(
        GetNodeAttr(node_or_attrs, arg_def.number_attr(), &repeats));
    // We can't handle outputs that are larger than int32 sizes.
    if (static_cast<int64_t>(static_cast<int32_t>(repeats)) != repeats) {
      return absl::InvalidArgumentError(
          absl::StrCat("Number of outputs is too big: ", repeats));
    }
    if (repeats < 0) {
      return absl::InvalidArgumentError(
          absl::StrCat("Value for number_attr() ", repeats, " < 0"));
    }

    if (!arg_def.type_attr().empty()) {
      DataType dtype;
      TF_RETURN_IF_ERROR(
          GetNodeAttr(node_or_attrs, arg_def.type_attr(), &dtype));
      for (int i = 0; i < repeats; ++i) {
        sig->push_back(dtype);
      }
    } else if (arg_def.type() != DT_INVALID) {
      for (int i = 0; i < repeats; ++i) {
        sig->push_back(arg_def.type());
      }
    } else {
      return absl::InvalidArgumentError(absl::StrCat(
          "Missing type or type_attr field in ", arg_def.ShortDebugString()));
    }
  } else if (!arg_def.type_attr().empty()) {
    const AttrValue* attr_value;
    TF_RETURN_IF_ERROR(AttrSlice(node_or_attrs)
                           .FindByString(arg_def.type_attr(), &attr_value));
    sig->push_back(attr_value->type());
  } else if (!arg_def.type_list_attr().empty()) {
    const AttrValue* attr_value;
    TF_RETURN_IF_ERROR(
        AttrSlice(node_or_attrs)
            .FindByString(arg_def.type_list_attr(), &attr_value));
    for (int dtype : attr_value->list().type()) {
      sig->push_back(static_cast<DataType>(dtype));
    }
  } else if (arg_def.type() != DT_INVALID) {
    sig->push_back(arg_def.type());
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("No type fields in ", arg_def.ShortDebugString()));
  }
  if (arg_def.is_ref()) {
    // For all types that were added by this function call, make them refs.
    for (size_t i = original_size; i < sig->size(); ++i) {
      if (IsRefType((*sig)[i])) {
        return absl::InvalidArgumentError(
            absl::StrCat("Requested reference to a reference type: ",
                         arg_def.ShortDebugString()));
      }
      (*sig)[i] = MakeRefType((*sig)[i]);
    }
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status InputTypeForNode(const NodeDef& node_def, const OpDef& op_def,
                              int input_port, DataType* input_type) {
  DataTypeVector input_types;
  for (const auto& arg : op_def.input_arg()) {
    TF_RETURN_IF_ERROR(AddArgToSig(node_def, arg, &input_types));
    int input_types_size = input_types.size();
    if (input_types_size > input_port) {
      const DataType dtype = input_types[input_port];
      *input_type = dtype;
      return absl::OkStatus();
    }
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "Input ", input_port, " not found for node ", node_def.name()));
}

absl::Status InputTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                               DataTypeVector* inputs) {
  for (const auto& arg : op_def.input_arg()) {
    TF_RETURN_IF_ERROR(AddArgToSig(node_def, arg, inputs));
  }
  return absl::OkStatus();
}

absl::Status OutputTypeForNode(const NodeDef& node_def, const OpDef& op_def,
                               int output_port, DataType* output_type) {
  DataTypeVector output_types;
  for (const auto& arg : op_def.output_arg()) {
    TF_RETURN_IF_ERROR(AddArgToSig(node_def, arg, &output_types));
    int output_types_size = output_types.size();
    if (output_types_size > output_port) {
      const DataType dtype = output_types[output_port];
      *output_type = dtype;
      return absl::OkStatus();
    }
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "Output ", output_port, " not found for node ", node_def.name()));
}

absl::Status OutputTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                                DataTypeVector* outputs) {
  for (const auto& arg : op_def.output_arg()) {
    TF_RETURN_IF_ERROR(AddArgToSig(node_def, arg, outputs));
  }
  return absl::OkStatus();
}

absl::Status OutputTypesForNode(const AttrSlice& attrs, const OpDef& op_def,
                                DataTypeVector* outputs) {
  for (const auto& arg : op_def.output_arg()) {
    TF_RETURN_IF_ERROR(AddArgToSig(attrs, arg, outputs));
  }
  return absl::OkStatus();
}

absl::Status InOutTypesForNode(const NodeDef& node_def, const OpDef& op_def,
                               DataTypeVector* inputs,
                               DataTypeVector* outputs) {
  TF_RETURN_IF_ERROR(InputTypesForNode(node_def, op_def, inputs));
  return OutputTypesForNode(node_def, op_def, outputs);
}

absl::Status NumOutputsForNode(const NodeDef& node_def, const OpDef& op_def,
                               int* num_outputs) {
  DataTypeVector outputs;
  TF_RETURN_IF_ERROR(OutputTypesForNode(node_def, op_def, &outputs));
  *num_outputs = outputs.size();
  return absl::OkStatus();
}

int OpPortIdToArgId(const NodeDef& node,
                    const protobuf::RepeatedPtrField<OpDef::ArgDef>& args,
                    int port_id) {
  for (int arg_id = 0; arg_id < args.size(); ++arg_id) {
    if (port_id < 0) {
      return -1;
    } else if (port_id == 0) {
      return arg_id;
    }

    // Default is 1 port per arg.
    int n = 1;

    const auto& arg = args.Get(arg_id);
    if (!arg.number_attr().empty()) {
      n = node.attr().at(arg.number_attr()).i();
    } else if (!arg.type_list_attr().empty()) {
      n = node.attr().at(arg.type_list_attr()).list().type_size();
    }

    if (n < 0) {
      // This should never happen.
      DCHECK_GE(n, 0);
      return -1;
    } else if (port_id < n) {
      return arg_id;
    }
    port_id -= n;
  }

  return -1;
}

absl::Status ValidateNodeDef(const NodeDef& node_def, const OpDef& op_def) {
  if (node_def.op() != op_def.name()) {
    return absl::InvalidArgumentError(
        absl::StrCat("NodeDef op '", node_def.op(), "' does not match ",
                     SummarizeOpDef(op_def),
                     "; NodeDef: ", FormatNodeDefForError(node_def)));
  }

  bool seen_control = false;
  size_t num_inputs = 0;
  // TODO(josh11b): Unify the input field validation.
  for (const std::string& input : node_def.input()) {
    if (absl::StartsWith(input, "^")) {
      seen_control = true;
      if (input.find(':') != std::string::npos) {
        return absl::InvalidArgumentError(absl::StrCat(
            "Control input '", input, "' must not have ':' in NodeDef: ",
            FormatNodeDefForError(node_def)));
      }
    } else if (seen_control) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Non-control input '", input, "' after control input in NodeDef: ",
          FormatNodeDefForError(node_def)));
    } else {
      ++num_inputs;
    }
  }

  std::unordered_map<std::string, const OpDef::AttrDef*> op_attrs;
  for (const auto& attr : op_def.attr()) {
    if (!gtl::InsertIfNotPresent(&op_attrs, attr.name(), &attr)) {
      return absl::InvalidArgumentError(
          absl::StrCat("OpDef has duplicate attr name '", attr.name(),
                       "': ", SummarizeOpDef(op_def)));
    }
  }
  for (const auto& attr : node_def.attr()) {
    // Allow internal optional attributes with names starting with "_".
    if (absl::StartsWith(attr.first, "_")) {
      continue;
    }
    auto iter = op_attrs.find(attr.first);
    if (iter == op_attrs.end()) {
      LOG_EVERY_N_SEC(ERROR, 5)
          << "NodeDef mentions attribute " << attr.first
          << " which is not in the op definition: " << SummarizeOpDef(op_def)
          << " This may be expected if your graph generating binary is newer "
          << " than this binary. Unknown attributes will be ignored."
          << " NodeDef: " << FormatNodeDefForError(node_def);
      continue;
    }

    // If attr value is placeholder, do not check it.
    if (attr.second.placeholder().empty()) {
      TF_RETURN_WITH_CONTEXT_IF_ERROR(
          ValidateAttrValue(attr.second, *iter->second),
          "; NodeDef: ", FormatNodeDefForError(node_def), "; ",
          SummarizeOpDef(op_def));
    }

    // Keep track of which attr names have (not) been found in the NodeDef.
    op_attrs.erase(iter);
  }

  // Were all attrs in the OpDef found in the NodeDef?
  if (!op_attrs.empty()) {
    std::string attrs;
    for (const auto& attr_pair : op_attrs) {
      if (!attrs.empty()) absl::StrAppend(&attrs, "', '");
      absl::StrAppend(&attrs, attr_pair.first);
    }
    return absl::InvalidArgumentError(absl::StrCat(
        "NodeDef missing attr", op_attrs.size() == 1 ? " '" : "s '", attrs,
        "' from ", SummarizeOpDef(op_def),
        "; NodeDef: ", FormatNodeDefForError(node_def)));
  }

  // Validate the number of inputs.
  DataTypeVector inputs, outputs;
  TF_RETURN_IF_ERROR(InOutTypesForNode(node_def, op_def, &inputs, &outputs));

  if (num_inputs != inputs.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("NodeDef expected inputs '", DataTypeVectorString(inputs),
                     "' do not match ", num_inputs, " inputs specified; ",
                     SummarizeOpDef(op_def),
                     "; NodeDef: ", FormatNodeDefForError(node_def)));
  }

  return absl::OkStatus();
}

namespace {  // Helpers for NameRangesForNode()

absl::Status ComputeArgRange(const AttrSlice& attrs,
                             const OpDef::ArgDef& arg_def, const OpDef& op_def,
                             int* num) {
  if (!arg_def.number_attr().empty()) {
    // Same type repeated "num" times.
    return GetNodeAttr(attrs, arg_def.number_attr(), num);
  } else if (!arg_def.type_list_attr().empty()) {
    const AttrValue* attr_value;
    TF_RETURN_IF_ERROR(attrs.Find(arg_def.type_list_attr(), &attr_value));
    *num = attr_value->list().type_size();
  } else if (!arg_def.type_attr().empty() || arg_def.type() != DT_INVALID) {
    *num = 1;
  } else {
    return absl::InvalidArgumentError(absl::StrCat(
        "Argument '", arg_def.name(),
        "' incorrectly specified in op definition: ", SummarizeOpDef(op_def)));
  }
  return absl::OkStatus();
}

absl::Status NameRangesHelper(
    const AttrSlice& attrs,
    const protobuf::RepeatedPtrField<OpDef::ArgDef>& args, const OpDef& op_def,
    NameRangeMap* result) {
  int start = 0;
  int num;
  for (const auto& arg : args) {
    TF_RETURN_IF_ERROR(ComputeArgRange(attrs, arg, op_def, &num));
    (*result)[arg.name()] = std::make_pair(start, start + num);
    start += num;
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status NameRangesForNode(const AttrSlice& attrs, const OpDef& op_def,
                               NameRangeMap* inputs, NameRangeMap* outputs) {
  if (inputs != nullptr) {
    TF_RETURN_IF_ERROR(
        NameRangesHelper(attrs, op_def.input_arg(), op_def, inputs));
  }
  if (outputs != nullptr) {
    return NameRangesHelper(attrs, op_def.output_arg(), op_def, outputs);
  }
  return absl::OkStatus();
}

void AddDefaultsToNodeDef(const OpDef& op_def, NodeDef* node_def) {
  for (const auto& attr_def : op_def.attr()) {
    AttrSlice attrs(*node_def);
    if (attr_def.has_default_value() && !attrs.Find(attr_def.name())) {
      AddNodeAttr(attr_def.name(), attr_def.default_value(), node_def);
    }
  }
}

void StripDefaultsFromNodeDef(const OpDef& op_def, NodeDef* node_def) {
  AttrSlice attrs(*node_def);
  for (const auto& attr_def : op_def.attr()) {
    if (attr_def.has_default_value()) {
      const AttrValue* attr = attrs.Find(attr_def.name());
      if (attr && AreAttrValuesEqual(*attr, attr_def.default_value()))
        node_def->mutable_attr()->erase(attr_def.name());
    }
  }
}

namespace {

using ::tensorflow::tstring;
using ::tensorflow::strings::Scanner;

bool IsValidNodeName(absl::string_view sp) {
  Scanner scanner(sp);
  scanner.One(Scanner::LETTER_DIGIT_DOT)
      .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);

  while (true) {
    if (!scanner.GetResult())  // Some error in previous iteration.
      return false;
    if (scanner.empty())  // No error, but nothing left, good.
      return true;

    // Absorb another name/namespace, starting with a '>'
    scanner.One(Scanner::RANGLE)
        .One(Scanner::LETTER_DIGIT_DOT)
        .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);
  }
}

bool IsValidDataInputName(absl::string_view sp) {
  // Data inputs are op_name, op_name:0, or op_name:12345.
  Scanner scan(sp);
  scan.One(Scanner::LETTER_DIGIT_DOT)
      .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);

  while (true) {
    if (!scan.GetResult())  // Some error in previous iteration.
      return false;
    if (scan.empty())  // No error, but nothing left, good.
      return true;

    if (scan.Peek() == ':') {  // Absorb identifier after the colon
      scan.OneLiteral(":");
      if (scan.Peek() == '0') {
        scan.OneLiteral("0");  // :0
      } else {
        scan.Many(Scanner::DIGIT);  // :[1-9][0-9]*
      }
    } else {
      // Absorb another name/namespace, starting with a '>'
      scan.One(Scanner::RANGLE)
          .One(Scanner::LETTER_DIGIT_DOT)
          .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);
    }
  }
}

bool IsValidControlInputName(absl::string_view sp) {
  Scanner scan(sp);
  scan.OneLiteral("^")
      .One(Scanner::LETTER_DIGIT_DOT)
      .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);

  while (true) {
    if (!scan.GetResult())  // Some error in previous iteration.
      return false;
    if (scan.empty())  // No error, but nothing left, good.
      return true;

    // Absorb another name/namespace, starting with a '>'
    scan.One(Scanner::RANGLE)
        .One(Scanner::LETTER_DIGIT_DOT)
        .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE);
  }
}

const absl::string_view kColocationGroupPrefixStringPiece(
    kColocationGroupPrefix);

}  // namespace

absl::Status ValidateOpInput(const std::string& input_name,
                             bool* is_control_input) {
  *is_control_input = false;
  if (IsValidDataInputName(input_name)) {
    return absl::OkStatus();
  } else if (IsValidControlInputName(input_name)) {
    *is_control_input = true;
    return absl::OkStatus();
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("Illegal op input name '", input_name, "'"));
  }
}

absl::Status ValidateNodeName(const std::string& node_name) {
  if (IsValidNodeName(node_name)) {
    return absl::OkStatus();
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("Illegal op name '", node_name, "'"));
  }
}

absl::Status ValidateExternalNodeDefSyntax(const NodeDef& node_def) {
  absl::Status s = ValidateNodeName(node_def.name());
  if (!s.ok()) {
    return AttachDef(s, node_def);
  }
  bool in_control_inputs = false;
  for (const std::string& input_name : node_def.input()) {
    bool is_control_input;
    s = ValidateOpInput(input_name, &is_control_input);
    if (!s.ok()) {
      return AttachDef(s, node_def);
    }

    if (in_control_inputs && !is_control_input) {
      return AttachDef(absl::InvalidArgumentError(
                           "All control inputs must follow all data inputs"),
                       node_def);
    }
    in_control_inputs = is_control_input;
  }
  return absl::OkStatus();
}

absl::Status AttachDef(const absl::Status& status, const NodeDef& node_def,
                       bool allow_multiple_formatted_node) {
  std::string node_error;
  if (!allow_multiple_formatted_node &&
      absl::StrContains(status.message(), "{{node ")) {
    node_error = node_def.name();
  } else {
    node_error = FormatNodeDefForError(node_def);
  }
  return errors::CreateWithUpdatedMessage(
      status,
      strings::StrCat(status.message(), "\n\t", " [[", node_error, "]]"));
}

void AddNodeAttr(absl::string_view name, const AttrValue& value,
                 NodeDef* node_def) {
  node_def->mutable_attr()->insert(
      AttrValueMap::value_type(std::string(name), value));
}

void AddNodeAttr(absl::string_view name, AttrValue&& value, NodeDef* node_def) {
  (*node_def->mutable_attr())[std::string(name)] = std::move(value);
}

#define ADD_NODE_ATTR(T)                                           \
  void AddNodeAttr(StringPiece name, T value, NodeDef* node_def) { \
    AttrValue attr_value;                                          \
    SetAttrValue(value, &attr_value);                              \
    AddNodeAttr(name, attr_value, node_def);                       \
  }
ADD_NODE_ATTR(absl::string_view)
ADD_NODE_ATTR(const char*)
ADD_NODE_ATTR(int32_t)
ADD_NODE_ATTR(int64_t)
ADD_NODE_ATTR(float)
ADD_NODE_ATTR(double)
ADD_NODE_ATTR(bool)
ADD_NODE_ATTR(DataType)
ADD_NODE_ATTR(const PartialTensorShape&)
ADD_NODE_ATTR(const Tensor&)
ADD_NODE_ATTR(const TensorProto&)
ADD_NODE_ATTR(const NameAttrList&)
ADD_NODE_ATTR(absl::Span<const absl::string_view>)
ADD_NODE_ATTR(absl::Span<const char* const>)
ADD_NODE_ATTR(absl::Span<const std::string>)
ADD_NODE_ATTR(absl::Span<const int32_t>)
ADD_NODE_ATTR(absl::Span<const int64_t>)
ADD_NODE_ATTR(absl::Span<const float>)
ADD_NODE_ATTR(absl::Span<const bool>)
ADD_NODE_ATTR(const std::vector<bool>&)
ADD_NODE_ATTR(absl::Span<const DataType>)
ADD_NODE_ATTR(absl::Span<const TensorShape>)
ADD_NODE_ATTR(absl::Span<const PartialTensorShape>)
ADD_NODE_ATTR(absl::Span<const TensorShapeProto>)
ADD_NODE_ATTR(absl::Span<const Tensor>)
ADD_NODE_ATTR(absl::Span<const NameAttrList>)
#undef ADD_NODE_ATTR

void AddAttr(absl::string_view name, const AttrValue& value,
             AttrValueMap* map) {
  map->insert(AttrValueMap::value_type(std::string(name), value));
}

#define ADD_ATTR(T)                                            \
  void AddAttr(StringPiece name, T value, AttrValueMap* map) { \
    AttrValue attr_value;                                      \
    SetAttrValue(value, &attr_value);                          \
    AddAttr(name, attr_value, map);                            \
  }
ADD_ATTR(bool)
#undef ADD_ATTR

absl::Status AddPrefixAndSuffixToNode(absl::string_view prefix,
                                      absl::string_view suffix,
                                      NodeDef* node_def,
                                      bool uniquify_frame_name) {
  node_def->set_name(absl::StrCat(prefix, node_def->name(), suffix));

  // Update frame name to avoid multiple LoopCond nodes in one frame.
  if (uniquify_frame_name &&
      (node_def->op() == "Enter" || node_def->op() == "RefEnter")) {
    std::string frame_name;
    TF_RETURN_IF_ERROR(GetNodeAttr(*node_def, "frame_name", &frame_name));
    AttrValue& attr = (*node_def->mutable_attr())["frame_name"];
    frame_name = absl::StrCat(prefix, frame_name, suffix);
    attr.set_s(frame_name);
  }

  return absl::OkStatus();
}

absl::Status MaybeAddPrefixToColocationConstraints(
    const std::unordered_set<std::string>& match, absl::string_view prefix,
    NodeDef* node_def) {
  auto attr = node_def->mutable_attr()->find(kColocationAttrName);
  if (attr == node_def->mutable_attr()->end()) {
    return absl::OkStatus();
  }
  auto constraints_list = attr->second.mutable_list();
  auto constraints_size = constraints_list->s_size();
  for (size_t i = 0; i < constraints_size; ++i) {
    absl::string_view original(constraints_list->s(i));
    if (absl::ConsumePrefix(&original, kColocationGroupPrefixStringPiece)) {
      if (match.find(std::string(original)) != match.end()) {
        (*constraints_list->mutable_s(i)) =
            absl::StrCat(kColocationGroupPrefix, prefix, original);
      }
    }
  }
  return absl::OkStatus();
}

absl::Status MaybeUpdateColocationConstraintsWithMap(
    const std::map<absl::string_view, absl::string_view>& node_name_map,
    NodeDef* node_def) {
  auto attr = node_def->mutable_attr()->find(kColocationAttrName);
  if (attr == node_def->mutable_attr()->end()) {
    return absl::OkStatus();
  }
  auto constraints_list = attr->second.mutable_list();
  auto constraints_size = constraints_list->s_size();
  for (size_t i = 0; i < constraints_size; ++i) {
    absl::string_view original(constraints_list->s(i));
    if (absl::ConsumePrefix(&original, kColocationGroupPrefixStringPiece)) {
      if (node_name_map.find(original) != node_name_map.end()) {
        (*constraints_list->mutable_s(i)) =
            absl::StrCat(kColocationGroupPrefix, node_name_map.at(original));
      }
    }
  }
  return absl::OkStatus();
}

void ChangeToNoOp(NodeDef* node_def) {
  node_def->set_op("NoOp");
  // NoOp nodes have no outputs. Remove any full type information describing
  // any outputs the node previous had.
  node_def->clear_experimental_type();
}

}  // namespace tensorflow

} // export

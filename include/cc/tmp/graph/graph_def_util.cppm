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
#include "tensorflow/core/framework/function.h"
#include "tensorflow/core/framework/function.pb.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/node_def_util.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_def_util.h"
#include "tensorflow/core/framework/versions.pb.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/strcat.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module cc_tmp:graph_graph_def_util;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // Forward declare proto so that it's symbols can be removed from .so exports
        class GraphDef;
        class NodeDef;

        // Produce a human-readable version of a GraphDef that is more concise
        // than a text-format proto.
        std::string SummarizeGraphDef(const GraphDef& graph_def);

        // Validates the syntax of a GraphDef provided externally.
        //
        // The following is an EBNF-style syntax for GraphDef objects. Note that
        // Node objects are actually specified as tensorflow::NodeDef protocol buffers,
        // which contain many other fields that are not (currently) validated.
        //
        // Graph        = Node *
        // Node         = NodeName, Inputs
        // Inputs       = ( DataInput * ), ( ControlInput * )
        // DataInput    = NodeName, ( ":", [1-9], [0-9] * ) ?
        // ControlInput = "^", NodeName
        // NodeName     = [A-Za-z0-9.], [A-Za-z0-9_./] *
        absl::Status ValidateExternalGraphDefSyntax(const GraphDef& graph_def);

        // Adds default attributes to NodeDefs in 'graph_def' starting
        // from the 'node_offset' node in 'graph_def'.
        //
        // Default attributes are defined by 'op_registry'.
        //
        // Returns OK on success, an error if 'graph_def' has a NodeDef
        // that cannot be found in 'op_registry'.
        //
        // REQUIRES: 'graph_def' and 'op_registry' are not nullptr.
        absl::Status AddDefaultAttrsToGraphDef(
            GraphDef* graph_def, const OpRegistryInterface& op_registry, int node_offset
        );

        // Same as above, except for the fact that it skips nodes that aren't found in
        // op_registry if skip_unknown_ops is true.
        absl::Status AddDefaultAttrsToGraphDef(
            GraphDef* graph_def,
            const OpRegistryInterface& op_registry,
            int node_offset,
            bool skip_unknown_ops
        );

        // Remove attrs from 'graph_def' that have the default value according
        // to 'producer_op_registry', but don't exist according to
        // 'consumer_op_registry'. This can allow 'graph_def' to run on the
        // consumer even if consumer was built at an earlier CL (before an
        // attr with a default was added). Note that this will not affect
        // attrs with non-default values, so you must run a
        // ValidateGraphDef...() function to see if the result is in fact
        // compatible. If not nullptr, the op/attr pairs that were removed
        // are added to '*op_attr_removed'.
        //
        // Expected usage, for a producer that wants to prepare a graph for
        // a consumer:
        // // For each consumer, update 'graph_def':
        //   OpListOpRegistry consumer_op_registry(consumer_server_op_list);
        //   std::unordered_set<std::pair<string, string>> op_attr_removed;
        //   TF_RETURN_IF_ERROR(RemoveNewDefaultAttrsFromGraphDef(
        //       &graph_def, consumer_op_registry, *OpRegistry::Global(),
        //       &op_attr_removed));
        // // Validate that each consumer can understand the resulting 'graph_def'
        //   TF_RETURN_IF_ERROR(graph::ValidateGraphDefAgainstOpRegistry(
        //       graph_def, consumer_op_registry));
        // // Consumer can use 'graph_def', and 'op_attr_removed' summarizes
        // // what changes had to be made to 'graph_def' for it to work.
        //
        // Expected usage, for a consumer that has a graph and a
        // (optionally-stripped) op_list from a producer (say from a call to
        // StrippedOpListForGraph(), or in the MetaGraphDef):
        //   OpListOpRegistry producer_op_registry(producer_stripped_op_list);
        //   TF_RETURN_IF_ERROR(RemoveNewDefaultAttrsFromGraphDef(
        //       &graph_def, *OpRegistry::Global(), producer_op_registry, nullptr));
        absl::Status RemoveNewDefaultAttrsFromGraphDef(
            GraphDef* graph_def,
            const OpRegistryInterface& consumer_op_registry,
            const OpRegistryInterface& producer_op_registry,
            std::set<std::pair<std::string, std::string>>* op_attr_removed
        );

        // Goes over the `nodes` and removes attributes that are set to their
        // default values according to op_registry.
        // If some node's definition is not found in the `op_registry`, this node is
        // simply skipped. In most cases, these nodes would be function calls.
        // If a stricter behavior is desired, one can add FunctionLibraryDefinition
        // argument to check for functions and their attributes.
        // This is obvious from signature, but as a warning, if `nodes` contain
        // nodes calling functions, e.g. PartitionCallOp or FunctionalIf, this
        // function does not "recurse" into them.
        void StripDefaultAttributes(
            const OpRegistryInterface& op_registry, protobuf::RepeatedPtrField<NodeDef>* nodes
        );

        // Two functions that collect the ops used by a graph.
        //
        // This returns the ops used as a set of strings.
        void OpsUsedByGraph(const GraphDef& graph_def, std::set<std::string>* ops_used_in_graph);

        // This function computes the stripped_op_list field of MetaGraphDef
        // and similar protos.  The op_registry should contain the ops used to
        // produce graph_def.  The resulting stripped_op_list can be
        // communicated from the producer to the consumer, which can use
        // RemoveNewDefaultAttrsFromGraphDef() to improve forwards compatibility
        // (using an OpListOpRegistry as indicated in the example above).
        //
        // Most users will pass *OpRegistry::Global() for op_registry to strip against
        // the list of ops registered in this process.
        absl::Status StrippedOpListForGraph(
            const GraphDef& graph_def,
            const OpRegistryInterface& op_registry,
            OpList* stripped_op_list
        );

    } // namespace tensorflow

    // ==================================================================
    // Implementation: graph_def_util.cc
    // ==================================================================

    namespace tensorflow {

        std::string SummarizeGraphDef(const GraphDef& graph_def)
        {
            std::string ret;
            absl::StrAppend(&ret, "versions = ", graph_def.versions().ShortDebugString(), ";\n");
            for (const NodeDef& node: graph_def.node()) {
                absl::StrAppend(&ret, SummarizeNodeDef(node), ";\n");
            }
            return ret;
        }

        absl::Status ValidateExternalGraphDefSyntax(const GraphDef& graph_def)
        {
            for (const NodeDef& node: graph_def.node()) {
                TF_RETURN_IF_ERROR(ValidateExternalNodeDefSyntax(node));
            }
            return absl::OkStatus();
        }

        absl::Status AddDefaultAttrsToGraphDef(
            GraphDef* graph_def, const OpRegistryInterface& op_registry, int node_offset
        )
        {
            return AddDefaultAttrsToGraphDef(graph_def, op_registry, node_offset, false);
        }

        absl::Status AddDefaultAttrsToGraphDef(
            GraphDef* graph_def,
            const OpRegistryInterface& op_registry,
            int node_offset,
            bool skip_unknown_ops
        )
        {
            if (node_offset > graph_def->node_size()) {
                return absl::InvalidArgumentError(
                    absl::StrCat(
                        "Tried to add default attrs to GraphDef "
                        "starting at offset ",
                        node_offset, " with total nodes in graph: ", graph_def->node_size()
                    )
                );
            }

            for (int i = node_offset; i < graph_def->node_size(); ++i) {
                NodeDef* node_def = graph_def->mutable_node(i);
                const OpDef* op_def;
                absl::Status s = op_registry.LookUpOpDef(node_def->op(), &op_def);
                if (s.ok()) {
                    AddDefaultsToNodeDef(*op_def, node_def);
                } else if (!skip_unknown_ops) {
                    return s;
                }
            }

            return absl::OkStatus();
        }

        static absl::Status RemoveNewDefaultAttrsFromNodeDef(
            NodeDef* node_def,
            const OpRegistryInterface& consumer_op_registry,
            const OpRegistryInterface& producer_op_registry,
            std::set<std::pair<std::string, std::string>>* op_attr_removed
        )
        {
            const OpDef* producer_op_def;
            const OpDef* consumer_op_def;
            TF_RETURN_IF_ERROR(producer_op_registry.LookUpOpDef(node_def->op(), &producer_op_def));
            TF_RETURN_IF_ERROR(consumer_op_registry.LookUpOpDef(node_def->op(), &consumer_op_def));

            std::vector<std::string> to_remove;
            for (const auto& attr: node_def->attr()) {
                // If the attr is not in consumer_op_def and doesn't start with '_'...
                if (!absl::StartsWith(attr.first, "_") &&
                    FindAttr(attr.first, *consumer_op_def) == nullptr) {
                    const OpDef::AttrDef* producer_attr_def =
                        FindAttr(attr.first, *producer_op_def);
                    if (producer_attr_def == nullptr) {
                        return absl::InvalidArgumentError(
                            absl::StrCat(
                                "Attr '", attr.first,
                                "' missing in producer's OpDef: ", SummarizeOpDef(*producer_op_def),
                                " but found in node: ", FormatNodeDefForError(*node_def)
                            )
                        );
                    }
                    // ...and it has the same value as the default in producer,
                    if (producer_attr_def->has_default_value() &&
                        AreAttrValuesEqual(producer_attr_def->default_value(), attr.second)) {
                        // then we will remove it below.
                        to_remove.emplace_back(attr.first);
                    }
                }
            }
            // We separate identifying which attrs should be removed from
            // actually removing them to avoid invalidating the loop iterators
            // above.
            for (const std::string& attr_name: to_remove) {
                node_def->mutable_attr()->erase(attr_name);
                if (op_attr_removed != nullptr) {
                    op_attr_removed->insert(std::make_pair(node_def->op(), attr_name));
                }
            }

            return absl::OkStatus();
        }

        static bool IsFunction(const GraphDef& graph_def, const std::string& op_name)
        {
            for (const auto& func_def: graph_def.library().function()) {
                if (op_name == func_def.signature().name()) {
                    return true;
                }
            }
            return false;
        }

        absl::Status RemoveNewDefaultAttrsFromGraphDef(
            GraphDef* graph_def,
            const OpRegistryInterface& consumer_op_registry,
            const OpRegistryInterface& producer_op_registry,
            std::set<std::pair<std::string, std::string>>* op_attr_removed
        )
        {
            // TODO(joshL): Make IsFunction() faster by collecting the names of
            // all functions as a preprocessing step.
            for (int n = 0; n < graph_def->node_size(); ++n) {
                NodeDef* node_def = graph_def->mutable_node(n);
                if (!IsFunction(*graph_def, node_def->op())) {
                    TF_RETURN_IF_ERROR(RemoveNewDefaultAttrsFromNodeDef(
                        node_def, consumer_op_registry, producer_op_registry, op_attr_removed
                    ));
                }
            }
            for (int f = 0; f < graph_def->library().function_size(); ++f) {
                FunctionDef* func_def = graph_def->mutable_library()->mutable_function(f);
                for (int n = 0; n < func_def->node_def_size(); ++n) {
                    NodeDef* node_def = func_def->mutable_node_def(n);
                    if (!IsFunction(*graph_def, node_def->op())) {
                        // TODO(josh11b): Better handling of attrs with placeholder values.
                        TF_RETURN_IF_ERROR(RemoveNewDefaultAttrsFromNodeDef(
                            node_def, consumer_op_registry, producer_op_registry, op_attr_removed
                        ));
                    }
                }
            }

            return absl::OkStatus();
        }

        void StripDefaultAttributes(
            const OpRegistryInterface& op_registry, protobuf::RepeatedPtrField<NodeDef>* nodes
        )
        {
            for (int i = 0; i < nodes->size(); ++i) {
                NodeDef* node = nodes->Mutable(i);

                const OpDef* op_def;
                const OpRegistrationData* op_reg_data = nullptr;
                absl::Status s = op_registry.LookUp(node->op(), &op_reg_data);
                if (!s.ok()) {
                    VLOG(1) << "Ignoring encountered unknown operation " << SummarizeNodeDef(*node)
                            << " when stripping default attributes. It is likely a function, "
                               "in which case ignoring it is fine";
                    continue;
                }
                op_def = &op_reg_data->op_def;

                for (const OpDef::AttrDef& attr_def: op_def->attr()) {
                    if (attr_def.has_default_value()) {
                        AttrValueMap* attrs = node->mutable_attr();
                        const std::string& name = attr_def.name();
                        auto iter = attrs->find(name);
                        if (iter != attrs->end()) {
                            const AttrValue& default_value = attr_def.default_value();
                            // There should never be an attribute whose default value is a tensor
                            // larger than 32MB so allow false negatives  for efficient
                            // comparison.
                            if (AreAttrValuesEqual(
                                    iter->second, default_value,
                                    /*allow_false_negatives=*/true
                                )) {
                                attrs->erase(name);
                            }
                        }
                    }
                }
            }
        }

        void OpsUsedByGraph(const GraphDef& graph_def, std::set<std::string>* ops_used_in_graph)
        {
            // Map function names to definitions.
            std::unordered_map<std::string, const FunctionDef*> name_to_function;
            for (const auto& function: graph_def.library().function()) {
                name_to_function.insert(std::make_pair(function.signature().name(), &function));
            }

            // Collect the sorted list of op names.  Since functions can reference
            // functions, we need a recursive traversal.
            std::set<std::string> used_ops; // Includes both primitive ops and functions
            std::vector<const FunctionDef*> functions_to_process; // A subset of used_ops
            // Collect the logic to mark an op in a lambda; it'll be used twice below.
            const auto mark_op_as_used = [&used_ops, &functions_to_process,
                                          &name_to_function](const std::string& op) {
                if (used_ops.insert(op).second) {
                    // If it's a function, we'll need to process further
                    const auto it = name_to_function.find(op);
                    if (it != name_to_function.end()) {
                        functions_to_process.push_back(it->second);
                    }
                }
            };
            for (const auto& node: graph_def.node()) {
                mark_op_as_used(node.op());
            }
            while (!functions_to_process.empty()) {
                const FunctionDef* fun = functions_to_process.back();
                functions_to_process.pop_back();
                for (const auto& node: fun->node_def()) {
                    mark_op_as_used(node.op());
                }
            }

            // Filter out function names to produce output.
            // TODO(josh11b): Change the above code to produce this directly.
            ops_used_in_graph->clear();
            for (const std::string& op_name: used_ops) {
                if (name_to_function.find(op_name) == name_to_function.end()) {
                    ops_used_in_graph->insert(op_name);
                }
            }
        }

        absl::Status StrippedOpListForGraph(
            const GraphDef& graph_def,
            const OpRegistryInterface& op_registry,
            OpList* stripped_op_list
        )
        {
            std::set<std::string> used_ops;
            OpsUsedByGraph(graph_def, &used_ops);

            // Build the stripped op list in sorted order, ignoring functions.
            stripped_op_list->clear_op();
            for (const std::string& op_name: used_ops) {
                const OpDef* op_def;
                TF_RETURN_IF_ERROR(op_registry.LookUpOpDef(op_name, &op_def));
                OpDef* stripped_op = stripped_op_list->add_op();
                stripped_op->CopyFrom(*op_def);
                RemoveDescriptionsFromOpDef(stripped_op);
            }
            return absl::OkStatus();
        }

    } // namespace tensorflow

} // export

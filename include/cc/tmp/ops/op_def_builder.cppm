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

#include "absl/strings/escaping.h"
#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/attr_value_util.h"
#include "tensorflow/core/framework/full_type.pb.h"
#include "tensorflow/core/framework/op_def.pb.h"
#include "tensorflow/core/framework/op_def_util.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/gtl/array_slice.h"
#include "tensorflow/core/lib/strings/scanner.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/macros.h"

#include <limits>
#include <string>
#include <utility>
#include <vector>

export module cc_tmp:ops_op_def_builder;

import std;
import cc_abi;

export {

    // Class and associated machinery for specifying an Op's OpDef and shape
    // inference function for Op registration.

#ifndef TENSORFLOW_CORE_FRAMEWORK_OP_DEF_BUILDER_H_
#    define TENSORFLOW_CORE_FRAMEWORK_OP_DEF_BUILDER_H_

    namespace tensorflow {

        // TODO(b/62899350): Refactor without proto dependencies.
        typedef std::function<absl::Status(OpDef* c)> OpTypeConstructor;

        typedef std::vector<std::reference_wrapper<const FullTypeDef>> TypeRefVector;

        // A callback into the type inference process, allowing type inference functions
        // to request inferring the type of some function (assumed to exist in the
        // runtime). The function is specified by name.
        typedef std::function<absl::StatusOr<FullTypeDef>(const std::string&, const TypeRefVector&)>
            FunctionTypeInferrer;

        // A type inference function, called for each node during type inference
        // (possibly multiple times).
        // The first argument (input_types) will hold the type of each of the node's
        // inputs. The second argument (type_vars) will hold the return type of
        // each function referred from any type variable (e.g. `FuncVar`) present
        // in the node's corresponding op definition.
        //
        // TODO(mdan): Consider a vector-in, vector-out contract.
        typedef std::function<
            absl::StatusOr<FullTypeDef>(const TypeRefVector&, const FunctionTypeInferrer&)>
            TypeInferenceFn;

        class FunctionDefHelper;

        namespace shape_inference {
            class InferenceContext;
        }

        typedef std::function<absl::Status(shape_inference::InferenceContext* c)>
            OpShapeInferenceFn;

        struct OpRegistrationData
        {
        public:
            OpRegistrationData() {}

            OpRegistrationData(const OpDef& def) :
                op_def(def)
            {
            }

            OpRegistrationData(
                const OpDef& def, const OpShapeInferenceFn& fn, bool is_function = false
            ) :
                op_def(def),
                shape_inference_fn(fn),
                is_function_op(is_function)
            {
            }

            OpDef op_def;
            OpShapeInferenceFn shape_inference_fn;

            // Type constructor. This callable initializes the type of this op.
            // It is provided as a programmatic mechanism for defining an op's
            // type, as part of its registration. It is to be eventually replaced by a
            // textual language.
            //
            // Important: historically, op registrations only contained partial
            // input/output type information in non-standardized attribute declarations
            // (e.g. typically, input types were held in a `dtype` attribute). The type
            // constructor currently duplicates such attribute information, with the aim
            // of entirely subsuming it, and eventually deprecating all type-related
            // attributes.
            //
            // Since ops are typically parametrized, the type created by this constructor
            // is also parametric.
            //
            // Example: for an op `Foo(x: T) -> Bar[T]`:
            //
            //  * typically, its op registration included a single attribute `T: type`;
            //    then the respective input was defined as `x: T`; the output type `Bar`
            //    was implied by the op name.
            //  * the type constructor creates a FullType object containing `Bar[T]`; this
            //    still relies on the `T` attribute which it references.
            //  * in the future, the type constructor will create a FullType containing
            //    `Callable[(x: T), Bar[T]]`, and the attribute `T` will be deprecated.
            OpTypeConstructor type_ctor;

            // Forward type inference function. This callable infers the return type of an
            // op based on its input types.
            //
            // Note that the type constructor and forward inference functions need not be
            // mutually exclusive: if there is some static information that can be set
            // based on attributes, then that should be set in the constructor. If more
            // information can be extracted from inputs, that should be done in the
            // forward inference function.
            //
            // This is similar to the shape function, but is more general, and applied
            // directly to NodeDefs, rather than working on the ShapeAndType structures.
            // Note that the op input/output declarations may specify some implicit type
            // constraints through attribute references (i.e. two inputs pointing to the
            // same type attribute). Those constraints may duplicate what this function
            // specifies in its body. That's intended, for a gradual transition to a more
            // formal type system.
            //
            // These type inference functions are intermediate solutions as well: once the
            // op registration has a complete, formal type definition, along with
            // a solver-based type inference, it will replace these functions.
            //
            // TODO(mdan): Merge with shape inference.
            // TODO(mdan): Replace with a union-based type inference algorithm.
            TypeInferenceFn fwd_type_fn;

            // Reverse type inference function. This callable infers some input types
            // based on the return type.
            //
            // TODO(mdan): Replace with a union-based type inference algorithm.
            TypeInferenceFn rev_type_fn;

            // The input number affected by reverse type inference. Only one input may be
            // updated in this manner.
            // TODO(mdan): Encode in a manner more consistent with the forward version.
            int rev_type_input;

            bool is_function_op = false;
        };

        // Builder class passed to the REGISTER_OP() macro.
        class OpDefBuilder
        {
        public:
            // Constructs an OpDef with just the name field set.
            explicit OpDefBuilder(std::string op_name);

            // Adds an attr to this OpDefBuilder (and returns *this). The spec has
            // format "<name>:<type>" or "<name>:<type>=<default>"
            // where <name> matches regexp [a-zA-Z][a-zA-Z0-9_]*
            // (by convention only using capital letters for attrs that can be inferred)
            // <type> can be:
            //   "string", "int", "float", "bool", "type", "shape", or "tensor"
            //   "numbertype", "realnumbertype", "quantizedtype"
            //       (meaning "type" with a restriction on valid values)
            //   "{int32,int64}" or {realnumbertype,quantizedtype,string}"
            //       (meaning "type" with a restriction containing unions of value types)
            //   "{\"foo\", \"bar\n baz\"}", or "{'foo', 'bar\n baz'}"
            //       (meaning "string" with a restriction on valid values)
            //   "list(string)", ..., "list(tensor)", "list(numbertype)", ...
            //       (meaning lists of the above types)
            //   "int >= 2" (meaning "int" with a restriction on valid values)
            //   "list(string) >= 2", "list(int) >= 2"
            //       (meaning "list(string)" / "list(int)" with length at least 2)
            // <default>, if included, should use the Proto text format
            // of <type>.  For lists use [a, b, c] format.
            //
            // Note that any attr specifying the length of an input or output will
            // get a default minimum of 1 unless the >= # syntax is used.
            //
            // TODO(josh11b): Perhaps support restrictions and defaults as optional
            // extra arguments to Attr() instead of encoding them in the spec string.
            // TODO(josh11b): Would like to have better dtype handling for tensor attrs:
            // * Ability to say the type of an input/output matches the type of
            //   the tensor.
            // * Ability to restrict the type of the tensor like the existing
            //   restrictions for type attrs.
            // Perhaps by linking the type of the tensor to a type attr?
            OpDefBuilder& Attr(std::string spec);

            // Adds an input or output to this OpDefBuilder (and returns *this).
            // The spec has form "<name>:<type-expr>" or "<name>:Ref(<type-expr>)"
            // where <name> matches regexp [a-z][a-z0-9_]* and <type-expr> can be:
            // * For a single tensor: <type>
            // * For a sequence of tensors with the same type: <number>*<type>
            // * For a sequence of tensors with different types: <type-list>
            // Where:
            //   <type> is either one of "float", "int32", "string", ...
            //                 or the name of an attr (see above) with type "type".
            //   <number> is the name of an attr with type "int".
            //   <type-list> is the name of an attr with type "list(type)".
            // TODO(josh11b): Indicate Ref() via an optional argument instead of
            // in the spec?
            // TODO(josh11b): SparseInput() and SparseOutput() matching the Python
            // handling?
            OpDefBuilder& Input(std::string spec);
            OpDefBuilder& Output(std::string spec);

            // Turns on the indicated boolean flag in this OpDefBuilder (and
            // returns *this).
            OpDefBuilder& SetIsCommutative(bool is_commutative = true);
            OpDefBuilder& SetIsAggregate(bool is_aggregate = true);
            OpDefBuilder& SetIsStateful(bool is_stateful = true);
            OpDefBuilder& SetAllowsUninitializedInput(bool allows_uninitialized_input = true);
            OpDefBuilder& SetIsDistributedCommunication(bool is_distributed_communication = true);

            // Deprecate the op at a certain GraphDef version.
            OpDefBuilder& Deprecated(int version, std::string explanation);

            // Adds docs to this OpDefBuilder (and returns *this).
            // Docs have the format:
            //   <1-line summary>
            //   <rest of the description>
            //   <name>: <description of name>
            //   <name>: <description of name>
            //     <if long, indent the description on subsequent lines>
            // Where <name> is the name of an attr, input, or output.  Please
            // wrap docs at 72 columns so that it may be indented in the
            // generated output.  For tensor inputs or outputs (not attrs), you
            // may start the description with an "=" (like name:= <description>)
            // to suppress the automatically-generated type documentation in
            // generated output.
            OpDefBuilder& Doc(std::string text);

            // Sets the function to be used as type constructor.
            // See OpRegistrationData::type_ctor.
            OpDefBuilder& SetTypeConstructor(OpTypeConstructor c);

            // Sets the function to be used for forward type inference.
            // See OpRegistrationData::fwd_type_fn.
            OpDefBuilder& SetForwardTypeFn(TypeInferenceFn f);

            // Sets the function to be used for reverse type inference.
            // See OpRegistrationData::rew_type_fn.
            OpDefBuilder& SetReverseTypeFn(int input_number, TypeInferenceFn f);

            // Sets the shape function to be used for shape inference.
            //
            // Note that currently (October 2016), python code still requires a
            // RegisterShape call to invoke this; see call_cpp_shape_fn in
            // python/framework/common_shapes.py
            OpDefBuilder& SetShapeFn(OpShapeInferenceFn fn);

            // Allows the `<type>` in calls to `Attr()` to be "any".
            // This is used by PythonAPIWrapper for pass-through parameters.
            OpDefBuilder& AllowAttrTypeAny();

            // Sets op_reg_data->op_def to the requested OpDef and
            // op_reg_data->shape_inference_fn to the requested shape inference function,
            // or returns an error.
            // Must be called after all of the above methods.
            //
            // Note that OpDefBuilder only reports parsing errors.  You should also
            // call ValidateOpDef() to detect other problems.
            absl::Status Finalize(OpRegistrationData* op_reg_data) const;

        private:
            friend class FunctionDefHelper;

            // Adds control output to this OpDefBuilder (and returns *this).
            // The <name> must be a valid node name (matches regexp
            // [a-zA-Z][a-zA-Z0-9_]*). Named control output can only exist for functions.
            OpDefBuilder& ControlOutput(std::string name);

            OpDef* op_def()
            {
                return &op_reg_data_.op_def;
            }

            OpRegistrationData op_reg_data_;
            std::vector<std::string> attrs_;
            std::vector<std::string> inputs_;
            std::vector<std::string> outputs_;
            std::vector<std::string> control_outputs_;
            std::string doc_;
            std::vector<std::string> errors_;
            bool allow_attr_type_any_ = false;
        };

    } // namespace tensorflow

#endif // TENSORFLOW_CORE_FRAMEWORK_OP_DEF_BUILDER_H_

    // ==================================================================
    // Implementation: op_def_builder.cc
    // ==================================================================

    using ::tensorflow::strings::Scanner;

    namespace tensorflow {

        namespace {

            std::string AttrError(absl::string_view orig, const std::string& op_name)
            {
                return absl::StrCat(" from Attr(\"", orig, "\") for Op ", op_name);
            }

            bool ConsumeAttrName(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .One(Scanner::LETTER)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .AnySpace()
                    .OneLiteral(":")
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeListPrefix(absl::string_view* sp)
            {
                return Scanner(*sp)
                    .OneLiteral("list")
                    .AnySpace()
                    .OneLiteral("(")
                    .AnySpace()
                    .GetResult(sp);
            }

            bool ConsumeQuotedString(char quote_ch, absl::string_view* sp, absl::string_view* out)
            {
                const std::string quote_str(1, quote_ch);
                return Scanner(*sp)
                    .OneLiteral(quote_str.c_str())
                    .RestartCapture()
                    .ScanEscapedUntil(quote_ch)
                    .StopCapture()
                    .OneLiteral(quote_str.c_str())
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeAttrType(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .Many(Scanner::LOWERLETTER_DIGIT)
                    .StopCapture()
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeAttrNumber(absl::string_view* sp, int64_t* out)
            {
                Scanner scan(*sp);
                absl::string_view match;
                absl::string_view remaining;

                scan.AnySpace().RestartCapture();
                if (scan.Peek() == '-') {
                    scan.OneLiteral("-");
                }
                if (!scan.Many(Scanner::DIGIT)
                         .StopCapture()
                         .AnySpace()
                         .GetResult(&remaining, &match)) {
                    return false;
                }
                int64_t value = 0;
                if (!absl::SimpleAtoi(match, &value)) {
                    return false;
                }
                *out = value;
                *sp = remaining;
                return true;
            }

#define VERIFY(expr, ...)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            errors->push_back(strings::StrCat(__VA_ARGS__, AttrError(orig, op_def->name())));      \
            return;                                                                                \
        }                                                                                          \
    } while (false)

            bool ConsumeCompoundAttrType(absl::string_view* sp, absl::string_view* out)
            {
                auto capture_data = sp->data();
                auto capture_begin = sp->begin();
                if (absl::ConsumePrefix(sp, "numbertype") ||
                    absl::ConsumePrefix(sp, "numerictype") ||
                    absl::ConsumePrefix(sp, "quantizedtype") ||
                    absl::ConsumePrefix(sp, "realnumbertype") ||
                    absl::ConsumePrefix(sp, "realnumberictype")) {
                    *out = absl::string_view(capture_data, sp->begin() - capture_begin);
                    return true;
                }
                return false;
            }

            bool ProcessCompoundType(const absl::string_view type_string, AttrValue* allowed)
            {
                if (type_string == "numbertype" || type_string == "numerictype") {
                    for (DataType dt: NumberTypes()) {
                        allowed->mutable_list()->add_type(dt);
                    }
                } else if (type_string == "quantizedtype") {
                    for (DataType dt: QuantizedTypes()) {
                        allowed->mutable_list()->add_type(dt);
                    }
                } else if (type_string == "realnumbertype" || type_string == "realnumerictype") {
                    for (DataType dt: RealNumberTypes()) {
                        allowed->mutable_list()->add_type(dt);
                    }
                } else {
                    return false;
                }
                return true;
            }

            void FinalizeAttr(
                absl::string_view spec,
                bool allow_attr_type_any,
                OpDef* op_def,
                std::vector<std::string>* errors
            )
            {
                OpDef::AttrDef* attr = op_def->add_attr();
                absl::string_view orig(spec);

                // Parse "<name>:" at the beginning.
                absl::string_view tmp_name;
                VERIFY(ConsumeAttrName(&spec, &tmp_name), "Trouble parsing '<name>:'");
                attr->set_name(tmp_name.data(), tmp_name.size());

                // Read "<type>" or "list(<type>)".
                bool is_list = ConsumeListPrefix(&spec);
                std::string type;
                absl::string_view type_string; // Used if type == "type"
                if (absl::ConsumePrefix(&spec, "string")) {
                    type = "string";
                } else if (absl::ConsumePrefix(&spec, "int")) {
                    type = "int";
                } else if (absl::ConsumePrefix(&spec, "float")) {
                    type = "float";
                } else if (absl::ConsumePrefix(&spec, "bool")) {
                    type = "bool";
                } else if (absl::ConsumePrefix(&spec, "type")) {
                    type = "type";
                } else if (absl::ConsumePrefix(&spec, "shape")) {
                    type = "shape";
                } else if (absl::ConsumePrefix(&spec, "tensor")) {
                    type = "tensor";
                } else if (absl::ConsumePrefix(&spec, "func")) {
                    type = "func";
                } else if (absl::ConsumePrefix(&spec, "any") && allow_attr_type_any) {
                    type = "any";
                } else if (ConsumeCompoundAttrType(&spec, &type_string)) {
                    type = "type";
                    AttrValue* allowed = attr->mutable_allowed_values();
                    VERIFY(
                        ProcessCompoundType(type_string, allowed),
                        "Expected to see a compound type, saw: ", type_string
                    );
                } else if (absl::ConsumePrefix(&spec, "{")) {
                    // e.g. "{ int32, float, bool }" or "{ \"foo\", \"bar\" }"
                    AttrValue* allowed = attr->mutable_allowed_values();
                    str_util::RemoveLeadingWhitespace(&spec);
                    if (absl::StartsWith(spec, "\"") || absl::StartsWith(spec, "'")) {
                        type = "string"; // "{ \"foo\", \"bar\" }" or "{ 'foo', 'bar' }"
                        while (true) {
                            absl::string_view escaped_string;
                            VERIFY(
                                ConsumeQuotedString('"', &spec, &escaped_string) ||
                                    ConsumeQuotedString('\'', &spec, &escaped_string),
                                "Trouble parsing allowed string at '", spec, "'"
                            );
                            std::string unescaped;
                            std::string error;
                            VERIFY(
                                absl::CUnescape(escaped_string, &unescaped, &error),
                                "Trouble unescaping \"", escaped_string, "\", got error: ", error
                            );
                            allowed->mutable_list()->add_s(unescaped);
                            if (absl::ConsumePrefix(&spec, ",")) {
                                str_util::RemoveLeadingWhitespace(&spec);
                                if (absl::ConsumePrefix(&spec, "}")) {
                                    break; // Allow ending with ", }".
                                }
                            } else {
                                VERIFY(
                                    absl::ConsumePrefix(&spec, "}"),
                                    "Expected , or } after strings in list, not: '", spec, "'"
                                );
                                break;
                            }
                        }
                    } else { // "{ bool, numbertype, string }"
                        type = "type";
                        while (true) {
                            VERIFY(
                                ConsumeAttrType(&spec, &type_string),
                                "Trouble parsing type string at '", spec, "'"
                            );
                            if (ProcessCompoundType(type_string, allowed)) {
                                // Processed a compound type.
                            } else {
                                DataType dt;
                                VERIFY(
                                    DataTypeFromString(type_string, &dt),
                                    "Unrecognized type string '", type_string, "'"
                                );
                                allowed->mutable_list()->add_type(dt);
                            }
                            if (absl::ConsumePrefix(&spec, ",")) {
                                str_util::RemoveLeadingWhitespace(&spec);
                                if (absl::ConsumePrefix(&spec, "}")) {
                                    break; // Allow ending with ", }".
                                }
                            } else {
                                VERIFY(
                                    absl::ConsumePrefix(&spec, "}"),
                                    "Expected , or } after types in list, not: '", spec, "'"
                                );
                                break;
                            }
                        }
                    }
                } else { // if spec.Consume("{")
                    VERIFY(false, "Trouble parsing type string at '", spec, "'");
                }
                str_util::RemoveLeadingWhitespace(&spec);

                // Write the type into *attr.
                if (is_list) {
                    VERIFY(
                        absl::ConsumePrefix(&spec, ")"), "Expected ) to close 'list(', not: '",
                        spec, "'"
                    );
                    str_util::RemoveLeadingWhitespace(&spec);
                    attr->set_type(absl::StrCat("list(", type, ")"));
                } else {
                    attr->set_type(type);
                }

                // Read optional minimum constraint at the end.
                if ((is_list || type == "int") && absl::ConsumePrefix(&spec, ">=")) {
                    int64_t min_limit = -999;
                    VERIFY(
                        ConsumeAttrNumber(&spec, &min_limit),
                        "Could not parse integer lower limit after '>=', found '", spec, "' instead"
                    );
                    attr->set_has_minimum(true);
                    attr->set_minimum(min_limit);
                }

                // Parse default value, if present.
                if (absl::ConsumePrefix(&spec, "=")) {
                    str_util::RemoveLeadingWhitespace(&spec);
                    VERIFY(
                        ParseAttrValue(attr->type(), spec, attr->mutable_default_value()),
                        "Could not parse default value '", spec, "'"
                    );
                } else {
                    VERIFY(spec.empty(), "Extra '", spec, "' unparsed at the end");
                }
            }

#undef VERIFY

            std::string
            InOutError(bool is_output, absl::string_view orig, const std::string& op_name)
            {
                return strings::StrCat(
                    " from ", is_output ? "Output" : "Input", "(\"", orig, "\") for Op ", op_name
                );
            }

            bool ConsumeInOutName(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .One(Scanner::LOWERLETTER)
                    .Any(Scanner::LOWERLETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .AnySpace()
                    .OneLiteral(":")
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeInOutRefOpen(absl::string_view* sp)
            {
                return Scanner(*sp)
                    .OneLiteral("Ref")
                    .AnySpace()
                    .OneLiteral("(")
                    .AnySpace()
                    .GetResult(sp);
            }

            bool ConsumeInOutRefClose(absl::string_view* sp)
            {
                return Scanner(*sp).OneLiteral(")").AnySpace().GetResult(sp);
            }

            bool ConsumeInOutNameOrType(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .One(Scanner::LETTER)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeInOutTimesType(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .OneLiteral("*")
                    .AnySpace()
                    .RestartCapture()
                    .One(Scanner::LETTER)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool ConsumeControlOutName(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .One(Scanner::LETTER)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .GetResult(sp, out);
            }

#define VERIFY(expr, ...)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            errors->push_back(                                                                     \
                strings::StrCat(__VA_ARGS__, InOutError(is_output, orig, op_def->name()))          \
            );                                                                                     \
            return;                                                                                \
        }                                                                                          \
    } while (false)

            void FinalizeInputOrOutput(
                absl::string_view spec,
                bool is_output,
                OpDef* op_def,
                std::vector<std::string>* errors
            )
            {
                OpDef::ArgDef* arg = is_output ? op_def->add_output_arg() : op_def->add_input_arg();

                absl::string_view orig(spec);

                // Parse "<name>:" at the beginning.
                absl::string_view tmp_name;
                VERIFY(ConsumeInOutName(&spec, &tmp_name), "Trouble parsing 'name:'");
                arg->set_name(tmp_name.data(), tmp_name.size());

                // Detect "Ref(...)".
                if (ConsumeInOutRefOpen(&spec)) {
                    arg->set_is_ref(true);
                }

                { // Parse "<name|type>" or "<name>*<name|type>".
                    absl::string_view first, second, type_or_attr;
                    VERIFY(
                        ConsumeInOutNameOrType(&spec, &first),
                        "Trouble parsing either a type or an attr name at '", spec, "'"
                    );
                    if (ConsumeInOutTimesType(&spec, &second)) {
                        arg->set_number_attr(first.data(), first.size());
                        type_or_attr = second;
                    } else {
                        type_or_attr = first;
                    }
                    DataType dt;
                    if (DataTypeFromString(type_or_attr, &dt)) {
                        arg->set_type(dt);
                    } else {
                        const OpDef::AttrDef* attr = FindAttr(type_or_attr, *op_def);
                        VERIFY(attr != nullptr, "Reference to unknown attr '", type_or_attr, "'");
                        if (attr->type() == "type") {
                            arg->set_type_attr(type_or_attr.data(), type_or_attr.size());
                        } else {
                            VERIFY(
                                attr->type() == "list(type)", "Reference to attr '", type_or_attr,
                                "' with type ", attr->type(), " that isn't type or list(type)"
                            );
                            arg->set_type_list_attr(type_or_attr.data(), type_or_attr.size());
                        }
                    }
                }

                // Closing ) for Ref(.
                if (arg->is_ref()) {
                    VERIFY(
                        ConsumeInOutRefClose(&spec),
                        "Did not find closing ')' for 'Ref(', instead found: '", spec, "'"
                    );
                }

                // Should not have anything else.
                VERIFY(spec.empty(), "Extra '", spec, "' unparsed at the end");

                // Int attrs that are the length of an input or output get a default
                // minimum of 1.
                if (!arg->number_attr().empty()) {
                    OpDef::AttrDef* attr = FindAttrMutable(arg->number_attr(), op_def);
                    if (attr != nullptr && !attr->has_minimum()) {
                        attr->set_has_minimum(true);
                        attr->set_minimum(1);
                    }
                } else if (!arg->type_list_attr().empty()) {
                    // If an input or output has type specified by a list(type) attr,
                    // it gets a default minimum of 1 as well.
                    OpDef::AttrDef* attr = FindAttrMutable(arg->type_list_attr(), op_def);
                    if (attr != nullptr && attr->type() == "list(type)" && !attr->has_minimum()) {
                        attr->set_has_minimum(true);
                        attr->set_minimum(1);
                    }
                }

                // If the arg's dtype is resource we should mark the op as stateful as it
                // likely touches a resource manager. This deliberately doesn't cover inputs /
                // outputs which resolve to resource via Attrs as those mostly operate on
                // resource handles as an opaque type (as opposed to ops which explicitly take
                // / produce resources).
                if (arg->type() == DT_RESOURCE) {
                    op_def->set_is_stateful(true);
                }
            }

#undef VERIFY

            std::string ControlOutError(absl::string_view orig, const std::string& op_name)
            {
                return absl::StrCat(" from ControlOutput(\"", orig, "\") for Op ", op_name);
            }

            void FinalizeControlOutput(
                absl::string_view name, OpDef* op_def, std::vector<std::string>* errors
            )
            {
                absl::string_view orig(name);

                // Parse control output name.
                absl::string_view tmp_name;
                if (!ConsumeControlOutName(&orig, &tmp_name)) {
                    errors->push_back(
                        absl::StrCat(
                            "Trouble parsing 'name:'", ControlOutError(orig, op_def->name())
                        )
                    );
                }

                *op_def->add_control_output() = std::string(tmp_name.data(), tmp_name.size());
            }

            int num_leading_spaces(absl::string_view s)
            {
                size_t i = 0;
                while (i < s.size() && s[i] == ' ') {
                    ++i;
                }
                return i;
            }

            bool ConsumeDocNameColon(absl::string_view* sp, absl::string_view* out)
            {
                return Scanner(*sp)
                    .One(Scanner::LETTER)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                    .StopCapture()
                    .AnySpace()
                    .OneLiteral(":")
                    .AnySpace()
                    .GetResult(sp, out);
            }

            bool IsDocNameColon(absl::string_view s)
            {
                return ConsumeDocNameColon(&s, nullptr /* out */);
            }

            void FinalizeDoc(
                const std::string& text, OpDef* op_def, std::vector<std::string>* errors
            )
            {
                std::vector<std::string> lines = str_util::Split(text, '\n');

                // Remove trailing spaces.
                for (std::string& line: lines) {
                    absl::StripTrailingAsciiWhitespace(&line);
                }

                // First non-blank line -> summary.
                int l = 0;
                while (static_cast<size_t>(l) < lines.size() && lines[l].empty()) {
                    ++l;
                }
                if (static_cast<size_t>(l) < lines.size()) {
                    op_def->set_summary(lines[l]);
                    ++l;
                }
                while (static_cast<size_t>(l) < lines.size() && lines[l].empty()) {
                    ++l;
                }

                // Lines until we see name: -> description.
                int start_l = l;
                while (static_cast<size_t>(l) < lines.size() && !IsDocNameColon(lines[l])) {
                    ++l;
                }
                int end_l = l;
                // Trim trailing blank lines from the description.
                while (start_l < end_l && lines[end_l - 1].empty()) {
                    --end_l;
                }
                std::string desc = absl::StrJoin(
                    absl::Span<const std::string>(lines.data() + start_l, end_l - start_l), "\n"
                );
                if (!desc.empty()) {
                    op_def->set_description(desc);
                }

                // name: description
                //   possibly continued on the next line
                //   if so, we remove the minimum indent
                absl::string_view name;
                std::vector<absl::string_view> description;
                while (static_cast<size_t>(l) < lines.size()) {
                    description.clear();
                    description.push_back(lines[l]);
                    ConsumeDocNameColon(&description.back(), &name);
                    ++l;
                    while (static_cast<size_t>(l) < lines.size() && !IsDocNameColon(lines[l])) {
                        description.push_back(lines[l]);
                        ++l;
                    }
                    // Remove any trailing blank lines.
                    while (!description.empty() && description.back().empty()) {
                        description.pop_back();
                    }
                    // Compute the minimum indent of all lines after the first.
                    int min_indent = -1;
                    for (size_t i = 1; i < description.size(); ++i) {
                        if (!description[i].empty()) {
                            int indent = num_leading_spaces(description[i]);
                            if (min_indent < 0 || indent < min_indent) {
                                min_indent = indent;
                            }
                        }
                    }
                    // Remove min_indent spaces from all lines after the first.
                    for (size_t i = 1; i < description.size(); ++i) {
                        if (!description[i].empty()) {
                            description[i].remove_prefix(min_indent);
                        }
                    }
                    // Concatenate lines into a single string.
                    const std::string complete(absl::StrJoin(description, "\n"));

                    // Find name.
                    bool found = false;
                    for (int i = 0; !found && i < op_def->input_arg_size(); ++i) {
                        if (op_def->input_arg(i).name() == name) {
                            op_def->mutable_input_arg(i)->set_description(complete);
                            found = true;
                        }
                    }
                    for (int i = 0; !found && i < op_def->output_arg_size(); ++i) {
                        if (op_def->output_arg(i).name() == name) {
                            op_def->mutable_output_arg(i)->set_description(complete);
                            found = true;
                        }
                    }
                    for (int i = 0; !found && i < op_def->attr_size(); ++i) {
                        if (op_def->attr(i).name() == name) {
                            op_def->mutable_attr(i)->set_description(complete);
                            found = true;
                        }
                    }
                    if (!found) {
                        errors->push_back(
                            absl::StrCat(
                                "No matching input/output/attr for name '", name,
                                "' from Doc() for Op ", op_def->name()
                            )
                        );
                        return;
                    }
                }
            }

        } // namespace

        OpDefBuilder::OpDefBuilder(std::string op_name)
        {
            op_def()->set_name(std::move(op_name));
        }

        OpDefBuilder& OpDefBuilder::Attr(std::string spec)
        {
            attrs_.push_back(std::move(spec));
            return *this;
        }

        OpDefBuilder& OpDefBuilder::Input(std::string spec)
        {
            inputs_.push_back(std::move(spec));
            return *this;
        }

        OpDefBuilder& OpDefBuilder::Output(std::string spec)
        {
            outputs_.push_back(std::move(spec));
            return *this;
        }

        OpDefBuilder& OpDefBuilder::ControlOutput(std::string name)
        {
            control_outputs_.push_back(std::move(name));
            return *this;
        }

        OpDefBuilder& OpDefBuilder::Doc(std::string text)
        {
#ifndef TF_LEAN_BINARY
            if (!doc_.empty()) {
                errors_.push_back(absl::StrCat("Extra call to Doc() for Op ", op_def()->name()));
            } else {
                doc_ = std::move(text);
            }
#endif
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetIsCommutative(bool is_commutative)
        {
            op_def()->set_is_commutative(is_commutative);
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetIsAggregate(bool is_aggregate)
        {
            op_def()->set_is_aggregate(is_aggregate);
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetIsStateful(bool is_stateful)
        {
            op_def()->set_is_stateful(is_stateful);
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetAllowsUninitializedInput(bool allows_uninitialized_input)
        {
            op_def()->set_allows_uninitialized_input(allows_uninitialized_input);
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetIsDistributedCommunication(bool is_distributed_communication)
        {
            op_def()->set_is_distributed_communication(is_distributed_communication);
            return *this;
        }

        OpDefBuilder& OpDefBuilder::Deprecated(int version, std::string explanation)
        {
            if (op_def()->has_deprecation()) {
                errors_.push_back(
                    absl::StrCat("Deprecated called twice for Op ", op_def()->name())
                );
            } else {
                OpDeprecation* deprecation = op_def()->mutable_deprecation();
                deprecation->set_version(version);
                deprecation->set_explanation(std::move(explanation));
            }
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetTypeConstructor(OpTypeConstructor c)
        {
            op_reg_data_.type_ctor = c;
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetForwardTypeFn(TypeInferenceFn f)
        {
            op_reg_data_.fwd_type_fn = f;
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetReverseTypeFn(int input_number, TypeInferenceFn f)
        {
            op_reg_data_.rev_type_fn = f;
            op_reg_data_.rev_type_input = input_number;
            return *this;
        }

        OpDefBuilder& OpDefBuilder::SetShapeFn(OpShapeInferenceFn fn)
        {
            if (op_reg_data_.shape_inference_fn != nullptr) {
                errors_.push_back(
                    absl::StrCat("SetShapeFn called twice for Op ", op_def()->name())
                );
            } else {
                op_reg_data_.shape_inference_fn = OpShapeInferenceFn(fn);
            }
            return *this;
        }

        OpDefBuilder& OpDefBuilder::AllowAttrTypeAny()
        {
            allow_attr_type_any_ = true;
            return *this;
        }

        absl::Status OpDefBuilder::Finalize(OpRegistrationData* op_reg_data) const
        {
            std::vector<std::string> errors = errors_;
            *op_reg_data = op_reg_data_;

            OpDef* op_def = &op_reg_data->op_def;
            for (absl::string_view attr: attrs_) {
                FinalizeAttr(attr, allow_attr_type_any_, op_def, &errors);
            }
            for (absl::string_view input: inputs_) {
                FinalizeInputOrOutput(input, false, op_def, &errors);
            }
            for (absl::string_view output: outputs_) {
                FinalizeInputOrOutput(output, true, op_def, &errors);
            }
            for (absl::string_view control_output: control_outputs_) {
                FinalizeControlOutput(control_output, op_def, &errors);
            }
            FinalizeDoc(doc_, op_def, &errors);

            if (op_reg_data->type_ctor != nullptr) {
                TF_RETURN_IF_ERROR(op_reg_data->type_ctor(op_def));
            }

            if (errors.empty()) {
                return absl::OkStatus();
            }
            return absl::InvalidArgumentError(absl::StrJoin(errors, "\n"));
        }

    } // namespace tensorflow

} // export

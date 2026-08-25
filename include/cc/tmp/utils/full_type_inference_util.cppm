module;

/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

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

#include <functional>
#include <string>
#include <vector>
#include "tensorflow/core/framework/full_type.pb.h"
#include "tensorflow/core/framework/op_def_builder.h"
#include "tensorflow/core/platform/statusor.h"
#include "absl/strings/str_cat.h"
#include "tensorflow/core/framework/full_type_util.h"
#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/protobuf/error_codes.pb.h"

export module cc_tmp:utils_full_type_inference_util;

import std;
import cc_abi;

export {

namespace tensorflow {

namespace full_type {

// TODO(mdan): Specific helpers won't get too far. Use a parser instead.

// Helpers that allow shorthand expression for the more common kinds of type
// inference functions.
// TODO(mdan): Break into separate header if it grows.
// Note: The information contained in these functions is also expressed to some
// extent by opdef attributes of the kind "input: T, output T". But in that
// context, T has strong DType semantics (i.e. T is DT_VARIANT for most
// interesting cases). The logic here extends to the op's FullType, so it's best
// to keep them separate, even though it leads to some redundancy. The
// same can be said about the shape inference function.

// Note: Unlike type constructors, which describe op definitions, type inference
// functions are meant to modify the type information of specific nodes (i.e.
// NodeDef proto).

// Helper for a no-op type inference function that indicates type inference
// should never alter the node's existing type.
// This is the same as not defining a type inference function at all, but
// explicitly communicates that intent.
TypeInferenceFn KeepExisting();

// A helper for a type inference function that indicates a single output that
// is a tensor of type t. This is the equivalent of a type construtor since it
// does not depend on inputs. This can be used with Tuple.
TypeInferenceFn Tensor(FullTypeId t);

// Helper for a type inference function which has the same type as the i'th
// input.
// The n arg allows multiple outputs, e.g. (T -> Product[T, T]).
// TODO(mdan): Drop defaults for readability if more non-(0, 1) cases appear.
// TODO(mdan): Rename to just Replicate.
TypeInferenceFn ReplicateInput(int i = 0, int n = 1);

// Helper for a type inference function which has the same type as a variadic
// number of inputs, e.g. (T, T -> Product[T]), (T, T, T -> Product[T]), etc.
// Infers the meet of the input types, in the sense of type meets (see
// https://en.wikipedia.org/wiki/Join_and_meet). This implementation is
// simplified to require the two inputs are a subtype of another.
TypeInferenceFn Merge();

// Helper for ops with semantics of encoding an input, that is,
// `T -> Encoded[T, <t>]`, where <t> is the encoded type.
TypeInferenceFn Encode(FullTypeId t, int i);

// Helper for ops with semantics of encoding an input, that is,
// `Encoded[T, <t>] -> T`, where <t> is the encoded type.
TypeInferenceFn Decode(FullTypeId t, int i);

// Helper for the type inference counterpart of Unary, that is (U ->
// PRODUCT[<t>[U]]), where <t> is parameterized by this factory, and U is the
// type of the input specified by element_idx.
// Note: when we migrate to a more formal type definition of an op, these two
// functions will naturally merge.
TypeInferenceFn UnaryContainerCreate(FullTypeId t, int element_idx);

// Helper for ops with semantics of adding an element to a container (<t>[T]),
// that is (<t>[U], V -> PRODUCT[<t>[Union[U, V]]]), where <t> is parameterized
// by this factory, U is the type of the input specified by container_idx, and V
// is the type of the input specified by element_idx. The homogeneous arg allows
// for constraints which guarantee that U and V must have a subtyping
// relationship, case in which either V or U is selected, whichever is the
// supertype.
TypeInferenceFn UnaryContainerAdd(FullTypeId t, int container_idx,
                                  int element_idx, bool homogeneous);

// Helper for ops with semantics of unstacking multiple inputs into a container
// `<t>[T1, ..., Tn]`, that is `T1, ..., Tn -> <t>[PRODUCT[U1, ..., Un]]`
// where Ui is obtained from an "unstack" mapping T -> U. Both <t> and the
// "unstack" mapping are parameterized by this factory.
// Note that when the "unstack" function is the identity function, this becomes
// equivalent to ContainerCreate.
TypeInferenceFn MultiaryUnstack(
    FullTypeId t, std::function<FullTypeDef(const FullTypeDef&)> unstack);

// Helper for ops with semantics of applying some transformation to the
// elements of a container:
// `<t>[PRODUCT[T1, ..., Tn]] -> <t>[PRODUCT[U1, ..., Un]]`,
// where Ui is obtained by applying a map T -> U. Both <t> and the "map"
// function are parameterized by this factory. See BatchTensor and ShardTensor
// for examples of "map".
TypeInferenceFn ContainerMap(
    FullTypeId t, int input_idx,
    std::function<FullTypeDef(const FullTypeDef&)> map);

// Helper for ops with semantics of repacking some element from a container to
// another `<t> -> <u>`, in a covariant way, that is, `<t>[T] -> <u>[T]`. <t>
// and <u> are parameterized by this factory. The input type is specified by
// element_idx.
TypeInferenceFn MapCovariant(FullTypeId t, FullTypeId u, int input_idx);

// Helper for ops with semantics of calling a function. The function is
// specified indirectly, as the name of an attribute that holds the actual
// function name.
TypeInferenceFn FunctionCall(const std::string& func_attr_name);

// Compose the type of a function by concatenating the outputs of multiple
// type inference functions. If func_list is {type inference function 1, type
// inference function 2} which return PRODUCT[T1], PRODUCT[T2] resprectively,
// the result is PRODUCT[T1, T2], This supports the Merge op that has an index
// output in addition to the result of the Merge type inference function.
TypeInferenceFn Tuple(const std::vector<TypeInferenceFn>& func_list);

// Auxiliary constructs to help creation of type inference functions.
// TODO(mdan): define these as type inference functions as well.

// Mapping function representing the type function for unstacking of
// Tensor (or Tensor-like) types. Note that this is a helper to use with
// other type inference functions; it's not a function itself.
// TODO(mdan): Replace with a trait, when available.
FullTypeDef UnstackTensor(const FullTypeDef& t);

// Mapping function representing the type function for an op that changes the
// batch size of dataset. Note that this is a helper to use with other type
// inference functions; it's not a function itself.
// TODO(mdan): Replace with a trait, when available.
FullTypeDef BatchTensor(const FullTypeDef& t);

// Mapping function representing the type function for an op that creates a
// fixed (given) number of tensors of a size calculated based on the input. Note
// that this is a helper to use with other type inference functions; it's not a
// function itself.
// TODO(mdan): Replace with a trait, when available.
FullTypeDef ShardTensor(const FullTypeDef& t);
}  // namespace full_type

}  // namespace tensorflow

// ==================================================================
// Implementation: full_type_inference_util.cc
// ==================================================================

namespace tensorflow {

namespace full_type {

// Note about error handling:
// For inputs which depend on the correctness of the op definition
// (i.e. if the op has three inputs, don't set an `i` that exceeds that),
// use DCHECK - an incorrect op def is considered a bug.
// Whereas for inputs that depend on the correctness of the graph (i.e. user
// used the correct ops), use Status - an incorrect graph is considered a user
// error.

TypeInferenceFn KeepExisting() { return nullptr; }

TypeInferenceFn Tensor(FullTypeId t) {
  return [t](const TypeRefVector& input_types,
             const FunctionTypeInferrer& infer_function_rets) {
    FullTypeDef ret_type;
    ret_type.set_type_id(TFT_PRODUCT);
    ret_type.add_args()->set_type_id(TFT_TENSOR);
    ret_type.mutable_args(0)->add_args()->set_type_id(t);
    return ret_type;
  };
}

TypeInferenceFn ReplicateInput(int i, int n) {
  return [i, n](const TypeRefVector& input_types,
                const FunctionTypeInferrer& infer_function_rets) {
    const FullTypeDef& in_type = input_types.at(i).get();
    FullTypeDef ret_type;
    if (in_type.type_id() != TFT_UNSET) {
      ret_type.set_type_id(TFT_PRODUCT);
      for (int k = 0; k < n; k++) {
        *(ret_type.add_args()) = in_type;
      }
    }
    return ret_type;
  };
}

TypeInferenceFn Merge() {
  return [](const TypeRefVector& input_types,
            const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK(!input_types.empty());

    FullTypeDef merged;
    for (int i = 0; i < input_types.size(); i++) {
      const auto& t = input_types[i].get();

      if (t.type_id() == TFT_UNSET) {
        continue;
      }

      if (IsSubtype(t, merged)) {
        merged = t;
        continue;
      }
      if (IsSubtype(merged, t)) {
        continue;
      }

      return absl::Status(
          absl::StatusCode::kInvalidArgument,
          absl::StrCat("expected compatible input types, but input ", i, ":\n",
                       t.DebugString(),
                       " is neither a subtype nor a supertype of the "
                       "combined inputs preceding it:\n",
                       merged.DebugString()));
    }

    FullTypeDef ret_type;
    if (merged.type_id() != TFT_UNSET) {
      ret_type.set_type_id(TFT_PRODUCT);
      *(ret_type.add_args()) = merged;
    }
    return ret_type;
  };
}

TypeInferenceFn Encode(FullTypeId t, int i) {
  return [t, i](const TypeRefVector& input_types,
                const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK(input_types.size() >= i);

    FullTypeDef ret_type;
    const FullTypeDef& in_t = input_types[i].get();
    if (in_t.type_id() == TFT_UNSET) {
      return ret_type;
    }

    ret_type.set_type_id(TFT_PRODUCT);

    auto* enc_type = ret_type.add_args();
    enc_type->set_type_id(TFT_ENCODED);
    *enc_type->add_args() = in_t;
    enc_type->add_args()->set_type_id(t);
    return ret_type;
  };
}

TypeInferenceFn Decode(FullTypeId t, int i) {
  return [t, i](const TypeRefVector& input_types,
                const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK(input_types.size() >= i);

    const FullTypeDef& in_t = input_types[i].get();

    const FullTypeId enc_tid = GetArgDefaultUnset(in_t, 1).type_id();
    if ((enc_tid != TFT_UNSET) && (enc_tid != t)) {
      return absl::Status(
          absl::StatusCode::kInvalidArgument,
          absl::StrCat("expected encoded type ", t, " for input ", i, ", got ",
                       in_t.DebugString()));
    }

    FullTypeDef ret_type;

    const FullTypeDef& out_t = GetArgDefaultUnset(in_t, 0);
    if (in_t.type_id() == TFT_UNSET) {
      return ret_type;
    }

    ret_type.set_type_id(TFT_PRODUCT);
    *ret_type.add_args() = out_t;
    return ret_type;
  };
}

TypeInferenceFn UnaryContainerCreate(FullTypeId t, int element_idx) {
  return [t, element_idx](const TypeRefVector& input_types,
                          const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK(input_types.size() >= element_idx);

    FullTypeDef ret_type;
    ret_type.set_type_id(TFT_PRODUCT);
    FullTypeDef* arg_t = ret_type.add_args();
    arg_t->set_type_id(t);
    *(arg_t->add_args()) = input_types[element_idx].get();

    return ret_type;
  };
}

TypeInferenceFn UnaryContainerAdd(FullTypeId t, int container_idx,
                                  int element_idx, bool homogeneous) {
  return [t, container_idx, element_idx, homogeneous](
             const TypeRefVector& input_types,
             const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK(input_types.size() >= container_idx);
    DCHECK(input_types.size() >= element_idx);

    FullTypeDef ret_type;
    ret_type.set_type_id(TFT_PRODUCT);
    FullTypeDef* cont_t = ret_type.add_args();
    cont_t->set_type_id(t);

    const FullTypeDef& in_cont_t = input_types[container_idx].get();
    const FullTypeDef& in_el_t = input_types[element_idx].get();

    if (in_cont_t.type_id() != TFT_UNSET) {
      if (in_cont_t.type_id() != t) {
        return absl::Status(
            absl::StatusCode::kInvalidArgument,
            absl::StrCat("expected container type ", t, " for input ",
                         container_idx, ", got ", in_cont_t.DebugString()));
      }
      *cont_t = in_cont_t;
    }

    VLOG(1) << "ContainerAddUnary: " << cont_t->DebugString() << ", "
            << in_el_t.DebugString() << ", " << container_idx << "; "
            << element_idx;
    for (const auto& tmp : input_types) {
      VLOG(1) << "  input: " << tmp.get().DebugString();
    }

    if (in_el_t.type_id() == TFT_UNSET) {
      return ret_type;
    }

    const FullTypeDef& el_t = GetArgDefaultUnset(*cont_t, 0);

    if (el_t.type_id() == TFT_UNSET) {
      cont_t->clear_args();
      *(cont_t->add_args()) = in_el_t;
      return ret_type;
    }

    if (IsSubtype(in_el_t, el_t)) {
      // Nothing to do, will not refine the container type based on a single
      // addition.
      return ret_type;
    }

    if (homogeneous) {
      return absl::Status(
          absl::StatusCode::kInvalidArgument,
          absl::StrCat("expected a subtype of ", el_t.DebugString(),
                       " for input ", element_idx,
                       " of a homogeneous container ", t, ", got ",
                       in_el_t.DebugString()));
    } else {
      // TODO(mdan): Implement if needed.
      return absl::Status(
          absl::StatusCode::kUnimplemented,
          absl::StrCat("need union types for heterogeneous containers.\n"
                       "A homogeneous container would expect a subtype of ",
                       el_t.DebugString(), " for input ", element_idx,
                       ", but got ", in_el_t.DebugString()));
    }
  };
}

TypeInferenceFn MultiaryUnstack(
    FullTypeId t, std::function<FullTypeDef(const FullTypeDef&)> unstack) {
  return [t, unstack](const TypeRefVector& input_types,
                      const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    FullTypeDef ret_type;
    ret_type.set_type_id(TFT_PRODUCT);
    FullTypeDef* cont_t = ret_type.add_args();
    cont_t->set_type_id(t);
    FullTypeDef* el_t = cont_t->add_args();
    el_t->set_type_id(TFT_PRODUCT);
    for (int element_idx = 0; element_idx < input_types.size(); ++element_idx) {
      *(el_t->add_args()) = unstack(input_types[element_idx].get());
    }
    return ret_type;
  };
}

FullTypeDef UnstackTensor(const FullTypeDef& t) {
  // For now, only TFT_TENSOR and TFT_RAGGED are supported and
  // only if they have a single argument (i.e. they don't specify a shape).
  // If these have a shape in the future, this function needs to changed
  // so that the output shape is computed based on the input shape and the
  // effect of the unstack operation (e.g. a dimension is removed).
  // TFT_UNSET is also allowed to support weak type inference where
  // not having a fulltype is allowed.
  DCHECK((t.type_id() == TFT_TENSOR) || (t.type_id() == TFT_RAGGED) ||
         (t.type_id() == TFT_UNSET));
  DCHECK_LE(t.args_size(), 1);
  return t;
}

TypeInferenceFn ContainerMap(
    FullTypeId t, int input_idx,
    std::function<FullTypeDef(const FullTypeDef&)> map) {
  return [t, input_idx, map](const TypeRefVector& input_types,
                             const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    DCHECK_GE(input_types.size(), input_idx);
    const FullTypeDef& in_cont_t = input_types.at(input_idx).get();
    FullTypeDef ret_type;
    if (in_cont_t.type_id() == TFT_UNSET) {
      return ret_type;
    }
    if (in_cont_t.type_id() != t) {
      return absl::Status(
          absl::StatusCode::kInvalidArgument,
          absl::StrCat("expected type ", t, " for input ", input_idx, ", got ",
                       in_cont_t.DebugString()));
    }
    ret_type.set_type_id(TFT_PRODUCT);
    FullTypeDef* out_cont_t = ret_type.add_args();
    out_cont_t->set_type_id(t);
    const FullTypeDef& in_el_t = GetArgDefaultUnset(in_cont_t, 0);
    if (in_el_t.type_id() == TFT_UNSET) {
      return ret_type;
    }
    if (in_el_t.type_id() != TFT_PRODUCT) {
      return absl::Status(
          absl::StatusCode::kInvalidArgument,
          absl::StrCat("expected PRODUCT element type for input ", input_idx,
                       ", got ", in_el_t.DebugString()));
    }
    FullTypeDef* out_el_t = out_cont_t->add_args();
    out_el_t->set_type_id(TFT_PRODUCT);
    for (int k = 0; k < in_el_t.args_size(); k++) {
      *(out_el_t->add_args()) = map(in_el_t.args(k));
    }
    return ret_type;
  };
}

TypeInferenceFn MapCovariant(FullTypeId t, FullTypeId u, int input_idx) {
  return
      [t, u, input_idx](const TypeRefVector& input_types,
                        const FunctionTypeInferrer& infer_function_rets)
          -> absl::StatusOr<FullTypeDef> {
        DCHECK_GE(input_types.size(), input_idx);
        const FullTypeDef& in_t = input_types.at(input_idx).get();
        FullTypeDef ret_type;
        if (in_t.type_id() == TFT_UNSET) {
          return ret_type;
        }
        if (in_t.type_id() != t) {
          return absl::Status(
              absl::StatusCode::kInvalidArgument,
              absl::StrCat("expected type ", t, " for input ", input_idx,
                           ", got ", in_t.DebugString()));
        }
        ret_type.set_type_id(TFT_PRODUCT);
        FullTypeDef* t = ret_type.add_args();
        t->set_type_id(u);
        *t->mutable_args() = in_t.args();
        return ret_type;
      };
}

TypeInferenceFn FunctionCall(const std::string& func_attr_name) {
  return [func_attr_name](const TypeRefVector& input_types,
                          const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    // TODO(b/224776031): Look up function name from attribute here.
    // This could be done by passing the node attributes to the lambda.
    // TODO(b/224776031): Is there a cleaner way to represent these
    // function-dependent types?
    return infer_function_rets(func_attr_name, input_types);
  };
}

TypeInferenceFn Tuple(const std::vector<TypeInferenceFn>& func_list) {
  return [func_list](const TypeRefVector& input_types,
                     const FunctionTypeInferrer& infer_function_rets)
             -> absl::StatusOr<FullTypeDef> {
    FullTypeDef ret_type;
    ret_type.set_type_id(TFT_PRODUCT);
    for (const auto& func : func_list) {
      const auto& status_or_t = func(input_types, infer_function_rets);
      TF_RETURN_WITH_CONTEXT_IF_ERROR(
          status_or_t.status(),
          absl::StrCat("for Tuple type infernce function ",
                       ret_type.args_size()));
      const FullTypeDef& t = status_or_t.value();
      if (t.type_id() == TFT_UNSET) {
        VLOG(1) << "For Tuple type inference function, function "
                << ret_type.args_size() << " is unset.";
        FullTypeDef unset_type;
        return unset_type;
      }
      if (t.type_id() != TFT_PRODUCT) {
        return absl::Status(
            absl::StatusCode::kInvalidArgument,
            absl::StrCat("for Tuple type inference function, expected result "
                         "of type inference function ",
                         ret_type.args_size(),
                         " to start with TFT_PRODUCT not ", t.DebugString()));
      }
      // If a type inferenence function describes a op with more than one
      // output, the default is to concatenate them. The is not needed for the
      // initial use case of the Merge op.
      for (int i = 0; i < t.args_size(); i++) {
        *(ret_type.add_args()) = t.args(i);
      }
    }
    return ret_type;
  };
}

FullTypeDef BatchTensor(const FullTypeDef& t) {
  // For now, just return the input type.
  // If the input type has a shape in the future, this function needs to be
  // changed so that the output shape is computed based on the input shape and
  // the effect of the op that changes the batch size (and this function would
  // require more information to do this computation).
  return t;
}

FullTypeDef ShardTensor(const FullTypeDef& t) {
  // For now, just return the input type.
  // If the input type has a shape in the future, this function needs to be
  // changed so that the output shape is computed based on the input shape and
  // the effect of the op that shards the input into multiple tensors (and this
  // function would require more information to do this computation).
  return t;
}

}  // namespace full_type

}  // namespace tensorflow

} // export

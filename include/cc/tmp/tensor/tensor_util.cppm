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

#include <algorithm>
#include <vector>
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor.pb.h"
#include "tensorflow/core/framework/tensor_shape.pb.h"
#include "tensorflow/core/framework/type_traits.h"
#include "tensorflow/core/platform/protobuf.h"
#include "tensorflow/core/platform/types.h"
#include <cmath>
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/variant.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/platform/tensor_coding.h"

export module cc_tmp:tensor_tensor_util;

import std;
import cc_abi;

export {

namespace tensorflow {
namespace tensor {

// DeepCopy returns a tensor whose contents are a deep copy of the
// contents of 'other'.  This function is intended only for
// convenience, not speed.
//
// REQUIRES: 'other' must point to data stored in CPU memory.
// REQUIRES: 'other' must be a Tensor of a copy-able type if
//           'other' is not appropriately memory-aligned.
Tensor DeepCopy(const Tensor& other);

// Deep copies input to output.  This function is similar to above, but assumes
// that the memory for the output has already been allocated.
void DeepCopy(const Tensor& input, Tensor* output);

// Concatenates 'tensors' into a single tensor, along their 0th dimension.
//
// REQUIRES: All members of 'tensors' must have the same data type parameter.
// REQUIRES: Each member of 'tensors' must have at least one dimension.
// REQUIRES: Each member of 'tensors' must point to data stored in CPU memory.
// REQUIRES: Each member of 'tensors' must be a Tensor of a copy-able type if it
//           is not appropriately memory-aligned.
absl::Status Concat(absl::Span<const Tensor> tensors, Tensor* result);

// Splits 'tensor' into 'sizes.size()' individual tensors, along the 0th
// dimension. The ith output tensor has 0th-dimension size 'sizes[i]'.
//
// REQUIRES: 'tensor' must have at least one dimension.
// REQUIRES: 'tensor.dim_size(0)' must equal the sum of the elements of 'sizes'.
// REQUIRES: 'tensor' must point to data stored in CPU memory.
// REQUIRES: 'tensor' must be a Tensor of a copy-able type if it is not
//           appropriately memory-aligned.
//
// Split() and Concat() are inverse operations.
absl::Status Split(const Tensor& tensor, absl::Span<const int64_t> sizes,
                   std::vector<Tensor>* result);

namespace internal {
void SetTensorProtoShape(absl::Span<const size_t> shape,
                         TensorShapeProto* shape_proto);

template <typename Type>
class TensorProtoFieldHelper : public std::false_type {};

#define DEFINE_PROTO_FIELD_HELPER(TYPE, FIELDNAME)                            \
  template <>                                                                 \
  class TensorProtoFieldHelper<TYPE> : public std::true_type {                \
   public:                                                                    \
    typedef decltype(                                                         \
        std::declval<TensorProto>().FIELDNAME##_val(0)) FieldType;            \
    typedef decltype(                                                         \
        std::declval<TensorProto>().FIELDNAME##_val()) RepeatedFieldType;     \
    typedef decltype(std::declval<TensorProto>().mutable_##FIELDNAME##_val()) \
        MutableRepeatedFieldType;                                             \
    static MutableRepeatedFieldType GetMutableField(TensorProto* proto) {     \
      return proto->mutable_##FIELDNAME##_val();                              \
    }                                                                         \
    static RepeatedFieldType& GetField(const TensorProto& proto) {            \
      return proto.FIELDNAME##_val();                                         \
    }                                                                         \
  }

// The argument pairs in the following macro instantiations encode the
// mapping from C++ type ($1) to repeated field name "$2_val" used for storing
// values in TensorProto. See tensorflow/core/framework/tensor.proto.
DEFINE_PROTO_FIELD_HELPER(float, float);
DEFINE_PROTO_FIELD_HELPER(double, double);
DEFINE_PROTO_FIELD_HELPER(int8_t, int);
DEFINE_PROTO_FIELD_HELPER(uint8_t, int);
DEFINE_PROTO_FIELD_HELPER(int16_t, int);
DEFINE_PROTO_FIELD_HELPER(uint16_t, int);
DEFINE_PROTO_FIELD_HELPER(int32_t, int);
DEFINE_PROTO_FIELD_HELPER(uint32_t, uint32);
DEFINE_PROTO_FIELD_HELPER(int64_t, int64);
DEFINE_PROTO_FIELD_HELPER(uint64_t, uint64);
DEFINE_PROTO_FIELD_HELPER(bool, bool);
DEFINE_PROTO_FIELD_HELPER(qint8, int);
DEFINE_PROTO_FIELD_HELPER(quint8, int);
DEFINE_PROTO_FIELD_HELPER(qint16, int);
DEFINE_PROTO_FIELD_HELPER(quint16, int);
DEFINE_PROTO_FIELD_HELPER(qint32, int);
DEFINE_PROTO_FIELD_HELPER(Eigen::half, half);
DEFINE_PROTO_FIELD_HELPER(bfloat16, half);
DEFINE_PROTO_FIELD_HELPER(complex64, scomplex);
DEFINE_PROTO_FIELD_HELPER(complex128, dcomplex);

#undef DEFINE_PROTO_HELPER

template <typename T>
struct CopyHelper {
  template <typename SrcIter, typename DstIter>
  static void ToArray(SrcIter begin, SrcIter end, DstIter dst) {
    using SrcType = typename std::iterator_traits<SrcIter>::value_type;
    using DstType = typename std::iterator_traits<DstIter>::value_type;
    std::transform(begin, end, dst, [](const SrcType& x) -> DstType {
      return static_cast<DstType>(x);
    });
  }
  template <typename SrcIter>
  static void ToArray(SrcIter begin, SrcIter end, SrcIter dst) {
    std::copy(begin, end, dst);
  }
  template <typename SrcIter, typename DstIter>
  static void FromArray(SrcIter begin, SrcIter end, DstIter dst) {
    ToArray(begin, end, dst);
  }
};

// Overloads for Eigen::half and bfloat16 that are 16 bits in size but are
// stored in an int32 field.
template <>
struct CopyHelper<Eigen::half> {
  template <typename SrcIter>
  static void ToArray(SrcIter begin, SrcIter end, Eigen::half* dst) {
    std::transform(begin, end, dst, [](int x) -> Eigen::half {
      return Eigen::numext::bit_cast<Eigen::half>(static_cast<uint16_t>(x));
    });
  }
  template <typename SrcIter, typename DstIter>
  static void FromArray(SrcIter begin, SrcIter end, DstIter dst) {
    std::transform(begin, end, dst, [](Eigen::half h) -> int {
      return static_cast<int>(Eigen::numext::bit_cast<uint16_t>(h));
    });
  }
};

template <>
struct CopyHelper<bfloat16> {
  template <typename SrcIter>
  static void ToArray(SrcIter begin, SrcIter end, bfloat16* dst) {
    std::transform(begin, end, dst, [](int x) -> bfloat16 {
      return Eigen::numext::bit_cast<bfloat16>(static_cast<uint16_t>(x));
    });
  }
  template <typename SrcIter, typename DstIter>
  static void FromArray(SrcIter begin, SrcIter end, DstIter dst) {
    std::transform(begin, end, dst, [](bfloat16 bf16) -> int {
      return static_cast<int>(Eigen::numext::bit_cast<uint16_t>(bf16));
    });
  }
};

// Overloads for complex types that store real and imaginary parts
// at indices 2*i and 2*i+1 in float or double field.
template <typename RealType>
struct CopyHelper<std::complex<RealType>> {
  template <typename SrcIter>
  static void ToArray(SrcIter begin, SrcIter end, std::complex<RealType>* dst) {
    RealType* real_dst = reinterpret_cast<RealType*>(dst);
    std::copy(begin, end, real_dst);
  }

  template <typename SrcIter, typename DstIter>
  static void FromArray(SrcIter begin, SrcIter end, DstIter dst) {
    size_t n = std::distance(begin, end);
    const RealType* real_begin = reinterpret_cast<const RealType*>(&(*begin));
    std::copy_n(real_begin, 2 * n, dst);
  }
};

// Helper class to extract and insert values into TensorProto represented as
// repeated fields.
template <typename T>
class TensorProtoHelper : public std::true_type {
 public:
  using FieldHelper = TensorProtoFieldHelper<T>;
  using FieldType = typename TensorProtoFieldHelper<T>::FieldType;

  static DataType GetDataType() { return DataTypeToEnum<T>::value; }

  // Returns the number of values of type T encoded in the proto.
  static size_t NumValues(const TensorProto& proto) {
    size_t raw_size = FieldHelper::GetField(proto).size();
    return is_complex<T>::value ? raw_size / 2 : raw_size;
  }

  static void AddValue(const T& value, TensorProto* proto) {
    const T* val_ptr = &value;
    AddValues(val_ptr, val_ptr + 1, proto);
  }

  static T GetValue(size_t index, const TensorProto& proto) {
    const size_t stride = is_complex<T>::value ? 2 : 1;
    T val;
    CopyHelper<T>::ToArray(
        FieldHelper::GetField(proto).begin() + stride * index,
        FieldHelper::GetField(proto).begin() + stride * (index + 1), &val);
    return val;
  }

  template <typename IterType>
  static void AddValues(IterType begin, IterType end, TensorProto* proto) {
    size_t n = std::distance(begin, end);
    FieldType* dst = AppendUninitialized(n, proto);
    CopyHelper<T>::FromArray(begin, end, dst);
  }

  template <typename IterType>
  static void CopyValues(IterType dst, const TensorProto& proto) {
    CopyHelper<T>::ToArray(FieldHelper::GetField(proto).begin(),
                           FieldHelper::GetField(proto).end(), dst);
  }

  static void Truncate(size_t new_size, TensorProto* proto) {
    if (is_complex<T>::value) new_size *= 2;
    FieldHelper::GetMutableField(proto)->Truncate(new_size);
  }

  static FieldType* AppendUninitialized(size_t n, TensorProto* proto) {
    if (is_complex<T>::value) n *= 2;
    auto* field = FieldHelper::GetMutableField(proto);
    field->Reserve(field->size() + n);
    return reinterpret_cast<FieldType*>(field->AddNAlreadyReserved(n));
  }
};

// Specialization for string.
template <>
class TensorProtoHelper<std::string> : public std::true_type {
 public:
  static DataType GetDataType() { return DataType::DT_STRING; }
  static void AddValue(const std::string& value, TensorProto* proto) {
    *proto->mutable_string_val()->Add() = value;
  }
  template <typename IterType>
  static void AddValues(IterType begin, IterType end, TensorProto* proto) {
    for (IterType it = begin; it != end; ++it) {
      AddValue(*it, proto);
    }
  }
  template <typename IterType>
  static void CopyToTensorContent(IterType begin, IterType end,
                                  TensorProto* proto) {
    AddValues(begin, end, proto);
  }
};

template <typename Type, typename IterType>
typename std::enable_if<internal::TensorProtoHelper<Type>::value,
                        TensorProto>::type
CreateTensorProto(IterType values_begin, IterType values_end,
                  const size_t values_size,
                  const absl::Span<const size_t> shape) {
  TensorProto tensor;
  TensorShapeProto tensor_shape_proto;
  internal::SetTensorProtoShape(shape, &tensor_shape_proto);
  if (TensorShape(tensor_shape_proto).num_elements() != values_size) {
    LOG(ERROR) << "Shape and number of values (" << values_size
               << ") are incompatible.";
    return tensor;
  }
  using TypeHelper = internal::TensorProtoHelper<Type>;
  tensor.set_dtype(TypeHelper::GetDataType());
  *tensor.mutable_tensor_shape() = std::move(tensor_shape_proto);
  TypeHelper::AddValues(values_begin, values_end, &tensor);
  return tensor;
}

}  // namespace internal

// Creates a 'TensorProto' with the specified shape and values. The dtype and a
// field to represent data values of the returned 'TensorProto' are determined
// based on Type. Note that unless the argument provided to `values` is already
// an absl::Span, `Type` will need to be provided as a template parameter--the
// compiler can't infer it:
//   auto proto = CreateTensorProtoSpan<float>(my_array, shape);
template <typename Type>
typename std::enable_if<internal::TensorProtoHelper<Type>::value,
                        TensorProto>::type
CreateTensorProtoSpan(const absl::Span<const Type> values,
                      const absl::Span<const size_t> shape) {
  return internal::CreateTensorProto<Type>(values.begin(), values.end(),
                                           values.size(), shape);
}

// Version of the above that's more convenient if `values` is an std::vector, in
// which case Type can automatically be inferred:
//   auto proto = CreateTensorProto(my_vector, shape);
template <typename Type>
typename std::enable_if<internal::TensorProtoHelper<Type>::value,
                        TensorProto>::type
CreateTensorProto(const std::vector<Type>& values,
                  const absl::Span<const size_t> shape) {
  // This awkward iterator passing is essentially just to support vector<bool>,
  // otherwise we could just represent the vector as a Span.
  return internal::CreateTensorProto<Type>(values.begin(), values.end(),
                                           values.size(), shape);
}

// Converts values in tensor to run-length encoded compressed form.
//
// The elements of a tensor can be stored in a TensorProto in one of the
// following two forms:
// 1. As a raw byte string in the field `tensor_content` containing the
//    serialized in-memory representation of the tensor.
// 2. As values of a repeated field depending on the datatype, e.g. that
//    values of a DT_FLOAT tensor would be stored in the repeated field
//    `float_val`.
// Storage scheme 2 may use a simple form of run-length encoding to compress
// data: If the values contains a tail of identical values, the repeated field
// will be truncated such that the number of values in the repeated field is
// less than the number of elements implied by the field`tensor_shape`. The
// original tensor can be recovered by repeating the final value in the repeated
// field.
//
// The TensorProto will be compressed if a) the tensor contains at least
// min_num_elements elements and b) the compressed tensor proto is would be at
// most the size of the original tensor proto divided by min_compression_ratio.
//
// Returns true if the tensor was compressed.
bool CompressTensorProtoInPlace(int64_t min_num_elements,
                                float min_compression_ratio,
                                TensorProto* tensor);

inline bool CompressTensorProtoInPlace(TensorProto* tensor) {
  static const int64_t kDefaultMinNumElements = 64;
  static const float kDefaultMinCompressionRatio = 2.0f;
  return CompressTensorProtoInPlace(kDefaultMinNumElements,
                                    kDefaultMinCompressionRatio, tensor);
}

// Make a TensorShape from the contents of shape_t. Shape_t must be a
// 1-dimensional tensor of type int32 or int64.
absl::Status MakeShape(const Tensor& shape_t, TensorShape* out);

}  // namespace tensor
}  // namespace tensorflow

// ==================================================================
// Implementation: tensor_util.cc
// ==================================================================

namespace tensorflow {
namespace tensor {

Tensor DeepCopy(const Tensor& other) {
  Tensor tmp = Tensor(other.dtype(), other.shape());
  DeepCopy(other, &tmp);
  return tmp;
}

void DeepCopy(const Tensor& input, Tensor* output) {
  if (DataTypeCanUseMemcpy(input.dtype())) {
    if (input.NumElements() > 0) {
      absl::string_view input_data = input.tensor_data();

      // We use StringPiece as a convenient map over the tensor buffer,
      // but we cast the type to get to the underlying buffer to do the
      // copy.
      absl::string_view output_data = output->tensor_data();
      memcpy(const_cast<char*>(output_data.data()), input_data.data(),
             input_data.size());
    }
  } else if (input.dtype() == DT_STRING) {
    output->unaligned_flat<tstring>() = input.unaligned_flat<tstring>();
  } else {
    CHECK_EQ(DT_VARIANT, input.dtype());
    output->unaligned_flat<Variant>() = input.unaligned_flat<Variant>();
  }
}

absl::Status Concat(const absl::Span<const Tensor> tensors, Tensor* result) {
  if (tensors.empty()) {
    return absl::InvalidArgumentError("Cannot concatenate zero tensors");
  }
  int64_t total_dim0_size = 0;
  for (const Tensor& tensor : tensors) {
    if (tensor.dims() == 0) {
      return absl::InvalidArgumentError(
          "Cannot concatenate a zero-dimensional tensor");
    }
    total_dim0_size += tensor.dim_size(0);
  }
  TensorShape shape = tensors[0].shape();
  shape.set_dim(0, total_dim0_size);

  const DataType dtype = tensors[0].dtype();
  for (int i = 1; i < tensors.size(); ++i) {
    if (tensors[i].dtype() != dtype) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Cannot concatenate tensors that have different data types.", " Got ",
          DataTypeString(dtype), " and ", DataTypeString(tensors[i].dtype()),
          "."));
    }
  }
  *result = Tensor(dtype, shape);

  if (DataTypeCanUseMemcpy(dtype)) {
    // We use StringPiece as a convenient map over the tensor buffer,
    // but we cast the type to get to the underlying buffer to do the
    // copy.
    absl::string_view to_data = result->tensor_data();

    int64_t offset = 0;
    for (const Tensor& tensor : tensors) {
      absl::string_view from_data = tensor.tensor_data();
      CHECK_LE(offset + from_data.size(), to_data.size());
      memcpy(const_cast<char*>(to_data.data()) + offset, from_data.data(),
             from_data.size());

      offset += from_data.size();
    }
  } else {
    if (dtype != DT_STRING) {
      return absl::InternalError("Unexpected data type");
    }
    tstring* to_strings = result->unaligned_flat<tstring>().data();

    int64_t offset = 0;
    for (const Tensor& tensor : tensors) {
      auto from_strings = tensor.flat<tstring>();
      CHECK_LE(offset + tensor.NumElements(), result->NumElements());
      for (int i = 0; i < tensor.NumElements(); ++i) {
        to_strings[offset + i] = from_strings(i);
      }

      offset += tensor.NumElements();
    }
  }

  return absl::OkStatus();
}

absl::Status Split(const Tensor& tensor, const absl::Span<const int64_t> sizes,
                   std::vector<Tensor>* result) {
  if (tensor.dims() == 0) {
    return absl::InvalidArgumentError("Cannot split a zero-dimensional tensor");
  }
  int64_t total_size = 0;
  for (int64_t size : sizes) {
    total_size += size;
  }
  if (total_size != tensor.dim_size(0)) {
    return absl::InvalidArgumentError(
        "The values in 'sizes' do not sum to the zeroth-dimension size of "
        "'tensor'");
  }

  if (DataTypeCanUseMemcpy(tensor.dtype())) {
    absl::string_view from_data = tensor.tensor_data();

    int64_t offset = 0;
    for (int64_t size : sizes) {
      TensorShape shape = tensor.shape();
      shape.set_dim(0, size);
      result->emplace_back(tensor.dtype(), shape);
      Tensor* split = &(*result)[result->size() - 1];

      // We use StringPiece as a convenient map over the tensor buffer,
      // but we cast the type to get to the underlying buffer to do the
      // copy.
      absl::string_view to_data = split->tensor_data();
      CHECK_LE(offset + to_data.size(), from_data.size());
      memcpy(const_cast<char*>(to_data.data()), from_data.data() + offset,
             to_data.size());

      offset += to_data.size();
    }
  } else {
    if (tensor.dtype() != DT_STRING) {
      return absl::InternalError("Unexpected data type");
    }
    auto from_strings = tensor.flat<tstring>();

    int64_t offset = 0;
    for (int64_t size : sizes) {
      TensorShape shape = tensor.shape();
      shape.set_dim(0, size);
      result->emplace_back(tensor.dtype(), shape);
      Tensor& split = (*result)[result->size() - 1];
      tstring* to_strings = split.unaligned_flat<tstring>().data();

      CHECK_LE(offset + split.NumElements(), tensor.NumElements());
      for (int i = 0; i < split.NumElements(); ++i) {
        to_strings[i] = from_strings(offset + i);
      }

      offset += split.NumElements();
    }
  }

  return absl::OkStatus();
}

namespace internal {
void SetTensorProtoShape(const absl::Span<const size_t> shape,
                         TensorShapeProto* shape_proto) {
  for (auto dim : shape) {
    shape_proto->mutable_dim()->Add()->set_size(dim);
  }
}

template <typename T>
bool CompressTensorContent(float min_compression_ratio,
                           const TensorShape& shape, TensorProto* tensor) {
  using TypeHelper = internal::TensorProtoHelper<T>;
  using FieldType = typename internal::TensorProtoHelper<T>::FieldType;
  const int64_t num_tensor_values = shape.num_elements();
  const int64_t num_bytes = tensor->tensor_content().size();
  const int64_t num_raw_values = num_bytes / sizeof(T);
  if (num_raw_values != num_tensor_values) {
    // Invalid or too small.
    return false;
  }
  int64_t last_offset = num_bytes - 1;
  int64_t prev_offset = last_offset - sizeof(T);
  // Inspect individual raw bytes sizeof(T) bytes apart in adjacent elements,
  // starting from the end, to find the last pair of elements that are not
  // identical.
  while (prev_offset >= 0) {
    if (tensor->tensor_content()[prev_offset] !=
        tensor->tensor_content()[last_offset]) {
      break;
    }
    --last_offset;
    --prev_offset;
  }
  if (prev_offset == -1) {
    // It this is a splat of value 0, it does not need an explicit value, just
    // erase the content.
    T splat_value;
    port::CopySubrangeToArray(tensor->tensor_content(), 0, sizeof(T),
                              reinterpret_cast<char*>(&splat_value));
    if (splat_value == T(0)) {
      tensor->clear_tensor_content();
      return true;
    }
  }
  // Round up to the next whole number of element of type T.
  const int64_t new_num_values = last_offset / sizeof(T) + 1;
  if (new_num_values * (is_complex<T>::value ? 2 : 1) * sizeof(FieldType) >
      static_cast<int64_t>(num_bytes / min_compression_ratio)) {
    return false;
  }
  // Copy values to truncated repeated field.
  if constexpr (sizeof(FieldType) == sizeof(T)) {
    FieldType* dst_ptr =
        TypeHelper::AppendUninitialized(new_num_values, tensor);
    port::CopySubrangeToArray(tensor->tensor_content(), 0,
                              new_num_values * sizeof(T),
                              reinterpret_cast<char*>(dst_ptr));
    tensor->clear_tensor_content();
  } else if constexpr (sizeof(T) > 1) {
    // Copy raw bytes to temp array first, then cast.
    gtl::InlinedVector<T, 64> tmp;
    if (new_num_values >= tmp.max_size()) return false;
    tmp.resize(new_num_values);

    port::CopySubrangeToArray(tensor->tensor_content(), 0,
                              new_num_values * sizeof(T),
                              reinterpret_cast<char*>(tmp.data()));
    tensor->clear_tensor_content();
    TypeHelper::AddValues(tmp.begin(), tmp.end(), tensor);
  } else {
    // Copy and cast, one byte at a time.
    for (int64_t i = 0; i < new_num_values; ++i) {
      char c = tensor->tensor_content()[i];
      TypeHelper::AddValue(static_cast<T>(c), tensor);
    }
    tensor->clear_tensor_content();
  }
  return true;
}

template <typename T>
inline bool PackedValuesNotEqual(T a, T b) {
  return a != b;
}
template <>
inline bool PackedValuesNotEqual(float a, float b) {
  return reinterpret_cast<int32_t&>(a) != reinterpret_cast<int32_t&>(b);
}
template <>
inline bool PackedValuesNotEqual(double a, double b) {
  return reinterpret_cast<int64_t&>(a) != reinterpret_cast<int64_t&>(b);
}
template <typename RealType>
inline bool PackedValuesNotEqual(const std::complex<RealType>& a,
                                 const std::complex<RealType>& b) {
  return PackedValuesNotEqual(a.real(), b.real()) ||
         PackedValuesNotEqual(a.imag(), b.imag());
}

// Integer can't be negative zero.
template <typename T,
          typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
static bool IsNegativeZero(T value) {
  return false;
}

template <typename T,
          typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr>
static bool IsNegativeZero(T value) {
  return value == T(0) && std::signbit(value);
}

template <typename T>
static bool IsNegativeZero(std::complex<T> value) {
  return IsNegativeZero(value.real()) || IsNegativeZero(value.imag());
}

static bool IsNegativeZero(Eigen::QUInt8 value) { return false; }
static bool IsNegativeZero(Eigen::QInt8 value) { return false; }
static bool IsNegativeZero(Eigen::QUInt16 value) { return false; }
static bool IsNegativeZero(Eigen::QInt16 value) { return false; }
static bool IsNegativeZero(Eigen::QInt32 value) { return false; }
static bool IsNegativeZero(Eigen::half value) {
  return IsNegativeZero<float>(static_cast<float>(value));
}
static bool IsNegativeZero(Eigen::bfloat16 value) {
  return IsNegativeZero<float>(static_cast<float>(value));
}

template <typename T>
bool CompressRepeatedField(float min_compression_ratio,
                           const TensorShape& shape, TensorProto* tensor) {
  using TypeHelper = internal::TensorProtoHelper<T>;
  using FieldType = typename internal::TensorProtoHelper<T>::FieldType;
  const int64_t num_tensor_values = shape.num_elements();
  const int64_t num_proto_values = TypeHelper::NumValues(*tensor);

  // Notice that for complex types the tensor is stored as an array of up to
  // 2 * num_tensor_values real values (real and imaginary parts), possibly
  // truncated. A 0-splat does not need any value present and is maximally
  // compressed.
  if (num_proto_values == 0) return false;

  const T last_value = TypeHelper::GetValue(num_proto_values - 1, *tensor);
  int64_t last_index = 0;
  for (int64_t i = num_proto_values - 2; i >= 0 && last_index == 0; --i) {
    const T cur_value = TypeHelper::GetValue(i, *tensor);
    if (PackedValuesNotEqual(cur_value, last_value)) {
      last_index = i + 1;
    }
  }

  // Detect all zeroes tensors: this is default value and the content can be
  // erased entirely.
  if (last_index == 0 && last_value == T(0) && !IsNegativeZero(last_value)) {
    TypeHelper::Truncate(0, tensor);
    return true;
  }

  const int64_t num_truncated_proto_values = last_index + 1;
  const int64_t num_bytes_as_field =
      num_truncated_proto_values * sizeof(FieldType);
  const int64_t num_bytes_as_tensor_content = num_tensor_values * sizeof(T);
  const int64_t num_bytes_before = num_proto_values * sizeof(FieldType);
  if (std::min(num_bytes_as_field, num_bytes_as_tensor_content) >
      static_cast<int64_t>(num_bytes_before / min_compression_ratio)) {
    return false;
  }
  if (num_bytes_as_field <= num_bytes_as_tensor_content) {
    TypeHelper::Truncate(num_truncated_proto_values, tensor);
  } else {
    gtl::InlinedVector<T, 64> tmp;
    if (num_proto_values == 1) {
      // Splat case.
      tmp.resize(num_tensor_values, last_value);
    } else {
      tmp.resize(num_tensor_values, T(0));
      TypeHelper::CopyValues(tmp.begin(), *tensor);
    }
    TypeHelper::Truncate(0, tensor);
    port::CopyFromArray(tensor->mutable_tensor_content(),
                        reinterpret_cast<const char*>(tmp.data()),
                        num_bytes_as_tensor_content);
  }
  return true;
}

template <typename T>
bool CompressTensorProtoInPlaceImpl(int64_t min_num_elements,
                                    float min_compression_ratio,
                                    TensorProto* tensor) {
  const TensorShape shape(tensor->tensor_shape());
  const int64_t num_tensor_values = shape.num_elements();
  if (num_tensor_values < min_num_elements) {
    return false;
  }
  if (tensor->tensor_content().empty()) {
    return CompressRepeatedField<T>(min_compression_ratio, shape, tensor);
  } else {
    return CompressTensorContent<T>(min_compression_ratio, shape, tensor);
  }
  return true;
}

}  // namespace internal

#define HANDLE_COMPRESS_CASE(TF_TYPE)                                  \
  case TF_TYPE:                                                        \
    return internal::CompressTensorProtoInPlaceImpl<                   \
        EnumToDataType<TF_TYPE>::Type>(min_num_elements,               \
                                       min_compression_ratio, tensor); \
    break

bool CompressTensorProtoInPlace(int64_t min_num_elements,
                                float min_compression_ratio,
                                TensorProto* tensor) {
  switch (tensor->dtype()) {
    HANDLE_COMPRESS_CASE(DT_FLOAT);
    HANDLE_COMPRESS_CASE(DT_DOUBLE);
    HANDLE_COMPRESS_CASE(DT_COMPLEX64);
    HANDLE_COMPRESS_CASE(DT_COMPLEX128);
    HANDLE_COMPRESS_CASE(DT_UINT8);
    HANDLE_COMPRESS_CASE(DT_INT8);
    HANDLE_COMPRESS_CASE(DT_UINT16);
    HANDLE_COMPRESS_CASE(DT_INT16);
    HANDLE_COMPRESS_CASE(DT_UINT32);
    HANDLE_COMPRESS_CASE(DT_INT32);
    HANDLE_COMPRESS_CASE(DT_UINT64);
    HANDLE_COMPRESS_CASE(DT_INT64);
    HANDLE_COMPRESS_CASE(DT_BOOL);
    HANDLE_COMPRESS_CASE(DT_QUINT8);
    HANDLE_COMPRESS_CASE(DT_QINT8);
    HANDLE_COMPRESS_CASE(DT_QUINT16);
    HANDLE_COMPRESS_CASE(DT_QINT16);
    HANDLE_COMPRESS_CASE(DT_QINT32);
    HANDLE_COMPRESS_CASE(DT_HALF);
    HANDLE_COMPRESS_CASE(DT_BFLOAT16);
    default:
      return false;
  }
}

#undef HANDLE_COMPRESS_CASE

absl::Status MakeShape(const Tensor& shape, TensorShape* out) {
  if (!TensorShapeUtils::IsVector(shape.shape())) {
    return absl::InvalidArgumentError(
        absl::StrCat("shape must be a vector of {int32,int64}, got shape ",
                     shape.shape().DebugString()));
  }
  if (shape.dtype() == DataType::DT_INT32) {
    auto vec = shape.flat<int32_t>();
    return TensorShapeUtils::MakeShape(vec.data(), vec.size(), out);
  } else if (shape.dtype() == DataType::DT_INT64) {
    auto vec = shape.flat<int64_t>();
    return TensorShapeUtils::MakeShape(vec.data(), vec.size(), out);
  } else {
    return absl::InvalidArgumentError(
        "shape must be a vector of {int32,int64}.");
  }
}

}  // namespace tensor
}  // namespace tensorflow

} // export

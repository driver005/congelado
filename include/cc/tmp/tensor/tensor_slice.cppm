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

#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/tensor_slice.pb.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/gtl/inlined_vector.h"
#include "tensorflow/core/lib/strings/numbers.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/logging.h"
#include "unsupported/Eigen/CXX11/Tensor" // from @eigen_archive

#include <limits>
#include <string>
#include <vector>

export module cc_tmp:tensor_tensor_slice;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // A tensor slice represents a slice of a given tensor. It is represented by a
        // list of (start, length) pairs, where the size of the list is the rank of the
        // tensor.

        class TensorSlice
        {
        public:
            // Construct a tensor slice: you have a number of ways:
            // -- creating an empty slice
            // -- from just a dimension (in this case it will create a full slice)
            // -- from an array of pairs of integers.
            // -- from a TensorSliceProto protocol buffer
            // -- from a string format of "start,length:start,length..." where each
            //    "start,length" pair represents the slice on one dimension. We allow a
            //    special "-" that means "everything for this dimension". One such example
            //    is:  0,10:-:14,1:-:-
            TensorSlice() {}

            explicit TensorSlice(int dim);
            explicit TensorSlice(const TensorSliceProto& proto);
            explicit TensorSlice(std::initializer_list<std::pair<int64_t, int64_t>> extents);

            // This factory methods should be used instead of the constructor that takes a
            // `TensorSliceProto` if calling code cannot validate that the sizes specify a
            // valid `TensorSlice`.
            static absl::Status
            BuildTensorSlice(const TensorSliceProto& proto, TensorSlice* output);

            static absl::Status Parse(const std::string& str, TensorSlice* output);

            static TensorSlice ParseOrDie(const std::string& str)
            {
                TensorSlice ret;
                absl::Status s = Parse(str, &ret);
                if (!s.ok()) {
                    LOG(FATAL) << "Could not parse TensorSlice";
                }
                return ret;
            }

            void Clear();

            // Accessors
            int dims() const
            {
                return starts_.size();
            }

            int64_t start(int d) const
            {
                DCHECK_GE(d, 0);
                DCHECK_LT(d, dims());
                return starts_[d];
            }

            int64_t length(int d) const
            {
                DCHECK_GE(d, 0);
                DCHECK_LT(d, dims());
                return lengths_[d];
            }

            int64_t end(int d) const
            {
                DCHECK_GE(d, 0);
                DCHECK_LT(d, dims());
                return start(d) + length(d);
            }

            void set_start(int d, int64_t x)
            {
                DCHECK_GE(d, 0);
                DCHECK_LT(d, dims());
                DCHECK_GE(x, 0);
                starts_[d] = x;
            }

            void set_length(int d, int64_t x)
            {
                DCHECK_GE(d, 0);
                DCHECK_LT(d, dims());
                lengths_[d] = x;
            }

            // If we have a full slice along dimension "d".
            bool IsFullAt(int d) const
            {
                return lengths_[d] == kFullExtent && starts_[d] == 0;
            }

            // If this is a full slice, i.e. IsFullAt(d) for every d.
            bool IsFull() const;

            // Set the slice to be a full slice of "dim" dimensions
            void SetFullSlice(int dim);

            // Extend a slice to "dim" dimensions: all the added dimensions are full.
            // Requires: dim >= dims().
            void Extend(int dim);

            // Conversion of a TensorSlice to other formats
            void AsProto(TensorSliceProto* proto) const;
            std::string DebugString() const;

            // Fill *indices and *sizes from *this (so that we can use the slice()
            // function in eigen tensor). We need a tensor shape in case some of the
            // slices are full slices.
            // We allow NDIMS to be greater than dims(), in which case we will pad the
            // higher dimensions with trivial dimensions.
            template<int NDIMS>
            void FillIndicesAndSizes(
                const TensorShape& shape,
                Eigen::DSizes<Eigen::DenseIndex, NDIMS>* indices,
                Eigen::DSizes<Eigen::DenseIndex, NDIMS>* sizes
            ) const;

            // Interaction with other TensorSlices.

            // Compute the intersection with another slice and if "result" is not
            // nullptr, store the results in *result; returns true if there is any real
            // intersection.
            bool Intersect(const TensorSlice& other, TensorSlice* result) const;

            // A short hand.
            bool Overlaps(const TensorSlice& other) const
            {
                return Intersect(other, nullptr);
            }

            // Equals iff "*this" and "other" are logically equivalent.
            bool operator==(const TensorSlice& other) const;

            bool operator!=(const TensorSlice& other) const
            {
                return !(*this == other);
            }

            // Interaction with TensorShape.

            // Slices a shape and stores the result into *result_shape.
            // Requires that the shape and *this have the same rank.
            // For example, given a tensor shape of {3, 4, 5}, and a slice of
            // 1,2:-:0,2, the result shape is {2, 4, 2}.
            absl::Status
            SliceTensorShape(const TensorShape& shape, TensorShape* result_shape) const;

            // Given slice "sub" where "sub" is fully contained in *this,
            // (meaning that the intersection of "sub" and *this equals "sub"), computes
            // the "relative" slice of "sub" with respect to *this.
            //
            // In other words, if we use A>S to denote slicing a shape S with a slice A,
            // then the function is computing a slice X such that:
            //   X > (this > S) = sub > S
            // for any shape S.
            //
            // In general, along every dimension, the start of the relative slice is the
            // start of the "sub" slice minus the start of *this; the length of the
            // relative slice is the length of the "sub" slice.
            //
            // For example, say we have a shape of {3, 4, 5}, "this" is 0,2:-:1,2, and
            // "sub" is 1,1:2:2,1,2, then the related slice is 1,1:2,2:0,2.
            //
            // The caller needs to make sure that "sub" is indeed a sub-slice of *this;
            // otherwise the result is undefined.
            void ComputeRelative(const TensorSlice& sub, TensorSlice* relative) const;

            // Updates the slice in such a way that it fully covers "other" slice.
            // Note, "other" slice should refer to the same tensor shape.
            // Example:
            //   given a slice [2:4, :, 3:] and "other" slice [:, 1:4, 2:4] the
            //   updated slice would be [:, :, 2:]. Here is why:
            //   dim 0: "2:4"  U  ":"    ->  ":"
            //   dim 1: ":"    U  "1-4"  ->  ":"
            //   dim 2: "3:"   U  "2:4"  ->  "2:"
            void UpdateToCover(const TensorSlice& other);

            // Returns true if the length field was specified in an Extent.
            static bool HasExtentLength(const TensorSliceProto::Extent& extent);

            // Returns the value of the length field in an Extent, or -1 if it
            // is not present.
            static int64_t GetExtentLength(const TensorSliceProto::Extent& extent);

        private:
            // a length value of kFullExtent (-1) means we have a full slice at this
            // dimension. It's defined in tensor_slice.cc.
            static const int64_t kFullExtent;

            // TODO(yangke): switch to Eigen once it supports variable size arrays.
            // A value of
            absl::InlinedVector<int64_t, 4UL> starts_;
            absl::InlinedVector<int64_t, 4UL> lengths_;
        };

        template<int NDIMS>
        void TensorSlice::FillIndicesAndSizes(
            const TensorShape& shape,
            Eigen::DSizes<Eigen::DenseIndex, NDIMS>* indices,
            Eigen::DSizes<Eigen::DenseIndex, NDIMS>* sizes
        ) const
        {
            CHECK_EQ(shape.dims(), dims())
                << "Incompatible dimensions between shape "
                << "slices: shape = " << shape.DebugString() << ", slice = " << DebugString();
            CHECK_GE(NDIMS, dims()) << "Asking for a " << NDIMS << "-dim slice from "
                                    << "a slice of dimension " << dims();
            for (int d = 0; d < dims(); ++d) {
                if (IsFullAt(d)) {
                    (*indices)[d] = 0;
                    (*sizes)[d] = shape.dim_size(d);
                } else {
                    (*indices)[d] = starts_[d];
                    (*sizes)[d] = lengths_[d];
                }
            }
            for (int d = dims(); d < NDIMS; ++d) {
                (*indices)[d] = 0;
                (*sizes)[d] = 1;
            }
        }

    } // namespace tensorflow

    // ==================================================================
    // Implementation: tensor_slice.cc
    // ==================================================================

    namespace tensorflow {

        TensorSlice::TensorSlice(int dim)
        {
            SetFullSlice(dim);
        }

        TensorSlice::TensorSlice(const TensorSliceProto& proto)
        {
            starts_.reserve(proto.extent_size());
            lengths_.reserve(proto.extent_size());
            for (const auto& e: proto.extent()) {
                starts_.push_back(e.start());
                lengths_.push_back(GetExtentLength(e));
            }
        }

        TensorSlice::TensorSlice(std::initializer_list<std::pair<int64_t, int64_t>> extents)
        {
            starts_.reserve(extents.size());
            lengths_.reserve(extents.size());
            for (const auto& e: extents) {
                starts_.push_back(e.first);
                lengths_.push_back(e.second);
            }
        }

        absl::Status
        TensorSlice::BuildTensorSlice(const TensorSliceProto& proto, TensorSlice* output)
        {
            output->Clear();
            output->starts_.reserve(proto.extent_size());
            output->lengths_.reserve(proto.extent_size());
            for (const auto& e: proto.extent()) {
                int64_t l = GetExtentLength(e);
                if (e.start() != 0 || l != kFullExtent) {
                    if (e.start() < 0 || l <= 0) {
                        return absl::InvalidArgumentError(
                            absl::StrCat(
                                "Expected non-negative start and positive length but got start = ",
                                e.start(), ", length = ", l, ": extent = ", e.ShortDebugString()
                            )
                        );
                    }
                    // Calculating the extent end must not cause signed integer overflow.
                    if (static_cast<uint64_t>(e.start()) + static_cast<uint64_t>(e.length()) >
                        std::numeric_limits<int64_t>::max()) {
                        return absl::InvalidArgumentError(
                            absl::StrCat(
                                "Extent end exceeds the maximum possible size: extent = ",
                                e.ShortDebugString()
                            )
                        );
                    }
                }
                output->starts_.push_back(e.start());
                output->lengths_.push_back(l);
            }

            return absl::OkStatus();
        }

        absl::Status TensorSlice::Parse(const std::string& str, TensorSlice* slice)
        {
            std::vector<std::string> items = str_util::Split(str, ':', str_util::SkipEmpty());
            slice->starts_.reserve(items.size());
            slice->lengths_.reserve(items.size());
            for (const std::string& x: items) {
                int64_t s, l;
                if (x == "-") {
                    // "everything"
                    s = 0;
                    l = kFullExtent;
                } else {
                    std::vector<std::string> sl = str_util::Split(x, ',', str_util::SkipEmpty());
                    if (sl.size() != 2 || !absl::SimpleAtoi(sl[0], &s) ||
                        !absl::SimpleAtoi(sl[1], &l)) {
                        return absl::InvalidArgumentError(
                            absl::StrCat(
                                "Expected a pair of numbers or '-' "
                                "but got '",
                                x, "': string = ", str
                            )
                        );
                    }
                    if (s < 0 || l <= 0) {
                        return absl::InvalidArgumentError(
                            absl::StrCat(
                                "Expected non-negative start and "
                                "positive length but got start = ",
                                s, ", length = ", l, ": string = ", str
                            )
                        );
                    }
                }
                slice->starts_.push_back(s);
                slice->lengths_.push_back(l);
            }

            return absl::OkStatus();
        }

        void TensorSlice::Clear()
        {
            starts_.clear();
            lengths_.clear();
        }

        bool TensorSlice::IsFull() const
        {
            for (int d = 0; d < dims(); ++d) {
                if (!IsFullAt(d)) {
                    return false;
                }
            }
            return true;
        }

        void TensorSlice::SetFullSlice(int dim)
        {
            Clear();
            starts_.reserve(dim);
            lengths_.reserve(dim);
            for (int d = 0; d < dim; ++d) {
                starts_.push_back(0);
                lengths_.push_back(kFullExtent);
            }
        }

        void TensorSlice::Extend(int dim)
        {
            int old_dim = dims();
            DCHECK_LE(old_dim, dim);
            starts_.resize(dim);
            lengths_.resize(dim);
            for (int d = old_dim; d < dim; ++d) {
                starts_[d] = 0;
                lengths_[d] = kFullExtent;
            }
        }

        void TensorSlice::AsProto(TensorSliceProto* proto) const
        {
            for (int d = 0; d < dims(); ++d) {
                TensorSliceProto::Extent* e = proto->add_extent();
                // We only need to record the explicit slice for non-full slices
                if (!IsFullAt(d)) {
                    e->set_start(starts_[d]);
                    e->set_length(lengths_[d]);
                }
            }
        }

        std::string TensorSlice::DebugString() const
        {
            std::string buffer;
            bool first = true;
            for (int d = 0; d < dims(); ++d) {
                if (!first) {
                    buffer.append(":");
                }
                if (IsFullAt(d)) {
                    buffer.append("-");
                } else {
                    absl::StrAppend(&buffer, starts_[d], ",", lengths_[d]);
                }
                first = false;
            }
            return buffer;
        }

        bool TensorSlice::Intersect(const TensorSlice& other, TensorSlice* result) const
        {
            // First, if two slices have different ranks, they obviously don't overlap
            // -- in fact they are not compatible.
            if (dims() != other.dims()) {
                return false;
            }

            // Setting the result to the right dimension
            if (result) {
                result->SetFullSlice(dims());
            }
            // The two slices overlap if they overlap in all dimensions.
            for (int d = 0; d < dims(); ++d) {
                if (IsFullAt(d)) {
                    if (result) {
                        result->set_start(d, other.start(d));
                        result->set_length(d, other.length(d));
                    }
                } else if (other.IsFullAt(d)) {
                    if (result) {
                        result->set_start(d, start(d));
                        result->set_length(d, length(d));
                    }
                } else {
                    // If we have an intersection here, it should have a start that is the
                    // max of the two starts and an end that is the min of the two ends.
                    int64_t s = std::max(start(d), other.start(d));
                    int64_t l = std::min(end(d), other.end(d)) - s;
                    if (l > 0) {
                        // We have a real intersection
                        if (result) {
                            result->set_start(d, s);
                            result->set_length(d, l);
                        }
                    } else {
                        // We don't have an intersection for this dimension -- thus we don't
                        // have any intersection at all.
                        if (result) {
                            result->Clear();
                        }
                        return false;
                    }
                }
            }
            // If we are here, we know there is overlap in every dimension.
            return true;
        }

        bool TensorSlice::operator==(const TensorSlice& other) const
        {
            return dims() == other.dims() && starts_ == other.starts_ && lengths_ == other.lengths_;
        }

        void TensorSlice::ComputeRelative(const TensorSlice& sub, TensorSlice* relative) const
        {
            DCHECK_EQ(dims(), sub.dims());
            relative->SetFullSlice(dims());
            for (int d = 0; d < dims(); ++d) {
                if (IsFullAt(d)) {
                    relative->set_start(d, sub.start(d));
                    relative->set_length(d, sub.length(d));
                } else {
                    // Otherwise the relative start is the difference between the start of
                    // sub and the start of base
                    relative->set_start(d, sub.start(d) - start(d));
                    relative->set_length(d, sub.length(d));
                }
            }
        }

        void TensorSlice::UpdateToCover(const TensorSlice& other)
        {
            DCHECK_EQ(dims(), other.dims());
            for (int d = 0; d < dims(); ++d) {
                if (!IsFullAt(d)) {
                    if (other.IsFullAt(d)) {
                        starts_[d] = 0;
                        lengths_[d] = kFullExtent;
                    } else {
                        const auto new_end = std::max(end(d), other.end(d));
                        set_start(d, std::min(start(d), other.start(d)));
                        set_length(d, new_end - start(d));
                    }
                }
            }
        }

        // static
        bool TensorSlice::HasExtentLength(const TensorSliceProto::Extent& extent)
        {
            return extent.has_length();
        }

        // static
        int64_t TensorSlice::GetExtentLength(const TensorSliceProto::Extent& extent)
        {
            if (!HasExtentLength(extent)) {
                return -1;
            }
            return extent.length();
        }

        absl::Status
        TensorSlice::SliceTensorShape(const TensorShape& shape, TensorShape* result_shape) const
        {
            result_shape->Clear();
            // Mismatching ranks: we can't apply the slice at all.
            if (shape.dims() != dims()) {
                return absl::InternalError(
                    absl::StrCat(
                        "Mismatching ranks: shape = ", shape.DebugString(),
                        ", slice = ", DebugString()
                    )
                );
            }
            for (int d = 0; d < dims(); ++d) {
                if (IsFullAt(d)) {
                    result_shape->AddDim(shape.dim_size(d));
                } else {
                    // Check if the extent applies to the dimension
                    if (end(d) <= shape.dim_size(d)) {
                        // Yes: the end is within the range of the dim -- we adjust the result
                        // shape so that its size along this dimension is the length of the
                        // slice.
                        result_shape->AddDim(length(d));
                    } else {
                        // The extent doesn't apply to the dimension
                        result_shape->Clear();
                        return absl::InternalError(
                            absl::StrCat(
                                "Extent in dimension ", d, " out of bounds: shape = ",
                                shape.DebugString(), ", slice = ", DebugString()
                            )
                        );
                    }
                }
            }
            // If we are here, we have successfully applied the shape.
            return absl::OkStatus();
        }

        const int64_t TensorSlice::kFullExtent = -1;

    } // namespace tensorflow

} // export

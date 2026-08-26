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

#include "absl/strings/ascii.h"
#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/util/padding.h"
#include "unsupported/Eigen/CXX11/Tensor" // from @eigen_archive

#include <algorithm>
#include <array>
#include <cmath>

export module cc_tmp:ops_ops_util;

import std;
import cc_abi;

export {

    // This file contains utilities for various operations.


    namespace tensorflow {

        // Calculates broadcast starting index and size.  For SAME padding, addition
        // padding could be applied to right, left, top and bottom.  Depending on the
        // current index, input size, kernel size, stride, padding size, the starting
        // index and size for broadcast for that dimension are different from the
        // current index and kernel size.
        // This is mainly used by gradient algorithms for pooling operations.
        absl::Status GetBroadcastSize(
            const int index,
            const int in_size,
            const int ksize,
            const int stride,
            const int pad_size,
            int* bindex,
            int* bsize
        );

        // Converts Brain's Padding to Eigen's PaddingType.
        Eigen::PaddingType BrainPadding2EigenPadding(Padding padding);

        // Given a shape 's' of a tensor of type T. Returns true iff the
        // number of bytes occupied by each dim 0 (i.e., &tensor(i + 1, ...) -
        // &tensor(i, ...)) is multiple of EIGEN_MAX_ALIGN_BYTES.
        template<typename T>
        bool IsInnerDimsSizeAligned(const TensorShape& s)
        {
            if (s.dims() == 0) {
                return false;
            }
            const int64_t dim0_size = s.dim_size(0);
            if (dim0_size == 0) {
                return false;
            }
#if EIGEN_MAX_ALIGN_BYTES == 0
            return true;
#else
            const int64_t bytes_per_dim0 = (s.num_elements() / dim0_size) * sizeof(T);
            return bytes_per_dim0 % EIGEN_MAX_ALIGN_BYTES == 0;
#endif
        }

        // Given a shape 's' of a tensor of type T and the `start` and `end` index of a
        // dim 0 slice, returns true iff slice is aligned with respect to original
        // tensor. Here aligned implies the address is a multiple of
        // EIGEN_MAX_ALIGN_BYTES.
        template<typename T>
        bool IsDim0SliceAligned(const TensorShape& s, int64_t start, int64_t end_or_size)
        {
            if (s.dims() == 1) {
#if EIGEN_MAX_ALIGN_BYTES == 0
                return true;
#else
                bool start_aligned = (start * sizeof(T)) % EIGEN_MAX_ALIGN_BYTES == 0;
                // End is aligned if either the explicit end index is passed and is a
                // a multiple of EIGEN_MAX_ALIGN_BYTES, or the start index is aligned and
                // the size is aligned. So for convenience we can either pass start and
                // index, or start and size.
                bool end_aligned = (end_or_size * sizeof(T)) % EIGEN_MAX_ALIGN_BYTES == 0;
                return start_aligned && end_aligned;
#endif
            } else {
                return IsInnerDimsSizeAligned<T>(s);
            }
        }

        // Returns <suffix> sanitized to have only [a-zA-Z0-9-_].
        std::string SanitizeThreadSuffix(std::string suffix);

        // Helper to compute 'strides' given a tensor 'shape'. I.e.,
        // strides[i] = prod(shape.dim_size[(i+1):])
        template<typename T>
        gtl::InlinedVector<T, 8> ComputeStride(const TensorShape& shape)
        {
            const int ndims = shape.dims();
            gtl::InlinedVector<T, 8> strides(ndims);
            T stride = 1;
            for (int i = ndims - 1; i >= 0; --i) {
                strides[i] = stride;
                stride *= static_cast<T>(shape.dim_size(i));
            }
            return strides;
        }

        // Helper to compute 'strides' given an Eigen TensorDimensions
        template<typename T, typename EigenDimensions>
        gtl::InlinedVector<T, 8> ComputeEigenStrides(const EigenDimensions& shape)
        {
            const int ndims = shape.rank();
            gtl::InlinedVector<T, 8> strides(ndims);
            T stride = 1;
            for (int i = ndims - 1; i >= 0; --i) {
                strides[i] = stride;
                stride *= static_cast<T>(shape[i]);
            }
            return strides;
        }

    } // namespace tensorflow

    // ==================================================================
    // Implementation: ops_util.cc
    // ==================================================================

    namespace tensorflow {

        Eigen::PaddingType BrainPadding2EigenPadding(Padding padding)
        {
            switch (padding) {
                case Padding::VALID:
                    return Eigen::PADDING_VALID;
                case Padding::SAME:
                    return Eigen::PADDING_SAME;
                case Padding::EXPLICIT:
                    LOG(FATAL) << "Eigen does not have explicit padding enum " // Crash OK
                                  "value";
            }
            return Eigen::PADDING_SAME; // Prevent compiler warning about missing return
        }

        absl::Status GetBroadcastSize(
            const int index,
            const int in_size,
            const int ksize,
            const int stride,
            const int pad_size,
            int* bindex,
            int* bsize
        )
        {
            // Cannot have index beyond the input size.
            if (index * stride > in_size) {
                return absl::InvalidArgumentError(
                    "index * stride must be less than or equal to input size"
                );
            }
            *bindex = index * stride;
            *bsize = ksize;
            if (*bindex < pad_size) {
                // If the current index is in the padding area, start broadcast  from index
                // 0 with broadcast size reduced by padding size.
                *bsize = ksize + *bindex - pad_size;
                *bindex = 0;
            } else {
                // Otherwise, start broadcast from current index reduced by padding size.
                *bindex -= pad_size;
            }
            if (*bindex + ksize > in_size) {
                *bsize = std::min((in_size - *bindex), ksize);
            }
            return absl::OkStatus();
        }

        std::string SanitizeThreadSuffix(std::string suffix)
        {
            std::string clean;
            for (int i = 0; i < suffix.size(); ++i) {
                const char ch = suffix[i];
                if (absl::ascii_isalnum(ch) || ch == '_' || ch == '-') {
                    clean += ch;
                } else {
                    clean += '_';
                }
            }
            return clean;
        }

    } // namespace tensorflow

} // export

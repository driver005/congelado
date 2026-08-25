module;

/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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
#include "tensorflow/core/framework/function.h"
#include "tensorflow/core/lib/gtl/map_util.h"
#include "tensorflow/core/lib/random/random.h"
#include "tensorflow/core/lib/strings/stringprintf.h"

export module cc_tmp:graph_function_handle_cache;

import std;
import cc_abi;

export {

namespace tensorflow {

// Thread-safe data structure for caching function instantiations.
class FunctionHandleCache {
 public:
  explicit FunctionHandleCache(FunctionLibraryRuntime* lib);

  ~FunctionHandleCache();

  // Looks up the function to be instantiated in the cache first. If present,
  // returns handle from there. Otherwise, instantiates a new function
  // and stores handle in the cache.
  //
  // The cache retains the ownership of the handle. In particular, the caller
  // should not invoke `ReleaseHandle`.
  absl::Status Instantiate(const std::string& function_name, AttrSlice attrs,
                           FunctionLibraryRuntime::InstantiateOptions options,
                           FunctionLibraryRuntime::Handle* handle);

  // Releases all the handles in the cache, clearing out the state for all
  // functions involved.
  absl::Status Clear();

 private:
  mutex mu_;
  FunctionLibraryRuntime* lib_ = nullptr;  // not owned
  const std::string state_handle_;
  std::unordered_map<std::string, FunctionLibraryRuntime::Handle> handles_
      TF_GUARDED_BY(mu_);
};

}  // namespace tensorflow

// ==================================================================
// Implementation: function_handle_cache.cc
// ==================================================================

namespace tensorflow {

FunctionHandleCache::FunctionHandleCache(FunctionLibraryRuntime* lib)
    : lib_(lib),
      state_handle_(
          absl::StrFormat("%lld", static_cast<long long>(random::New64()))) {}

FunctionHandleCache::~FunctionHandleCache() {
  absl::Status s = Clear();
  if (!s.ok()) {
    LOG(ERROR) << "Failed to clear function handle cache: " << s.ToString();
  }
}

absl::Status FunctionHandleCache::Instantiate(
    const std::string& function_name, AttrSlice attrs,
    FunctionLibraryRuntime::InstantiateOptions options,
    FunctionLibraryRuntime::Handle* handle) {
  std::string key = Canonicalize(function_name, attrs, options);
  FunctionLibraryRuntime::Handle h;
  {
    tf_shared_lock l(mu_);
    h = gtl::FindWithDefault(handles_, key, kInvalidHandle);
  }
  if (h == kInvalidHandle) {
    options.state_handle = state_handle_;
    TF_RETURN_IF_ERROR(
        lib_->Instantiate(function_name, attrs, options, handle));
    mutex_lock l(mu_);
    handles_[key] = *handle;
  } else {
    *handle = h;
  }
  return absl::OkStatus();
}

absl::Status FunctionHandleCache::Clear() {
  mutex_lock l(mu_);
  for (const auto& entry : handles_) {
    TF_RETURN_IF_ERROR(lib_->ReleaseHandle(entry.second));
  }
  handles_.clear();
  return absl::OkStatus();
}

}  // namespace tensorflow

} // export

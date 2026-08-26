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

#include "tensorflow/core/framework/function.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/gtl/map_util.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/thread_annotations.h"
#include "tensorflow/core/platform/types.h"

#include <string>
#include <unordered_map>

export module cc_tmp:ops_op_segment;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // OpSegment keeps track of OpKernels registered for sessions running
        // on a device.
        //
        // The implementation maintains a two-level map. The 1st level maps
        // session handle to the map of registered OpKernels. The 2nd level
        // map maps node names to instantiated OpKernel objects.
        //
        // Each 2-nd level map is reference-counted and the caller can call
        // AddHold to obtain a reference on all kernels of a session and
        // ensure these kernels are alive until a corresponding RemoveHold is
        // called on the same session.
        class OpSegment
        {
        public:
            OpSegment();
            ~OpSegment();

            // A hold can be placed on a session, preventing all its kernels
            // from being deleted.
            void AddHold(const std::string& session_handle);
            void RemoveHold(const std::string& session_handle);

            // If the kernel for "node_name" has been created in the
            // "session_handle", returns the existing op kernel in "*kernel".
            // Otherwise, creates the kernel by calling create_fn(), cache it,
            // and returns it in "*kernel". If create_fn() fails, returns the
            // error.
            //
            // OpSegment keeps the ownership of the returned "*kernel".
            typedef std::function<absl::Status(OpKernel**)> CreateKernelFn;
            absl::Status FindOrCreate(
                const std::string& session_handle,
                const std::string& node_name,
                OpKernel** kernel,
                CreateKernelFn create_fn
            );

            // Returns true if OpSegment should own the kernel.
            static bool ShouldOwnKernel(FunctionLibraryRuntime* lib, const std::string& node_op);

        private:
            // op name -> OpKernel
            typedef std::unordered_map<std::string, OpKernel*> KernelMap;

            struct Item
            {
                int num_holds = 1;     // Num of holds put on the session.
                KernelMap name_kernel; // op name -> kernel.
                ~Item();
            };

            // session handle -> item.
            // Session handles are produced by strings::FpToString()
            typedef std::unordered_map<std::string, Item*> SessionMap;

            mutable mutex mu_;
            SessionMap sessions_ TF_GUARDED_BY(mu_);

            OpSegment(const OpSegment&) = delete;
            void operator=(const OpSegment&) = delete;
        };

    } // end namespace tensorflow

    // ==================================================================
    // Implementation: op_segment.cc
    // ==================================================================

    namespace tensorflow {

        OpSegment::Item::~Item()
        {
            for (const auto& kv: name_kernel) {
                delete kv.second;
            }
        }

        OpSegment::OpSegment() {}

        OpSegment::~OpSegment()
        {
            for (const auto& kv: sessions_) {
                delete kv.second;
            }
        }

        absl::Status OpSegment::FindOrCreate(
            const std::string& session_handle,
            const std::string& node_name,
            OpKernel** kernel,
            CreateKernelFn create_fn
        )
        {
            {
                mutex_lock l(mu_);
                auto item = gtl::FindPtrOrNull(sessions_, session_handle);
                if (item == nullptr) {
                    return absl::NotFoundError(
                        absl::StrCat("Session ", session_handle, " is not found.")
                    );
                }
                *kernel = gtl::FindPtrOrNull(item->name_kernel, node_name);
                if (*kernel != nullptr) {
                    return absl::OkStatus();
                }
            }
            absl::Status s = create_fn(kernel);
            if (!s.ok()) {
                LOG(ERROR) << "Create kernel failed: " << s;
                return s;
            }
            {
                mutex_lock l(mu_);
                auto item = gtl::FindPtrOrNull(sessions_, session_handle);
                if (item == nullptr) {
                    return absl::NotFoundError(
                        absl::StrCat("Session ", session_handle, " is not found.")
                    );
                }
                OpKernel** p_kernel = &(item->name_kernel[node_name]);
                if (*p_kernel == nullptr) {
                    *p_kernel = *kernel; // Inserts 'kernel' in the map.
                } else {
                    delete *kernel;
                    *kernel = *p_kernel;
                }
            }
            return absl::OkStatus();
        }

        void OpSegment::AddHold(const std::string& session_handle)
        {
            mutex_lock l(mu_);
            Item** item = &sessions_[session_handle];
            if (*item == nullptr) {
                *item = new Item; // num_holds == 1
            } else {
                ++((*item)->num_holds);
            }
        }

        void OpSegment::RemoveHold(const std::string& session_handle)
        {
            Item* item = nullptr;
            {
                mutex_lock l(mu_);
                auto siter = sessions_.find(session_handle);
                if (siter == sessions_.end()) {
                    VLOG(1) << "Session " << session_handle << " is not found.";
                    return;
                }
                item = siter->second;
                if (--(item->num_holds) > 0) {
                    return;
                } else {
                    sessions_.erase(siter);
                }
            }
            delete item;
        }

        bool OpSegment::ShouldOwnKernel(FunctionLibraryRuntime* lib, const std::string& node_op)
        {
            // OpSegment should not own kernel if the node is stateless, or a function.
            return lib->IsStateful(node_op) &&
                   lib->GetFunctionLibraryDefinition()->Find(node_op) == nullptr &&
                   node_op != "PartitionedCall" && node_op != "StatefulPartitionedCall";
        }

    } // end namespace tensorflow

} // export

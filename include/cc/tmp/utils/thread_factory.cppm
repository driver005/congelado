module;

/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/core/platform/types.h"

#include <functional>
#include <memory>

export module cc_tmp:utils_thread_factory;

import std;
import cc_abi;

export {

    namespace tsl {
        class Thread;
    } // namespace tsl

    namespace tensorflow {
        using tsl::Thread; // NOLINT

        // Virtual interface for an object that creates threads.
        class ThreadFactory
        {
        public:
            virtual ~ThreadFactory() {}

            // Runs `fn` asynchronously in a different thread. `fn` may block.
            //
            // NOTE: The caller is responsible for ensuring that this `ThreadFactory`
            // outlives the returned `Thread`.
            virtual std::unique_ptr<Thread>
            StartThread(const std::string& name, std::function<void()> fn) = 0;
        };

    } // namespace tensorflow

} // export

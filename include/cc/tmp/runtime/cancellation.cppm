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

#include "absl/memory/memory.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/notification.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/gtl/flatmap.h"
#include "tensorflow/core/lib/hash/hash.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/platform/stringpiece.h"
#include "tensorflow/core/platform/thread_annotations.h"
#include "tensorflow/core/platform/types.h"

#include <atomic>
#include <forward_list>
#include <functional>

export module cc_tmp:runtime_cancellation;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // A token that can be used to register and deregister a
        // CancelCallback with a CancellationManager.
        //
        // CancellationToken values must be created by a call to
        // CancellationManager::get_cancellation_token.
        typedef int64_t CancellationToken;

        // A callback that is invoked when a step is canceled.
        //
        // NOTE(mrry): See caveats about CancelCallback implementations in the
        // comment for CancellationManager::RegisterCallback.
        typedef std::function<void()> CancelCallback;

        // This class should never simultaneously be used as the cancellation manager
        // for two separate sets of executions (i.e two separate steps, or two separate
        // function executions).
        class CancellationManager
        {
        public:
            // A value that won't be returned by get_cancellation_token().
            static const CancellationToken kInvalidToken;

            CancellationManager();

            // Constructs a new CancellationManager that is a "child" of `*parent`.
            //
            // If `*parent` is cancelled, `*this` will be cancelled. `*parent` must
            // outlive the created CancellationManager.
            explicit CancellationManager(CancellationManager* parent);

            ~CancellationManager();

            // Run all callbacks associated with this manager.
            void StartCancel();

            // Run all callbacks associated with this manager with a status.
            // Currently the status is for logging purpose only. See also
            // CancellationManager::RegisterCallbackWithErrorLogging.
            void StartCancelWithStatus(const Status& status);

            // Returns true iff StartCancel() has been called.
            bool IsCancelled()
            {
                return is_cancelled_.load(std::memory_order_acquire);
            }

            // Returns a token that must be used in calls to RegisterCallback
            // and DeregisterCallback.
            CancellationToken get_cancellation_token()
            {
                return next_cancellation_token_.fetch_add(1);
            }

            // Attempts to register the given callback to be invoked when this
            // manager is cancelled. Returns true if the callback was
            // registered; returns false if this manager was already cancelled,
            // and the callback was not registered.
            //
            // If this method returns false, it is the caller's responsibility
            // to perform any cancellation cleanup.
            //
            // This method is tricky to use correctly. The following usage pattern
            // is recommended:
            //
            // class ObjectWithCancellableOperation {
            //   mutex mu_;
            //   void CancellableOperation(CancellationManager* cm,
            //                             std::function<void(Status)> callback) {
            //     bool already_cancelled;
            //     CancellationToken token = cm->get_cancellation_token();
            //     {
            //       mutex_lock(mu_);
            //       already_cancelled = !cm->RegisterCallback(
            //           [this, token]() { Cancel(token); });
            //       if (!already_cancelled) {
            //         // Issue asynchronous operation. Associate the pending operation
            //         // with `token` in some object state, or provide another way for
            //         // the Cancel method to look up the operation for cancellation.
            //         // Ensure that `cm->DeregisterCallback(token)` is called without
            //         // holding `mu_`, before `callback` is invoked.
            //         // ...
            //       }
            //     }
            //     if (already_cancelled) {
            //       callback(errors::Cancelled("Operation was cancelled"));
            //     }
            //   }
            //
            //   void Cancel(CancellationToken token) {
            //     mutex_lock(mu_);
            //     // Take action to cancel the operation with the given cancellation
            //     // token.
            //   }
            //
            // NOTE(mrry): The caller should take care that (i) the calling code
            // is robust to `callback` being invoked asynchronously (e.g. from
            // another thread), (ii) `callback` is deregistered by a call to
            // this->DeregisterCallback(token) when the operation completes
            // successfully, and (iii) `callback` does not invoke any method
            // on this cancellation manager. Furthermore, it is important that
            // the eventual caller of the complementary DeregisterCallback does not
            // hold any mutexes that are required by `callback`.
            bool RegisterCallback(CancellationToken token, CancelCallback callback);

            // Similar to RegisterCallback, but if the cancellation manager starts a
            // cancellation with an error status, it will log the error status before
            // invoking the callback. `callback_name` is a human-readable name of the
            // callback, which will be displayed on the log.
            bool RegisterCallbackWithErrorLogging(
                CancellationToken token,
                CancelCallback callback,
                tensorflow::StringPiece callback_name
            );

            // Deregister the callback that, when registered, was associated
            // with the given cancellation token. Returns true iff the callback
            // was deregistered and will not be invoked; otherwise returns false
            // after the callback has been invoked, blocking if necessary.
            //
            // NOTE(mrry): This method may block if cancellation is in progress.
            // The caller of this method must not hold any mutexes that are required
            // to invoke any cancellation callback that has been registered with this
            // cancellation manager.
            bool DeregisterCallback(CancellationToken token);

            // Deregister the callback that, when registered, was associated
            // with the given cancellation token. Returns true iff the callback
            // was deregistered and will not be invoked; otherwise returns false
            // immediately, with no guarantee that the callback has completed.
            //
            // This method is guaranteed to return true if StartCancel has not been
            // called.
            bool TryDeregisterCallback(CancellationToken token);

            // Returns true iff cancellation is in progress.
            bool IsCancelling();

        private:
            struct CallbackConfiguration
            {
                CancelCallback callback;
                std::string name;
                bool log_error = false;
            };

            struct State
            {
                Notification cancelled_notification;
                gtl::FlatMap<CancellationToken, CallbackConfiguration> callbacks;

                // If this CancellationManager has any children, this member points to the
                // head of a doubly-linked list of its children.
                CancellationManager* first_child = nullptr; // Not owned.
            };

            bool RegisterCallbackConfig(CancellationToken token, CallbackConfiguration config);

            bool RegisterChild(CancellationManager* child);
            void DeregisterChild(CancellationManager* child);

            bool is_cancelling_;
            std::atomic_bool is_cancelled_;
            std::atomic<CancellationToken> next_cancellation_token_;

            CancellationManager* const parent_ = nullptr; // Not owned.

            // If this CancellationManager is associated with a parent, this member will
            // be set to `true` after this is removed from the parent's list of children.
            bool is_removed_from_parent_ TF_GUARDED_BY(parent_->mu_) = false;

            // If this CancellationManager is associated with a parent, these members form
            // a doubly-linked list of that parent's children.
            //
            // These fields are valid only when `this->is_removed_from_parent_` is false.
            CancellationManager* prev_sibling_ TF_GUARDED_BY(parent_->mu_) = nullptr; // Not owned.
            CancellationManager* next_sibling_ TF_GUARDED_BY(parent_->mu_) = nullptr; // Not owned.

            mutex mu_;
            std::unique_ptr<State> state_ TF_GUARDED_BY(mu_);
        };

        // Registers the given cancellation callback, returning a function that can be
        // used to deregister the callback. If `cancellation_manager` is NULL, no
        // registration occurs and `deregister_fn` will be a no-op.
        Status RegisterCancellationCallback(
            CancellationManager* cancellation_manager,
            std::function<void()> callback,
            std::function<void()>* deregister_fn
        );

    } // namespace tensorflow

    // ==================================================================
    // Implementation: cancellation.cc
    // ==================================================================

    namespace tensorflow {

        const CancellationToken CancellationManager::kInvalidToken = -1;

        CancellationManager::CancellationManager() :
            is_cancelling_(false),
            is_cancelled_(false),
            next_cancellation_token_(0)
        {
        }

        CancellationManager::CancellationManager(CancellationManager* parent) :
            is_cancelling_(false),
            next_cancellation_token_(0),
            parent_(parent)
        {
            is_cancelled_ = parent->RegisterChild(this);
        }

        void CancellationManager::StartCancel()
        {
            // An "OK" status will not be logged by a callback registered by
            // RegisterCallbackWithErrorLogging.
            StartCancelWithStatus(OkStatus());
        }

        void CancellationManager::StartCancelWithStatus(const Status& status)
        {
            gtl::FlatMap<CancellationToken, CallbackConfiguration> callbacks_to_run;
            std::forward_list<CancellationManager*> children_to_cancel;
            Notification* cancelled_notification = nullptr;
            {
                mutex_lock l(mu_);
                if (is_cancelled_.load(std::memory_order_relaxed) || is_cancelling_) {
                    return;
                }
                is_cancelling_ = true;
                if (state_) {
                    std::swap(state_->callbacks, callbacks_to_run);

                    // Remove all children from the list of children.
                    CancellationManager* child = state_->first_child;
                    while (child != nullptr) {
                        children_to_cancel.push_front(child);
                        child->is_removed_from_parent_ = true;
                        child = child->next_sibling_;
                    }
                    state_->first_child = nullptr;

                    cancelled_notification = &state_->cancelled_notification;
                }
            }
            // We call these callbacks without holding mu_, so that concurrent
            // calls to DeregisterCallback, which can happen asynchronously, do
            // not block. The callbacks remain valid because any concurrent call
            // to DeregisterCallback will block until the
            // cancelled_notification_ is notified.
            for (auto key_and_value: callbacks_to_run) {
                CallbackConfiguration& config = key_and_value.second;
                if (!status.ok() && config.log_error) {
                    LOG(WARNING) << "Cancellation callback \"" << config.name
                                 << "\" is triggered due to a "
                                 << (StatusGroup::IsDerived(status) ? "derived" : "root")
                                 << " error: " << status.ToString();
                }
                config.callback();
            }
            for (CancellationManager* child: children_to_cancel) {
                child->StartCancelWithStatus(status);
            }
            {
                mutex_lock l(mu_);
                is_cancelling_ = false;
                is_cancelled_.store(true, std::memory_order_release);
            }
            if (cancelled_notification) {
                cancelled_notification->Notify();
            }
        }

        bool CancellationManager::RegisterCallback(CancellationToken token, CancelCallback callback)
        {
            return RegisterCallbackConfig(token, CallbackConfiguration{callback, "", false});
        }

        bool CancellationManager::RegisterCallbackWithErrorLogging(
            CancellationToken token, CancelCallback callback, tensorflow::StringPiece callback_name
        )
        {
            return RegisterCallbackConfig(
                token, CallbackConfiguration{callback, std::string(callback_name), true}
            );
        }

        bool CancellationManager::RegisterCallbackConfig(
            CancellationToken token, CallbackConfiguration config
        )
        {
            DCHECK_LT(token, next_cancellation_token_) << "Invalid cancellation token";
            mutex_lock l(mu_);
            bool should_register = !is_cancelled_ && !is_cancelling_;
            if (should_register) {
                if (!state_) {
                    state_ = absl::make_unique<State>();
                }
                std::swap(state_->callbacks[token], config);
            }
            return should_register;
        }

        bool CancellationManager::DeregisterCallback(CancellationToken token)
        {
            mu_.lock();
            if (is_cancelled_) {
                mu_.unlock();
                return false;
            } else if (is_cancelling_) {
                Notification* cancelled_notification =
                    state_ ? &state_->cancelled_notification : nullptr;
                mu_.unlock();
                // Wait for all of the cancellation callbacks to be called. This
                // wait ensures that the caller of DeregisterCallback does not
                // return immediately and free objects that may be used in the
                // execution of any currently pending callbacks in StartCancel.
                if (cancelled_notification) {
                    cancelled_notification->WaitForNotification();
                }
                return false;
            } else {
                if (state_) {
                    state_->callbacks.erase(token);
                }
                mu_.unlock();
                return true;
            }
        }

        bool CancellationManager::RegisterChild(CancellationManager* child)
        {
            mutex_lock l(mu_);
            if (is_cancelled_.load(std::memory_order_relaxed) || is_cancelling_) {
                child->is_removed_from_parent_ = true;
                return true;
            }

            if (!state_) {
                state_ = absl::make_unique<State>();
            }

            // Push `child` onto the front of the list of children.
            CancellationManager* current_head = state_->first_child;
            state_->first_child = child;
            child->prev_sibling_ = nullptr;
            child->next_sibling_ = current_head;
            if (current_head) {
                current_head->prev_sibling_ = child;
            }

            return false;
        }

        void CancellationManager::DeregisterChild(CancellationManager* child)
        {
            DCHECK_EQ(child->parent_, this);
            Notification* cancelled_notification = nullptr;
            {
                mutex_lock l(mu_);
                if (!child->is_removed_from_parent_) {
                    // Remove the child from this manager's list of children.
                    DCHECK(state_);

                    if (child->prev_sibling_ == nullptr) {
                        // The child was at the head of the list.
                        DCHECK_EQ(state_->first_child, child);
                        state_->first_child = child->next_sibling_;
                    } else {
                        child->prev_sibling_->next_sibling_ = child->next_sibling_;
                    }

                    if (child->next_sibling_ != nullptr) {
                        child->next_sibling_->prev_sibling_ = child->prev_sibling_;
                    }

                    child->is_removed_from_parent_ = true;
                }
                if (is_cancelling_) {
                    cancelled_notification = &state_->cancelled_notification;
                }
            }

            // Wait for an ongoing call to StartCancel() to finish. This wait ensures that
            // the caller of DeregisterChild does not return immediately and free a child
            // that may currently be being cancelled by StartCancel().
            if (cancelled_notification) {
                cancelled_notification->WaitForNotification();
            }
        }

        bool CancellationManager::TryDeregisterCallback(CancellationToken token)
        {
            mutex_lock lock(mu_);
            if (is_cancelled_ || is_cancelling_) {
                return false;
            } else {
                if (state_) {
                    state_->callbacks.erase(token);
                }
                return true;
            }
        }

        CancellationManager::~CancellationManager()
        {
            if (parent_) {
                parent_->DeregisterChild(this);
            }
            if (state_) {
                StartCancel();
            }
        }

        bool CancellationManager::IsCancelling()
        {
            mutex_lock lock(mu_);
            return is_cancelling_;
        }

        Status RegisterCancellationCallback(
            CancellationManager* cancellation_manager,
            CancelCallback callback,
            std::function<void()>* deregister_fn
        )
        {
            if (cancellation_manager) {
                CancellationToken token = cancellation_manager->get_cancellation_token();
                if (!cancellation_manager->RegisterCallback(token, std::move(callback))) {
                    return errors::Cancelled("Operation was cancelled");
                }
                *deregister_fn = [cancellation_manager, token]() {
                    cancellation_manager->DeregisterCallback(token);
                };
            } else {
                VLOG(1) << "Cancellation manager is not set. Cancellation callback will "
                           "not be registered.";
                *deregister_fn = []() {};
            }
            return OkStatus();
        }

    } // end namespace tensorflow

} // export

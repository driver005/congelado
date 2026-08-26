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

#include "tensorflow/core/framework/allocator.h"
#include "tensorflow/core/lib/gtl/inlined_vector.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/thread_annotations.h"
#include "tensorflow/core/platform/types.h"

#include <unordered_map>

export module cc_tmp:allocator_tracking_allocator;

import std;
import cc_abi;

export {

    namespace tensorflow {

        // TrackingAllocator is a wrapper for an Allocator. It keeps a running
        // count of the number of bytes allocated through the wrapper. It is
        // used by the Executor to "charge" allocations to particular Op
        // executions. Each Op gets a separate TrackingAllocator wrapper
        // around the underlying allocator.
        //
        // The implementation assumes the invariant that all calls to
        // AllocateRaw by an Op (or work items spawned by the Op) will occur
        // before the Op's Compute method returns. Thus the high watermark is
        // established once Compute returns.
        //
        // DeallocateRaw can be called long after the Op has finished,
        // e.g. when an output tensor is deallocated, and the wrapper cannot
        // be deleted until the last of these calls has occurred.  The
        // TrackingAllocator keeps track of outstanding calls using a
        // reference count, and deletes itself once the last call has been
        // received and the high watermark has been retrieved.
        struct AllocRecord
        {
            AllocRecord(int64_t a_btyes, int64_t a_micros) :
                alloc_bytes(a_btyes),
                alloc_micros(a_micros)
            {
            }

            AllocRecord() :
                AllocRecord(0, 0)
            {
            }

            int64_t alloc_bytes;
            int64_t alloc_micros;
        };

        class TrackingAllocator : public Allocator
        {
        public:
            explicit TrackingAllocator(Allocator* allocator, bool track_ids);

            std::string Name() override
            {
                return allocator_->Name();
            }

            void* AllocateRaw(size_t alignment, size_t num_bytes) override
            {
                return AllocateRaw(alignment, num_bytes, AllocationAttributes());
            }

            void* AllocateRaw(
                size_t alignment, size_t num_bytes, const AllocationAttributes& allocation_attr
            ) override;
            void DeallocateRaw(void* ptr) override;
            bool TracksAllocationSizes() const override;
            size_t RequestedSize(const void* ptr) const override;
            size_t AllocatedSize(const void* ptr) const override;
            int64_t AllocationId(const void* ptr) const override;
            absl::optional<AllocatorStats> GetStats() override;
            bool ClearStats() override;

            AllocatorMemoryType GetMemoryType() const override
            {
                return allocator_->GetMemoryType();
            }

            // If the underlying allocator tracks allocation sizes, this returns
            // a tuple where the first value is the total number of bytes
            // allocated through this wrapper, the second value is the high
            // watermark of bytes allocated through this wrapper and the third value is
            // the allocated bytes through this wrapper that are still alive. If the
            // underlying allocator does not track allocation sizes the first
            // value is the total number of bytes requested through this wrapper
            // and the second and the third are 0.
            //
            std::tuple<size_t, size_t, size_t> GetSizes();
            // After GetRecordsAndUnRef is called, the only further calls allowed
            // on this wrapper are calls to DeallocateRaw with pointers that
            // were allocated by this wrapper and have not yet been
            // deallocated. After this call completes and all allocated pointers
            // have been deallocated the wrapper will delete itself.
            gtl::InlinedVector<AllocRecord, 4> GetRecordsAndUnRef();
            // Returns a copy of allocation records collected so far.
            gtl::InlinedVector<AllocRecord, 4> GetCurrentRecords();

        protected:
            ~TrackingAllocator() override {}

        private:
            bool UnRef() TF_EXCLUSIVE_LOCKS_REQUIRED(mu_);

            Allocator* allocator_; // not owned.
            mutable mutex mu_;
            // the number of calls to AllocateRaw that have not yet been matched
            // by a corresponding call to DeAllocateRaw, plus 1 if the Executor
            // has not yet read out the high watermark.
            int ref_ TF_GUARDED_BY(mu_);
            // the current number of outstanding bytes that have been allocated
            // by this wrapper, or 0 if the underlying allocator does not track
            // allocation sizes.
            size_t allocated_ TF_GUARDED_BY(mu_);
            // the maximum number of outstanding bytes that have been allocated
            // by this wrapper, or 0 if the underlying allocator does not track
            // allocation sizes.
            size_t high_watermark_ TF_GUARDED_BY(mu_);
            // the total number of bytes that have been allocated by this
            // wrapper if the underlying allocator tracks allocation sizes,
            // otherwise the total number of bytes that have been requested by
            // this allocator.
            size_t total_bytes_ TF_GUARDED_BY(mu_);

            gtl::InlinedVector<AllocRecord, 4> allocations_ TF_GUARDED_BY(mu_);

            // Track allocations locally if requested in the constructor and the
            // underlying allocator doesn't already do it for us.
            const bool track_sizes_locally_;

            struct Chunk
            {
                size_t requested_size;
                size_t allocated_size;
                int64_t allocation_id;
            };

            std::unordered_map<const void*, Chunk> in_use_ TF_GUARDED_BY(mu_);
            int64_t next_allocation_id_ TF_GUARDED_BY(mu_);
        };

    } // end namespace tensorflow

    // ==================================================================
    // Implementation: tracking_allocator.cc
    // ==================================================================

    namespace tensorflow {

        TrackingAllocator::TrackingAllocator(Allocator* allocator, bool track_sizes) :
            allocator_(allocator),
            ref_(1),
            allocated_(0),
            high_watermark_(0),
            total_bytes_(0),
            track_sizes_locally_(track_sizes && !allocator_->TracksAllocationSizes()),
            next_allocation_id_(0)
        {
        }

        void* TrackingAllocator::AllocateRaw(
            size_t alignment, size_t num_bytes, const AllocationAttributes& allocation_attr
        )
        {
            void* ptr = allocator_->AllocateRaw(alignment, num_bytes, allocation_attr);
            // If memory is exhausted AllocateRaw returns nullptr, and we should
            // pass this through to the caller
            if (nullptr == ptr) {
                return ptr;
            }
            if (allocator_->TracksAllocationSizes()) {
                size_t allocated_bytes = allocator_->AllocatedSize(ptr);
                {
                    mutex_lock lock(mu_);
                    allocated_ += allocated_bytes;
                    high_watermark_ = std::max(high_watermark_, allocated_);
                    total_bytes_ += allocated_bytes;
                    allocations_.emplace_back(allocated_bytes, Env::Default()->NowMicros());
                    ++ref_;
                }
            } else if (track_sizes_locally_) {
                // Call the underlying allocator to try to get the allocated size
                // whenever possible, even when it might be slow. If this fails,
                // use the requested size as an approximation.
                size_t allocated_bytes = allocator_->AllocatedSizeSlow(ptr);
                allocated_bytes = std::max(num_bytes, allocated_bytes);
                mutex_lock lock(mu_);
                next_allocation_id_ += 1;
                Chunk chunk = {num_bytes, allocated_bytes, next_allocation_id_};
                in_use_.emplace(std::make_pair(ptr, chunk));
                allocated_ += allocated_bytes;
                high_watermark_ = std::max(high_watermark_, allocated_);
                total_bytes_ += allocated_bytes;
                allocations_.emplace_back(allocated_bytes, Env::Default()->NowMicros());
                ++ref_;
            } else {
                mutex_lock lock(mu_);
                total_bytes_ += num_bytes;
                allocations_.emplace_back(num_bytes, Env::Default()->NowMicros());
                ++ref_;
            }
            return ptr;
        }

        void TrackingAllocator::DeallocateRaw(void* ptr)
        {
            // freeing a null ptr is a no-op
            if (nullptr == ptr) {
                return;
            }
            bool should_delete;
            // fetch the following outside the lock in case the call to
            // AllocatedSize is slow
            bool tracks_allocation_sizes = allocator_->TracksAllocationSizes();
            size_t allocated_bytes = 0;
            if (tracks_allocation_sizes) {
                allocated_bytes = allocator_->AllocatedSize(ptr);
            } else if (track_sizes_locally_) {
                mutex_lock lock(mu_);
                auto itr = in_use_.find(ptr);
                if (itr != in_use_.end()) {
                    tracks_allocation_sizes = true;
                    allocated_bytes = (*itr).second.allocated_size;
                    in_use_.erase(itr);
                }
            }
            Allocator* allocator = allocator_;
            {
                mutex_lock lock(mu_);
                if (tracks_allocation_sizes) {
                    CHECK_GE(allocated_, allocated_bytes);
                    allocated_ -= allocated_bytes;
                    allocations_.emplace_back(-allocated_bytes, Env::Default()->NowMicros());
                }
                should_delete = UnRef();
            }
            allocator->DeallocateRaw(ptr);
            if (should_delete) {
                delete this;
            }
        }

        bool TrackingAllocator::TracksAllocationSizes() const
        {
            return track_sizes_locally_ || allocator_->TracksAllocationSizes();
        }

        size_t TrackingAllocator::RequestedSize(const void* ptr) const
        {
            if (track_sizes_locally_) {
                mutex_lock lock(mu_);
                auto it = in_use_.find(ptr);
                if (it != in_use_.end()) {
                    return (*it).second.requested_size;
                }
                return 0;
            } else {
                return allocator_->RequestedSize(ptr);
            }
        }

        size_t TrackingAllocator::AllocatedSize(const void* ptr) const
        {
            if (track_sizes_locally_) {
                mutex_lock lock(mu_);
                auto it = in_use_.find(ptr);
                if (it != in_use_.end()) {
                    return (*it).second.allocated_size;
                }
                return 0;
            } else {
                return allocator_->AllocatedSize(ptr);
            }
        }

        int64_t TrackingAllocator::AllocationId(const void* ptr) const
        {
            if (track_sizes_locally_) {
                mutex_lock lock(mu_);
                auto it = in_use_.find(ptr);
                if (it != in_use_.end()) {
                    return (*it).second.allocation_id;
                }
                return 0;
            } else {
                return allocator_->AllocationId(ptr);
            }
        }

        absl::optional<AllocatorStats> TrackingAllocator::GetStats()
        {
            return allocator_->GetStats();
        }

        bool TrackingAllocator::ClearStats()
        {
            return allocator_->ClearStats();
        }

        std::tuple<size_t, size_t, size_t> TrackingAllocator::GetSizes()
        {
            size_t high_watermark;
            size_t total_bytes;
            size_t still_live_bytes;
            {
                mutex_lock lock(mu_);
                high_watermark = high_watermark_;
                total_bytes = total_bytes_;
                still_live_bytes = allocated_;
            }
            return std::make_tuple(total_bytes, high_watermark, still_live_bytes);
        }

        gtl::InlinedVector<AllocRecord, 4> TrackingAllocator::GetRecordsAndUnRef()
        {
            bool should_delete;
            gtl::InlinedVector<AllocRecord, 4> allocations;
            {
                mutex_lock lock(mu_);
                allocations.swap(allocations_);
                should_delete = UnRef();
            }
            if (should_delete) {
                delete this;
            }
            return allocations;
        }

        gtl::InlinedVector<AllocRecord, 4> TrackingAllocator::GetCurrentRecords()
        {
            gtl::InlinedVector<AllocRecord, 4> allocations;
            {
                mutex_lock lock(mu_);
                for (const AllocRecord& alloc: allocations_) {
                    allocations.push_back(alloc);
                }
            }
            return allocations;
        }

        bool TrackingAllocator::UnRef()
        {
            CHECK_GE(ref_, 1);
            --ref_;
            return (ref_ == 0);
        }

    } // end namespace tensorflow

} // export

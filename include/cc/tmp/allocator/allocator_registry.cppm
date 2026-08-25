module;

/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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
#include <vector>
#include "tensorflow/core/framework/allocator.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/numa.h"
#include "tensorflow/core/platform/logging.h"

export module cc_tmp:allocator_allocator_registry;

import std;
import cc_abi;

export {

// Classes to maintain a static registry of memory allocator factories.
#ifndef TENSORFLOW_CORE_FRAMEWORK_ALLOCATOR_REGISTRY_H_
#define TENSORFLOW_CORE_FRAMEWORK_ALLOCATOR_REGISTRY_H_



namespace tensorflow {

class AllocatorFactory {
 public:
  virtual ~AllocatorFactory() {}

  // Returns true if the factory will create a functionally different
  // SubAllocator for different (legal) values of numa_node.
  virtual bool NumaEnabled() { return false; }

  // Create an Allocator.
  virtual Allocator* CreateAllocator() = 0;

  // Create a SubAllocator. If NumaEnabled() is true, then returned SubAllocator
  // will allocate memory local to numa_node.  If numa_node == kNUMANoAffinity
  // then allocated memory is not specific to any NUMA node.
  virtual SubAllocator* CreateSubAllocator(int numa_node) = 0;
};

// ProcessState is defined in a package that cannot be a dependency of
// framework.  This definition allows us to access the one method we need.
class ProcessStateInterface {
 public:
  virtual ~ProcessStateInterface() {}
  virtual Allocator* GetCPUAllocator(int numa_node) = 0;
};

// A singleton registry of AllocatorFactories.
//
// Allocators should be obtained through ProcessState or cpu_allocator()
// (deprecated), not directly through this interface.  The purpose of this
// registry is to allow link-time discovery of multiple AllocatorFactories among
// which ProcessState will obtain the best fit at startup.
class AllocatorFactoryRegistry {
 public:
  AllocatorFactoryRegistry() {}
  ~AllocatorFactoryRegistry() {}

  void Register(const char* source_file, int source_line, const string& name,
                int priority, AllocatorFactory* factory);

  // Returns 'best fit' Allocator.  Find the factory with the highest priority
  // and return an allocator constructed by it.  If multiple factories have
  // been registered with the same priority, picks one by unspecified criteria.
  Allocator* GetAllocator();

  // Returns 'best fit' SubAllocator.  First look for the highest priority
  // factory that is NUMA-enabled.  If none is registered, fall back to the
  // highest priority non-NUMA-enabled factory.  If NUMA-enabled, return a
  // SubAllocator specific to numa_node, otherwise return a NUMA-insensitive
  // SubAllocator.
  SubAllocator* GetSubAllocator(int numa_node);

  // Returns the singleton value.
  static AllocatorFactoryRegistry* singleton();

  ProcessStateInterface* process_state() const { return process_state_; }

 protected:
  friend class ProcessState;
  ProcessStateInterface* process_state_ = nullptr;

 private:
  mutex mu_;
  bool first_alloc_made_ = false;
  struct FactoryEntry {
    const char* source_file;
    int source_line;
    string name;
    int priority;
    std::unique_ptr<AllocatorFactory> factory;
    std::unique_ptr<Allocator> allocator;
    // Index 0 corresponds to kNUMANoAffinity, other indices are (numa_node +
    // 1).
    std::vector<std::unique_ptr<SubAllocator>> sub_allocators;
  };
  std::vector<FactoryEntry> factories_ TF_GUARDED_BY(mu_);

  // Returns any FactoryEntry registered under 'name' and 'priority',
  // or 'nullptr' if none found.
  const FactoryEntry* FindEntry(const string& name, int priority) const
      TF_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  TF_DISALLOW_COPY_AND_ASSIGN(AllocatorFactoryRegistry);
};

class AllocatorFactoryRegistration {
 public:
  AllocatorFactoryRegistration(const char* file, int line, const string& name,
                               int priority, AllocatorFactory* factory) {
    AllocatorFactoryRegistry::singleton()->Register(file, line, name, priority,
                                                    factory);
  }
};

#define REGISTER_MEM_ALLOCATOR(name, priority, factory)                     \
  REGISTER_MEM_ALLOCATOR_UNIQ_HELPER(__COUNTER__, __FILE__, __LINE__, name, \
                                     priority, factory)

#define REGISTER_MEM_ALLOCATOR_UNIQ_HELPER(ctr, file, line, name, priority, \
                                           factory)                         \
  REGISTER_MEM_ALLOCATOR_UNIQ(ctr, file, line, name, priority, factory)

#define REGISTER_MEM_ALLOCATOR_UNIQ(ctr, file, line, name, priority, factory) \
  static AllocatorFactoryRegistration allocator_factory_reg_##ctr(            \
      file, line, name, priority, new factory)

}  // namespace tensorflow

#endif  // TENSORFLOW_CORE_FRAMEWORK_ALLOCATOR_REGISTRY_H_

// ==================================================================
// Implementation: allocator_registry.cc
// ==================================================================

namespace tensorflow {

// static
AllocatorFactoryRegistry* AllocatorFactoryRegistry::singleton() {
  static AllocatorFactoryRegistry* singleton = new AllocatorFactoryRegistry;
  return singleton;
}

const AllocatorFactoryRegistry::FactoryEntry*
AllocatorFactoryRegistry::FindEntry(const string& name, int priority) const {
  for (auto& entry : factories_) {
    if (!name.compare(entry.name) && priority == entry.priority) {
      return &entry;
    }
  }
  return nullptr;
}

void AllocatorFactoryRegistry::Register(const char* source_file,
                                        int source_line, const string& name,
                                        int priority,
                                        AllocatorFactory* factory) {
  mutex_lock l(mu_);
  CHECK(!first_alloc_made_) << "Attempt to register an AllocatorFactory "
                            << "after call to GetAllocator()";
  CHECK(!name.empty()) << "Need a valid name for Allocator";
  CHECK_GE(priority, 0) << "Priority needs to be non-negative";

  const FactoryEntry* existing = FindEntry(name, priority);
  if (existing != nullptr) {
    // Duplicate registration is a hard failure.
    LOG(FATAL) << "New registration for AllocatorFactory with name=" << name
               << " priority=" << priority << " at location " << source_file
               << ":" << source_line
               << " conflicts with previous registration at location "
               << existing->source_file << ":" << existing->source_line;
  }

  FactoryEntry entry;
  entry.source_file = source_file;
  entry.source_line = source_line;
  entry.name = name;
  entry.priority = priority;
  entry.factory.reset(factory);
  factories_.push_back(std::move(entry));
}

Allocator* AllocatorFactoryRegistry::GetAllocator() {
  mutex_lock l(mu_);
  first_alloc_made_ = true;
  FactoryEntry* best_entry = nullptr;
  for (auto& entry : factories_) {
    if (best_entry == nullptr) {
      best_entry = &entry;
    } else if (entry.priority > best_entry->priority) {
      best_entry = &entry;
    }
  }
  if (best_entry) {
    if (!best_entry->allocator) {
      best_entry->allocator.reset(best_entry->factory->CreateAllocator());
    }
    return best_entry->allocator.get();
  } else {
    LOG(FATAL) << "No registered CPU AllocatorFactory";
    return nullptr;
  }
}

SubAllocator* AllocatorFactoryRegistry::GetSubAllocator(int numa_node) {
  mutex_lock l(mu_);
  first_alloc_made_ = true;
  FactoryEntry* best_entry = nullptr;
  for (auto& entry : factories_) {
    if (best_entry == nullptr) {
      best_entry = &entry;
    } else if (best_entry->factory->NumaEnabled()) {
      if (entry.factory->NumaEnabled() &&
          (entry.priority > best_entry->priority)) {
        best_entry = &entry;
      }
    } else {
      DCHECK(!best_entry->factory->NumaEnabled());
      if (entry.factory->NumaEnabled() ||
          (entry.priority > best_entry->priority)) {
        best_entry = &entry;
      }
    }
  }
  if (best_entry) {
    int index = 0;
    if (numa_node != port::kNUMANoAffinity) {
      CHECK_LE(numa_node, port::NUMANumNodes());
      index = 1 + numa_node;
    }
    if (best_entry->sub_allocators.size() < static_cast<size_t>(index + 1)) {
      best_entry->sub_allocators.resize(index + 1);
    }
    if (!best_entry->sub_allocators[index].get()) {
      best_entry->sub_allocators[index].reset(
          best_entry->factory->CreateSubAllocator(numa_node));
    }
    return best_entry->sub_allocators[index].get();
  } else {
    LOG(FATAL) << "No registered CPU AllocatorFactory";
    return nullptr;
  }
}

}  // namespace tensorflow

} // export

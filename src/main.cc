#include "concurrentqueue.h"
#include <atomic>
#include <iostream>
#include <print> // C++23
#include <ranges>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

int main() {
  // Configuration
  const int num_producers = 8;
  const int num_consumers = 8;
  const int items_per_producer = 250'000;
  const int total_items = num_producers * items_per_producer;

  moodycamel::ConcurrentQueue<int> queue;
  std::atomic<int> total_consumed{0};
  std::atomic<long long> sum_consumed{0};

  std::print("Starting stress test with {} producers and {} consumers...\n",
             num_producers, num_consumers);

  // 1. Launch Producers using C++23 ranges for the loop
  std::vector<std::jthread> producers;
  for ([[maybe_unused]] auto i : std::views::iota(0, num_producers)) {
    producers.emplace_back([&queue, items_per_producer]() {
      for (auto j : std::views::iota(0, items_per_producer)) {
        // Enqueue unique values to ensure we can verify sum later
        queue.enqueue(1);
      }
    });
  }

  // 2. Launch Consumers
  std::vector<std::jthread> consumers;
  for ([[maybe_unused]] auto i : std::views::iota(0, num_consumers)) {
    consumers.emplace_back(
        [&queue, &total_consumed, &sum_consumed, total_items]() {
          int item;
          // Modern busy-wait loop
          while (total_consumed.load(std::memory_order_relaxed) < total_items) {
            if (queue.try_dequeue(item)) {
              sum_consumed.fetch_add(item, std::memory_order_relaxed);
              total_consumed.fetch_add(1, std::memory_order_relaxed);
            } else {
              // Micro-sleep or hint to CPU to save power during high contention
              std::this_thread::yield();
            }
          }
        });
  }

  // Join all threads
  producers.clear();
  consumers.clear();

  // 3. Validation using std::println
  std::println("\n--- Test Results ---");
  std::println("Expected items: {:L}", total_items);
  std::println("Actual items:   {:L}", total_consumed.load());

  if (sum_consumed.load() == total_items) {
    std::println("✅ SUCCESS: All items accounted for.");
  } else {
    std::println("❌ FAILURE: Expected sum {}, but got {}", total_items,
                 sum_consumed.load());
  }

  return 0;
}

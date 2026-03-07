#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

import std;
import atomic_list;
import node;

struct TestNode : Node {
  int value;
  explicit TestNode(int v) : value(v) {}
};

TEST_CASE("Benchmark: Single-threaded Latency") {
  AtomicList pool;
  auto *n = new TestNode(42);

  BENCHMARK("add + try_get (No Contention)") {
    pool.add(n);
    Node *result = pool.try_get();
    return result;
  };

  delete n;
}

TEST_CASE("Benchmark: Two-threaded Handshake") {
  AtomicList pool;
  auto *n = new TestNode(1);
  std::atomic<bool> stop{false};

  // We use a worker to immediately put back anything it gets
  std::thread worker([&] {
    while (!stop.load(std::memory_order_acquire)) {
      if (Node *popped = pool.try_get()) {
        pool.add(popped);
      }
    }
  });

  BENCHMARK("Producer-Consumer Ping-Pong") {
    pool.add(n);
    Node *popped = nullptr;
    // Wait for the worker to put it back
    while (!(popped = pool.try_get())) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    return popped;
  };

  stop.store(true, std::memory_order_release);
  worker.join();
  delete n;
}

TEST_CASE("Benchmark: Multi-threaded Contention Storm") {
  AtomicList pool;
  constexpr int N_THREADS = 4;
  constexpr int NODES_PER_THREAD = 100;

  std::vector<TestNode *> all_nodes;
  for (int i = 0; i < N_THREADS * NODES_PER_THREAD; ++i) {
    auto *n = new TestNode(i);
    all_nodes.push_back(n);
    pool.add(n);
  }

  BENCHMARK("4-Thread Burst Throughput") {
    std::vector<std::thread> threads;
    for (int t = 0; t < N_THREADS; ++t) {
      threads.emplace_back([&pool] {
        for (int i = 0; i < 500; ++i) {
          if (Node *n = pool.try_get()) {
            pool.add(n);
          }
        }
      });
    }
    for (auto &th : threads)
      th.join();
    return pool.try_get();
  };

  // Cleanup
  Node *drain;
  while ((drain = pool.try_get()))
    ;
  for (auto *n : all_nodes)
    delete n;
}

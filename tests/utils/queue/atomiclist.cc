#include <catch2/catch_test_macros.hpp>

import std;
import atomic_list;
import node;

struct TestNode : Node {
  int value;
  explicit TestNode(int v) : value(v) {}
};

// Universe helper to manage raw memory since we aren't using smart pointers for speed
struct NodeUniverse {
  std::vector<TestNode *> nodes;
  NodeUniverse(int count) {
    for (int i = 0; i < count; ++i)
      nodes.push_back(new TestNode(i));
  }
  ~NodeUniverse() {
    for (auto *n : nodes)
      delete n;
  }
};

TEST_CASE("AtomicList: Basic Add/Get") {
  AtomicList pool;
  TestNode n1(1);

  pool.add(&n1);
  Node *popped = pool.try_get();

  REQUIRE(popped == &n1);
  REQUIRE(pool.try_get() == nullptr);
}

TEST_CASE("AtomicList: LIFO (Stack) Behavior") {
  AtomicList pool;
  TestNode n1(1), n2(2), n3(3);

  pool.add(&n1);
  pool.add(&n2);
  pool.add(&n3);

  // Should pop in reverse order
  REQUIRE(pool.try_get() == &n3);
  REQUIRE(pool.try_get() == &n2);
  REQUIRE(pool.try_get() == &n1);
}

TEST_CASE("AtomicList: Concurrent Contention") {
  constexpr int N_THREADS = 8;
  constexpr int N_NODES = 1000;
  AtomicList pool;
  NodeUniverse u(N_NODES);

  // Pre-fill pool
  for (auto *n : u.nodes)
    pool.add(n);

  auto worker = [&]() {
    for (int i = 0; i < 10000; ++i) {
      Node *n = pool.try_get();
      if (n) {
        // Do fake work
        std::this_thread::yield();
        pool.add(n);
      }
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < N_THREADS; ++i)
    threads.emplace_back(worker);
  for (auto &t : threads)
    t.join();

  // Verify all nodes are still in the pool and none were lost
  int count = 0;
  while (pool.try_get())
    count++;
  REQUIRE(count == N_NODES);
}

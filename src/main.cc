#include <cassert>
#include <cstdlib>
#include <iostream>
#include <print>

import page;
import atomic_list;

#define BLOCK_SIZE 32

template <typename TAlign> [[nodiscard]] static void *aligned_malloc(std::size_t size) {
  constexpr std::size_t alignment = alignof(TAlign);
  constexpr bool needs_manual_align = alignment > alignof(std::max_align_t);

  if constexpr (needs_manual_align) {
    std::println("Alignment needed for type with alignment {}.", alignof(TAlign));
    // Round size up to a multiple of alignment (required by std::aligned_alloc)
    std::size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    return std::aligned_alloc(alignment, aligned_size);
  } else {
    std::println("No special alignment needed for type with alignment {}.", alignof(TAlign));
    return std::malloc(size);
  }
}

template <typename TAlign> static void aligned_free(void *ptr) {
  std::free(ptr); // std::aligned_alloc memory is freed with std::free directly
}

template <typename U> [[nodiscard]] static U *create() {
  void *p = aligned_malloc<U>(sizeof(U));
  return p ? new (p) U : nullptr;
}

template <typename U, typename... Args> [[nodiscard]] static U *create(Args &&...args) {
  void *p = aligned_malloc<U>(sizeof(U));
  return p ? new (p) U(std::forward<Args>(args)...) : nullptr;
}

template <typename U> static void destroy(U *ptr) {
  if (!ptr)
    return;
  ptr->~U();
  aligned_free<U>(ptr);
}
struct AlignedNode : Node {
  int value{0};
  explicit AlignedNode(int v) : value(v) {}
};

struct TestNode : Node {
  int value{0};
  explicit TestNode(int v) : value(v) {}
};

int main() {
  Page<TestNode> page_stack;
  Page<TestNode> *page = create<Page<TestNode>>();

  std::println("Testing page:");
  std::println("Size of page: {}", sizeof(page_stack));
  std::println("Heap sizeof page: {}", sizeof(*page));

  std::println("\nTesting over-aligned allocation (alignas(32)):");
  AlignedNode *an = create<AlignedNode>(42);
  std::println("address:     {:#x}", reinterpret_cast<std::uintptr_t>(an));
  std::println("alignment:   {}", alignof(AlignedNode));
  std::println("is aligned:  {}", reinterpret_cast<std::uintptr_t>(an) % alignof(AlignedNode) == 0);
  std::println("value:       {}", an->value);
  destroy(an);

  aligned_free<Page<TestNode>>(page);

  AtomicList pool;

  auto seed = [&](TestNode *node) { pool.add(node); };
  seed(new TestNode(1));
  seed(new TestNode(2));
  seed(new TestNode(3));

  TestNode *node = static_cast<TestNode *>(pool.try_get());
  assert(node != nullptr);
  std::cout << "Got node value: " << node->value << "\n";

  node = static_cast<TestNode *>(pool.try_get());
  assert(node != nullptr);
  std::cout << "Got node value: " << node->value << "\n";

  node = static_cast<TestNode *>(pool.try_get());
  assert(node != nullptr);
  std::cout << "Got node value: " << node->value << "\n";

  node = static_cast<TestNode *>(pool.try_get());
  assert(node == nullptr);
  std::cout << "Pool empty: ok\n";

  return 0;
}

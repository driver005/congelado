// #include <catch2/catch_test_macros.hpp>
//
// import std;
// import atomic_list;
// import pager;
// import page;
// import consts;
//
// static AtomicList make_recycle_list() { return AtomicList{}; }
//
// // A type that tracks live instances so we can verify destructor calls.
// struct Tracked {
//   static std::atomic<int> live;
//   int value;
//   explicit Tracked(int v = 0) : value(v) { live.fetch_add(1); }
//   Tracked(const Tracked &o) : value(o.value) { live.fetch_add(1); }
//   Tracked(Tracked &&o) noexcept : value(o.value) { live.fetch_add(1); }
//   Tracked &operator=(const Tracked &o) {
//     value = o.value;
//     return *this;
//   }
//   Tracked &operator=(Tracked &&o) noexcept {
//     value = o.value;
//     return *this;
//   }
//   ~Tracked() { live.fetch_sub(1); }
// };
// std::atomic<int> Tracked::live{0};
//
// TEST_CASE("enqueue then dequeue returns the same value", "[pager][enqueue][dequeue]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   REQUIRE(p.enqueue<AllocationMode::CanAlloc>(42));
//
//   int out = 0;
//   REQUIRE(p.dequeue<AllocationMode::CanAlloc>(out));
//   CHECK(out == 42);
// }
//
// TEST_CASE("dequeue on empty pager returns false", "[pager][dequeue]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   int out = 0;
//   CHECK_FALSE(p.dequeue<AllocationMode::CanAlloc>(out));
// }
//
// TEST_CASE("enqueue preserves FIFO order", "[pager][enqueue][dequeue]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   for (int i = 0; i < 10; ++i)
//     REQUIRE(p.enqueue<AllocationMode::CanAlloc>(i));
//
//   for (int i = 0; i < 10; ++i) {
//     int out = -1;
//     REQUIRE(p.dequeue<AllocationMode::CanAlloc>(out));
//     CHECK(out == i);
//   }
// }
//
// TEST_CASE("enqueue across a page boundary (> BLOCK_SIZE elements)", "[pager][enqueue][dequeue]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const int N = BLOCK_SIZE * 3;
//   for (int i = 0; i < N; ++i)
//     REQUIRE(p.enqueue<AllocationMode::CanAlloc>(i));
//
//   for (int i = 0; i < N; ++i) {
//     int out = -1;
//     REQUIRE(p.dequeue<AllocationMode::CanAlloc>(out));
//     CHECK(out == i);
//   }
// }
//
// TEST_CASE("enqueue in AllocationMode::CannotAlloc mode fails when no recycled pages available",
//           "[pager][enqueue][AllocationMode::CannotAlloc]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   // Fill the first page completely.
//   for (std::size_t i = 0; i < BLOCK_SIZE; ++i)
//     REQUIRE(p.enqueue<AllocationMode::CannotAlloc>(i));
//
//   // Next enqueue needs a new page but cannot allocate.
//   CHECK_FALSE(p.enqueue<AllocationMode::CannotAlloc>(99));
// }
//
// TEST_CASE("size reflects enqueued minus dequeued", "[pager][size]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   CHECK(p.size() == 0);
//
//   for (int i = 0; i < 5; ++i)
//     p.enqueue<AllocationMode::CanAlloc>(i);
//   CHECK(p.size() == 5);
//
//   int out;
//   p.dequeue<AllocationMode::CanAlloc>(out);
//   CHECK(p.size() == 4);
// }
//
// TEST_CASE("size is 0 after draining all elements", "[pager][size]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   for (int i = 0; i < 10; ++i)
//     p.enqueue<AllocationMode::CanAlloc>(i);
//   int out;
//   for (int i = 0; i < 10; ++i)
//     p.dequeue<AllocationMode::CanAlloc>(out);
//
//   CHECK(p.size() == 0);
// }
//
// TEST_CASE("enqueue_bulk then dequeue one-by-one", "[pager][enqueue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const std::vector<int> src = {10, 20, 30, 40, 50};
//   REQUIRE(p.enqueue_bulk<AllocationMode::CanAlloc>(src.data(), src.size()));
//
//   for (int expected : src) {
//     int out = -1;
//     REQUIRE(p.dequeue<AllocationMode::CanAlloc>(out));
//     CHECK(out == expected);
//   }
// }
//
// TEST_CASE("enqueue_bulk with count == 0 succeeds and adds nothing", "[pager][enqueue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   REQUIRE(p.enqueue_bulk<AllocationMode::CanAlloc>(nullptr, 0));
//   CHECK(p.size() == 0);
// }
//
// TEST_CASE("enqueue_bulk spanning multiple pages", "[pager][enqueue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const std::size_t N = BLOCK_SIZE * 4;
//   std::vector<int> src(N);
//   for (std::size_t i = 0; i < N; ++i)
//     src[i] = static_cast<int>(i);
//
//   REQUIRE(p.enqueue_bulk<AllocationMode::CanAlloc>(src.data(), N));
//   CHECK(p.size() == N);
//
//   for (std::size_t i = 0; i < N; ++i) {
//     int out = -1;
//     REQUIRE(p.dequeue<AllocationMode::CanAlloc>(out));
//     CHECK(out == static_cast<int>(i));
//   }
// }
//
// TEST_CASE("enqueue_bulk preserves all values (no off-by-one in index)", "[pager][enqueue_bulk]") {
//   // Specifically exercises the items[write_index - start_index] fix.
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const std::size_t N = BLOCK_SIZE + 1; // straddles a page boundary
//   std::vector<int> src(N);
//   for (std::size_t i = 0; i < N; ++i)
//     src[i] = static_cast<int>(i * 7); // non-trivial pattern
//
//   REQUIRE(p.enqueue_bulk<AllocationMode::CanAlloc>(src.data(), N));
//
//   std::vector<int> dst(N, -1);
//   for (std::size_t i = 0; i < N; ++i) {
//     REQUIRE(p.dequeue<AllocationMode::CanAlloc>(dst[i]));
//   }
//   CHECK(dst == src);
// }
//
// TEST_CASE("dequeue_bulk retrieves all enqueued elements", "[pager][dequeue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const int N = 20;
//   for (int i = 0; i < N; ++i)
//     p.enqueue<AllocationMode::CanAlloc>(i);
//
//   std::vector<int> out(N, -1);
//   std::size_t got = p.dequeue_bulk<AllocationMode::CanAlloc>(out.data(), N);
//
//   REQUIRE(got == static_cast<std::size_t>(N));
//   for (int i = 0; i < N; ++i)
//     CHECK(out[i] == i);
// }
//
// TEST_CASE("dequeue_bulk on empty pager returns 0", "[pager][dequeue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   std::vector<int> out(10, -1);
//   CHECK(p.dequeue_bulk<AllocationMode::CanAlloc>(out.data(), 10) == 0);
// }
//
// TEST_CASE("dequeue_bulk with max < available returns only max elements", "[pager][dequeue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   for (int i = 0; i < 10; ++i)
//     p.enqueue<AllocationMode::CanAlloc>(i);
//
//   std::vector<int> out(4, -1);
//   std::size_t got = p.dequeue_bulk<AllocationMode::CanAlloc>(out.data(), 4);
//
//   REQUIRE(got == 4);
//   for (int i = 0; i < 4; ++i)
//     CHECK(out[i] == i);
//   CHECK(p.size() == 6);
// }
//
// TEST_CASE("dequeue_bulk spanning multiple pages", "[pager][dequeue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const std::size_t N = BLOCK_SIZE * 3;
//   std::vector<int> src(N);
//   for (std::size_t i = 0; i < N; ++i)
//     src[i] = static_cast<int>(i);
//   p.enqueue_bulk<AllocationMode::CanAlloc>(src.data(), N);
//
//   std::vector<int> dst(N, -1);
//   std::size_t got = p.dequeue_bulk<AllocationMode::CanAlloc>(dst.data(), N);
//
//   REQUIRE(got == N);
//   CHECK(dst == src);
// }
//
// TEST_CASE("dequeue_bulk partial then remainder", "[pager][dequeue_bulk]") {
//   AtomicList rl = make_recycle_list();
//   Pager<int> p(&rl);
//
//   const int N = 30;
//   for (int i = 0; i < N; ++i)
//     p.enqueue<AllocationMode::CanAlloc>(i);
//
//   std::vector<int> first(10), second(20);
//   REQUIRE(p.dequeue_bulk<AllocationMode::CanAlloc>(first.data(), 10) == 10);
//   REQUIRE(p.dequeue_bulk<AllocationMode::CanAlloc>(second.data(), 20) == 20);
//
//   for (int i = 0; i < 10; ++i)
//     CHECK(first[i] == i);
//   for (int i = 0; i < 20; ++i)
//     CHECK(second[i] == i + 10);
// }
//
// TEST_CASE("elements are destroyed after dequeue_bulk", "[pager][dequeue_bulk][destructor]") {
//   Tracked::live = 0;
//   {
//     AtomicList rl = make_recycle_list();
//     Pager<Tracked> p(&rl);
//
//     const int N = 10;
//     std::vector<Tracked> src;
//     src.reserve(N);
//     for (int i = 0; i < N; ++i)
//       src.emplace_back(i);
//
//     p.enqueue_bulk<AllocationMode::CanAlloc>(src.data(), N);
//
//     std::vector<Tracked> dst(N);
//     p.dequeue_bulk<AllocationMode::CanAlloc>(dst.data(), N);
//
//     // Slots inside the page were explicitly destroyed (el.~T()) after move,
//     // so live count should only reflect src + dst, not double-counted page slots.
//     CHECK(Tracked::live == N * 2); // src(N) + dst(N)
//   }
//   CHECK(Tracked::live == 0);
// }
//
// TEST_CASE("elements are destroyed after single dequeue", "[pager][dequeue][destructor]") {
//   Tracked::live = 0;
//   {
//     AtomicList rl = make_recycle_list();
//     Pager<Tracked> p(&rl);
//
//     p.enqueue<AllocationMode::CanAlloc>(Tracked{42});
//     CHECK(Tracked::live == 1); // only the one inside the page
//
//     Tracked out{0};
//     p.dequeue<AllocationMode::CanAlloc>(out);
//     CHECK(out.value == 42);
//   }
//   CHECK(Tracked::live == 0);
// }
//
// TEST_CASE("removed pages are returned to recycle list", "[pager][recycle]") {
//   AtomicList rl = make_recycle_list();
//   {
//     Pager<int> p(&rl);
//
//     // Fill and drain two full pages so remove_page is called twice.
//     const int N = BLOCK_SIZE * 2;
//     for (int i = 0; i < N; ++i)
//       p.enqueue<AllocationMode::CanAlloc>(i);
//     int out;
//     for (int i = 0; i < N; ++i)
//       p.dequeue<AllocationMode::CanAlloc>(out);
//   }
//
//   // At least one page should have been recycled back.
//   CHECK(rl.get_head() != nullptr);
// }
//
// TEST_CASE("recycled pages are reused by a new Pager", "[pager][recycle]") {
//   AtomicList rl = make_recycle_list();
//
//   {
//     Pager<int> p(&rl);
//     for (std::size_t i = 0; i < BLOCK_SIZE; ++i)
//       p.enqueue<AllocationMode::CanAlloc>(i);
//     int out;
//     for (std::size_t i = 0; i < BLOCK_SIZE; ++i)
//       p.dequeue<AllocationMode::CanAlloc>(out);
//   }
//   // rl now holds at least one recycled page.
//
//   Pager<int> p2(&rl);
//   // AllocationMode::CannotAlloc mode — can only succeed if it reuses a recycled page.
//   REQUIRE(p2.enqueue<AllocationMode::CannotAlloc>(99));
//   int out = -1;
//   REQUIRE(p2.dequeue<AllocationMode::CannotAlloc>(out));
//   CHECK(out == 99);
// }

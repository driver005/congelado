// #include <catch2/catch_test_macros.hpp>
// #include <catch2/matchers/catch_matchers_range_equals.hpp>
// #include <catch2/matchers/catch_matchers_string.hpp>
// #include <optional>
// #include <string>
// // #include <cstddef>
//
// import hpack;
//
// // =============================================================================
// // Helpers
// // =============================================================================
// namespace {
//
// std::vector<std::uint8_t> huff_encode(const hpack::huffman::Huffman<> &h, std::string_view s) {
//   std::vector<std::uint8_t> out;
//   out.reserve(s.size());
//   h.encode(s, std::back_inserter(out));
//   return out;
// }
//
// auto huff_roundtrip(const hpack::huffman::Huffman<> &h, std::string_view s) {
//   auto encoded = huff_encode(h, s);
//   return h.decode({encoded.data(), encoded.size()});
// }
//
// // Expose Codec internals for primitive tests
// class TestCodec {
// public:
//   static std::vector<std::uint8_t> enc_int(std::uint32_t data, std::uint8_t prefix, std::uint8_t prefix_data = 0) {
//     std::vector<std::uint8_t> out;
//     auto it = std::back_inserter(out);
//     hpack::raw::Atom::encode_int(data, prefix, prefix_data, it);
//     return out;
//   }
//
//   static std::size_t dec_int(std::vector<std::uint8_t> data, std::uint8_t prefix) {
//     std::size_t pos = 0;
//     return hpack::raw::Atom::decode_int(data, pos, prefix);
//   }
//
//   std::vector<std::uint8_t> enc_str(const hpack::huffman::Huffman<> *huffman, std::string_view s) {
//     std::vector<std::uint8_t> out;
//     auto it = std::back_inserter(out);
//     hpack::raw::Atom::encode_stirng(huffman, s, it);
//     return out;
//   }
//
//   std::string dec_str(const hpack::huffman::Huffman<> &h, std::vector<std::uint8_t> data) {
//     std::size_t pos = 0;
//     return hpack::raw::Atom::decode_string(h, data, pos);
//   }
// };
//
// } // namespace
//
// using Catch::Matchers::ContainsSubstring;
// using Catch::Matchers::RangeEquals;
//
// // =============================================================================
// // Huffman
// // =============================================================================
// TEST_CASE("huffman — RFC 7541 Appendix C wire vectors", "[huffman][rfc]") {
//   hpack::huffman::Huffman<> h;
//
//   SECTION("www.example.com — C.4.1") {
//     CHECK_THAT(
//         huff_encode(h, "www.example.com"),
//         RangeEquals(std::vector<std::uint8_t>{0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4,
//         0xff}));
//   }
//
//   SECTION("no-cache — C.4.1") {
//     CHECK_THAT(huff_encode(h, "no-cache"), RangeEquals(std::vector<std::uint8_t>{0xa8, 0xeb, 0x10, 0x64, 0x9c,
//     0xbf}));
//   }
//
//   SECTION("custom-key — C.4.2") {
//     CHECK_THAT(huff_encode(h, "custom-key"),
//                RangeEquals(std::vector<std::uint8_t>{0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xa9, 0x7d, 0x7f}));
//   }
//
//   SECTION("custom-value — C.4.2") {
//     CHECK_THAT(huff_encode(h, "custom-value"),
//                RangeEquals(std::vector<std::uint8_t>{0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xb8, 0xe8, 0xb4, 0xbf}));
//   }
//
//   SECTION("302 — C.6.1") { CHECK_THAT(huff_encode(h, "302"), RangeEquals(std::vector<std::uint8_t>{0x64, 0x02})); }
//
//   SECTION("private — C.6.1") {
//     CHECK_THAT(huff_encode(h, "private"), RangeEquals(std::vector<std::uint8_t>{0xae, 0xc3, 0x77, 0x1a, 0x4b}));
//   }
//
//   SECTION("Mon, 21 Oct 2013 20:13:21 GMT — C.6.1") {
//     CHECK_THAT(
//         huff_encode(h, "Mon, 21 Oct 2013 20:13:21 GMT"),
//         RangeEquals(std::vector<std::uint8_t>{0xd0, 0x7a, 0xbe, 0x94, 0x10, 0x54, 0xd4, 0x44, 0xa8, 0x20, 0x05,
//                                               0x95, 0x04, 0x0b, 0x81, 0x66, 0xe0, 0x82, 0xa6, 0x2d, 0x1b, 0xff}));
//   }
//
//   SECTION("https://www.example.com — C.6.1") {
//     CHECK_THAT(huff_encode(h, "https://www.example.com"),
//                RangeEquals(std::vector<std::uint8_t>{0x9d, 0x29, 0xad, 0x17, 0x18, 0x63, 0xc7, 0x8f, 0x0b, 0x97,
//                0xc8,
//                                                      0xe9, 0xae, 0x82, 0xae, 0x43, 0xd3}));
//   }
// }
//
// TEST_CASE("huffman — roundtrip", "[huffman]") {
//   hpack::huffman::Huffman<> h;
//
//   SECTION("common HTTP strings") {
//     for (const char *s : {"ass", "hello world", "content-type", "application/json", ":method", "GET", "0123456789",
//                           "!\"#$%&'()*+,-./"}) {
//       CAPTURE(s);
//       CHECK(huff_roundtrip(h, s).contains(s));
//     }
//   }
//
//   SECTION("empty string") {
//     CHECK(huff_roundtrip(h, "").contains(""));
//     CHECK(huff_encode(h, "").empty());
//   }
//
//   SECTION("single characters") {
//     CHECK(huff_roundtrip(h, "a").contains("a"));
//     CHECK(huff_roundtrip(h, "0").contains("0"));
//   }
//
//   SECTION("all printable ASCII") {
//     std::string all;
//     for (int c = 32; c < 127; ++c)
//       all += static_cast<char>(c);
//     CHECK(huff_roundtrip(h, all).data() == all);
//   }
//
//   SECTION("long repeated string") {
//     std::string s(4096, 'a');
//     CHECK(huff_roundtrip(h, s).data() == s);
//   }
//
//   SECTION("huffman output smaller than raw for typical headers") {
//     std::string_view hdr = "www.example.com";
//     CHECK(huff_encode(h, hdr).size() < hdr.size());
//   }
// }
//
// TEST_CASE("huffman — decode errors", "[huffman][errors]") {
//   hpack::huffman::Huffman<> h;
//
//   SECTION("padding longer than 7 bits") {
//     std::vector<std::uint8_t> bad = {0x1f, 0xff};
//     CHECK_THROWS_WITH(h.decode(bad), ContainsSubstring("padding"));
//   }
//
//   SECTION("EOS symbol in stream") {
//     std::vector<std::uint8_t> bad = {0xff, 0xff, 0xff, 0xfc};
//     CHECK_THROWS_WITH(h.decode(bad), ContainsSubstring("EOS"));
//   }
// }
//
// // =============================================================================
// // Integer encoding — RFC 7541 §5.1 / Appendix C.1
// // =============================================================================
// TEST_CASE("integer encoding — RFC 7541 §5.1", "[integer][rfc]") {
//
//   SECTION("C.1.1 encode 10 N=5") {
//     CHECK_THAT(TestCodec::enc_int(10, 5), RangeEquals(std::vector<std::uint8_t>{0x0a}));
//   }
//
//   SECTION("C.1.2 encode 1337 N=5") {
//     CHECK_THAT(TestCodec::enc_int(1337, 5), RangeEquals(std::vector<std::uint8_t>{0x1f, 0x9a, 0x0a}));
//   }
//
//   SECTION("C.1.3 encode 42 N=8") {
//     CHECK_THAT(TestCodec::enc_int(42, 8), RangeEquals(std::vector<std::uint8_t>{0x2a}));
//   }
//
//   SECTION("C.1.1 decode 10 N=5") { CHECK(TestCodec::dec_int({0x0a}, 5) == 10); }
//
//   SECTION("C.1.2 decode 1337 N=5") { CHECK(TestCodec::dec_int({0x1f, 0x9a, 0x0a}, 5) == 1337); }
//
//   SECTION("C.1.3 decode 42 N=8") { CHECK(TestCodec::dec_int({0x2a}, 8) == 42); }
//
//   SECTION("boundary — value == 2^N-1") {
//     CHECK_THAT(TestCodec::enc_int(31, 5), RangeEquals(std::vector<std::uint8_t>{0x1f, 0x00}));
//     CHECK(TestCodec::dec_int({0x1f, 0x00}, 5) == 31);
//   }
//
//   SECTION("zero") {
//     CHECK_THAT(TestCodec::enc_int(0, 5), RangeEquals(std::vector<std::uint8_t>{0x00}));
//     CHECK(TestCodec::dec_int({0x00}, 5) == 0);
//   }
//
//   SECTION("prefix_data ORed into first byte") {
//     CHECK_THAT(TestCodec::enc_int(10, 5, 0x60), RangeEquals(std::vector<std::uint8_t>{0x6a}));
//   }
//
//   SECTION("roundtrip — values × prefix widths") {
//     for (std::uint32_t v : {0u, 1u, 30u, 31u, 127u, 128u, 255u, 256u, 1337u, 65535u, 1u << 20}) {
//       for (std::uint8_t n : {1, 2, 4, 5, 6, 7, 8}) {
//         CAPTURE(v, n);
//         CHECK(TestCodec::dec_int(TestCodec::enc_int(v, n), n) == v);
//       }
//     }
//   }
//
//   SECTION("error — truncated multi-byte integer") {
//     CHECK_THROWS_AS(TestCodec::dec_int({0x1f, 0x80}, 5), std::runtime_error);
//   }
//
//   SECTION("error — empty span") { CHECK_THROWS_AS(TestCodec::dec_int({}, 5), std::runtime_error); }
// }
//
// // =============================================================================
// // String encoding — RFC 7541 §5.2
// // =============================================================================
// TEST_CASE("string encoding — RFC 7541 §5.2", "[string]") {
//   TestCodec tc;
//   hpack::huffman::Huffman<> h;
//
//   SECTION("encode uses raw when huffman not smaller") {
//     auto enc = tc.enc_str(nullptr, "GET");
//     auto val = enc[0];
//     CHECK((val & 0x80) == 0);
//     CHECK_THAT(enc, RangeEquals(std::vector<std::uint8_t>{0x03, 0x47, 0x45, 0x54}));
//   }
//
//   SECTION("encode uses huffman when smaller") {
//     auto enc = tc.enc_str(&h, "www.example.com");
//     auto val = enc[0];
//     CHECK((val & 0x80) != 0);
//     CHECK((val & 0x7f) == 12);
//     CHECK(enc.size() == 13);
//   }
//
//   SECTION("encode empty string") { CHECK_THAT(tc.enc_str(nullptr, ""), RangeEquals(std::vector<std::uint8_t>{0x00}));
//   }
//
//   SECTION("decode raw string") { CHECK(tc.dec_str(h, {0x03, 0x66, 0x6f, 0x6f}).contains("foo")); }
//
//   SECTION("decode huffman string — www.example.com") {
//     CHECK(tc.dec_str(h, {0x8c, 0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff}) ==
//           "www.example.com");
//   }
//
//   SECTION("decode empty raw string") { CHECK(tc.dec_str(h, {0x00}).contains("")); }
//
//   SECTION("roundtrip") {
//     for (const char *s : {"no-cache", "custom-key", "custom-value", "application/json", "https://www.example.com",
//     "",
//                           "a", "0123456789"}) {
//       CAPTURE(s);
//       CHECK(tc.dec_str(h, tc.enc_str(&h, s)) == s);
//     }
//   }
//
//   SECTION("error — declared length exceeds data") { CHECK_THROWS_AS(tc.dec_str(h, {0x0a, 0x01}), std::runtime_error);
//   }
//
//   SECTION("error — empty span") { CHECK_THROWS_AS(tc.dec_str(h, {}), std::runtime_error); }
// }
//
// // =============================================================================
// // Header table — RFC 7541 §2.3 / §4
// // =============================================================================
// TEST_CASE("header table — static entries", "[table][static]") {
//   hpack::table::HeaderTable t;
//
//   SECTION("index 1 = :authority") {
//     auto e = t[1];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains(":authority"));
//     CHECK(e.value().value().contains(""));
//   }
//
//   SECTION("index 2 = :method GET") {
//     auto e = t[2];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains(":method"));
//     CHECK(e.value().value().contains("GET"));
//   }
//
//   SECTION("index 61 = www-authenticate") {
//     auto e = t[61];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains("www-authenticate"));
//   }
//
//   SECTION("index 0 returns nullptr") {
//     auto e = t[0];
//     CHECK(e.has_value() == false);
//   }
//
//   SECTION("index 62 on empty dynamic table returns nullptr") {
//     auto e = t[62];
//     CHECK(e.has_value() == false);
//   }
// }
//
// TEST_CASE("header table — dynamic insertion", "[table][dynamic]") {
//   hpack::table::HeaderTable t;
//   t.insert("custom-key", "custom-value");
//
//   SECTION("newest entry at index 62") {
//     auto e = t[62];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains("custom-key"));
//     CHECK(e.value().value().contains("custom-value"));
//   }
//
//   SECTION("size = name + value + 32") {
//     CHECK(t.current_size() == 54); // 10 + 12 + 32
//   }
//
//   SECTION("second insert shifts first to 63") {
//     t.insert("content-type", "application/json");
//     auto e62 = t[62];
//     auto e63 = t[63];
//     REQUIRE(e62.has_value() == true);
//     REQUIRE(e63.has_value() == true);
//     CHECK(e62->name().contains("content-type"));
//     CHECK(e63->name().contains("custom-key"));
//   }
// }
//
// TEST_CASE("header table — search", "[table][search]") {
//   hpack::table::HeaderTable t;
//   t.insert("content-type", "application/json");
//
//   SECTION("full match in static table") {
//     auto [idx, full] = t.search(":method", "GET");
//     CHECK(idx == 2);
//     CHECK(full == true);
//   }
//
//   SECTION("name-only match in static table") {
//     auto [idx, full] = t.search(":method", "DELETE");
//     CHECK(idx != 0);
//     CHECK(full == false);
//   }
//
//   SECTION("full match in dynamic table") {
//     auto [idx, full] = t.search("content-type", "application/json");
//     CHECK(idx == 62);
//     CHECK(full == true);
//   }
//
//   SECTION("static full match preferred over dynamic name-only") {
//     t.insert(":status", "418");
//     auto [idx, full] = t.search(":status", "200");
//     CHECK(idx == 8);
//     CHECK(full == true);
//   }
//
//   SECTION("not found") {
//     auto [idx, full] = t.search("x-unknown", "whatever");
//     CHECK(idx == 0);
//     CHECK(full == false);
//   }
// }
//
// TEST_CASE("header table — eviction", "[table][eviction]") {
//
//   SECTION("evicts oldest when full") {
//     hpack::table::HeaderTable t{64};
//     t.insert("a", "b");   // 34 bytes
//     t.insert("cc", "dd"); // 36 bytes → 70 > 64 → evict a:b
//     CHECK(t.dynamic_size() == 1);
//     auto e = t[62];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains("cc"));
//   }
//
//   SECTION("entry larger than max_size clears table") {
//     hpack::table::HeaderTable t{10};
//     t.insert("a", "b"); // 34 > 10 → clear, not inserted
//     CHECK(t.dynamic_size() == 0);
//     CHECK(t.current_size() == 0);
//   }
//
//   SECTION("set_max_size evicts oldest entries") {
//     hpack::table::HeaderTable t{4096};
//     t.insert("key1", "val1");
//     t.insert("key2", "val2");
//     t.set_max_size(40);
//     CHECK(t.dynamic_size() == 1);
//     auto e = t[62];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains("key2"));
//   }
//
//   SECTION("set_max_size(0) clears everything") {
//     hpack::table::HeaderTable t{4096};
//     t.insert("x", "y");
//     t.set_max_size(0);
//     CHECK(t.dynamic_size() == 0);
//     CHECK(t.current_size() == 0);
//   }
// }
//
// TEST_CASE("header table — insert by index (RFC §4.4)", "[table][insert-by-index]") {
//
//   SECTION("name resolved before eviction") {
//     hpack::table::HeaderTable t{80};
//     t.insert("content-type", "text/html"); // 13+9+32 = 54
//
//     // new entry = 13+16+32 = 61 → 54+61 = 115 > 80 → evicts index 62
//     // bool ok = t.insert(62, "application/json");
//     t.insert("content-type", "application/json");
//
//     // REQUIRE(ok);
//     auto e = t[62];
//     REQUIRE(e.has_value() == true);
//     CHECK(e.value().name().contains("content-type"));
//     CHECK(e.value().value().contains("application/json"));
//     CHECK(t.dynamic_size() == 1);
//   }
//
//   // SECTION("invalid index returns false") {
//   //   hpack::table::HeaderTable t;
//   //   CHECK(t.insert(999, "value") == false);
//   // }
// }

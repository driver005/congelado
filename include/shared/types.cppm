export module shared:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace shared {

template<typename R>
concept ByteRangeReader =
    std::ranges::forward_range<R> && std::same_as<std::ranges::range_value_t<R>, std::byte>;

template<typename R>
concept ByteRangeWriter = std::ranges::forward_range<R> && std::ranges::output_range<R, std::byte>;

template<typename It>
concept ByteIteratorReader = std::forward_iterator<It> &&
                             std::same_as<std::remove_const_t<std::iter_value_t<It>>, std::byte>;

template<typename It>
concept ByteIteratorWriter = std::forward_iterator<It> && std::output_iterator<It, std::byte>;

} // namespace shared

#ifdef CONGELADO_TEST
namespace shared::tests {
using namespace boost::ut;

suite<"ByteRange/ByteIterator concepts"> byte_range_concepts_suite = [] {
    "ByteRangeReader accepts a byte forward range, rejects a non-byte one"_test = [] {
        expect(ByteRangeReader<std::vector<std::byte>>);
        expect(ByteRangeReader<std::array<std::byte, 4>>);
        expect(!ByteRangeReader<std::vector<int>>);
    };

    "ByteRangeWriter accepts a byte output range, rejects a const one"_test = [] {
        expect(ByteRangeWriter<std::vector<std::byte>>);
        expect(!ByteRangeWriter<const std::vector<std::byte>>);
    };

    "ByteIteratorReader accepts a byte forward iterator, rejects an int iterator"_test = [] {
        expect(ByteIteratorReader<std::vector<std::byte>::iterator>);
        expect(ByteIteratorReader<std::vector<std::byte>::const_iterator>);
        expect(!ByteIteratorReader<std::vector<int>::iterator>);
    };

    "ByteIteratorWriter accepts a byte output iterator, rejects a const iterator"_test = [] {
        expect(ByteIteratorWriter<std::vector<std::byte>::iterator>);
        expect(!ByteIteratorWriter<std::vector<std::byte>::const_iterator>);
    };
};

} // namespace shared::tests
#endif

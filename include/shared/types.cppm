export module shared:types;

import std;

export namespace shared {

template <typename R>
concept ByteRangeReader = std::ranges::forward_range<R> && std::same_as<std::ranges::range_value_t<R>, std::byte>;

template <typename R>
concept ByteRangeWriter = std::ranges::forward_range<R> && std::ranges::output_range<R, std::byte>;

template <typename It>
concept ByteIteratorReader =
    std::forward_iterator<It> && std::same_as<std::remove_const_t<std::iter_value_t<It>>, std::byte>;

template <typename It>
concept ByteIteratorWriter = std::forward_iterator<It> && std::output_iterator<It, std::byte>;

} // namespace shared

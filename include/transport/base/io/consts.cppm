export module io:consts;

import std;

export namespace transport::base::io {

constexpr std::size_t PAGE_SIZE = 4096;
constexpr std::size_t BUFFER_SIZE = PAGE_SIZE * 2;

} // namespace transport::base::io

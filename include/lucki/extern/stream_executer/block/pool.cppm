module;

#include <sycl/sycl.hpp>

export module lucki_extern_stream_executor:block_pool;

import std;

export namespace lucki::stream_executor::block {


struct BlockSizeLess
{
public:
    bool operator()(const Block* left, const Block* right) const noexcept
    {
        if (left->get_queue() != right->get_queue()) {
            return std::less<>{}(left->get_queue(), right->get_queue());
        }
        if (left->get_size() != right->get_size()) {
            return left->get_size() < right->get_size();
        }
        return std::less<>{}(left->get_data(), right->get_data());
    }
};

class BlockAddressLess
{
public:
    bool operator()(const Block* left, const Block* right) const noexcept
    {
        if (left->get_queue() != right->get_queue()) {
            return std::less<>{}(left->get_queue(), right->get_queue());
        }
        return std::less<>{}(left->get_data(), right->get_data());
    }
};

class BlockPool
{
public:

private:
}


} // namespace lucki::stream_executor::block

module;
#include <cstddef>
export module server:handler;

import std;
import :consts;
import :types;

export namespace transport::server {

template <typename Request, typename Response, std::uint8_t MaxHandlerSize = 8>
class Handler {
  public:
    constexpr Handler() : m_handler{}, m_handler_mask{HANDLER_MASK}, m_handler_index{0} {}

    constexpr void push(const Method &method, HandlerFn<Request, Response> handler) {
        if (m_handler_index >= MaxHandlerSize)
            throw std::runtime_error(
                "HandlerSize cannot be greater than MaxHandlerSize due to offerflow limitations, please check "
                "your routes");

        const std::uint8_t shift = std::to_underlying(method) * 8;

        if (((m_handler_mask >> shift) & 0xFF) != 0xFF) {
            throw std::runtime_error("Handler for method already exists");
        }

        m_handler_mask &= ~(0xFFULL << shift);
        m_handler_mask |= (static_cast<std::uint64_t>(m_handler_index) << shift);
        m_handler[m_handler_index++] = handler;
    }

    constexpr HandlerFn<Request, Response> find(Method method) const noexcept {
        const std::uint8_t idx = (m_handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return idx != 0xFF ? m_handler[idx] : nullptr;
    }

    constexpr const std::uint8_t &get_size() const noexcept { return m_handler_index; }
    constexpr const std::size_t &get_mask() const noexcept { return m_handler_mask; }
    constexpr const std::array<HandlerFn<Request, Response>, MaxHandlerSize> &get_handler() const noexcept {
        return m_handler;
    }

  private:
    std::array<HandlerFn<Request, Response>, MaxHandlerSize> m_handler;
    std::size_t m_handler_mask;
    std::uint8_t m_handler_index;
};

template <typename Request, typename Response, std::size_t MaxHandlerSize>
class HandlerPool {
  public:
    constexpr HandlerPool() : m_handler{}, m_handler_index{0} {}

    constexpr HandlerFn<Request, Response> find(std::size_t idx, std::size_t handler_mask,
                                                Method method) const noexcept {
        const std::uint8_t offset = (handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return offset != 0xFF ? m_handler[idx + offset] : nullptr;
    }

    constexpr void push(HandlerFn<Request, Response> handler) {
        if (m_handler_index >= MaxHandlerSize)
            throw std::runtime_error(
                "HandlerSize cannot be greater than MaxHandlerSize due to offerflow limitations, please check "
                "your routes");

        m_handler[m_handler_index++] = handler;
    }

    constexpr auto begin() noexcept { return m_handler.begin(); }
    constexpr auto end() noexcept { return m_handler.begin() + m_handler_index; }

    constexpr auto begin() const noexcept { return m_handler.begin(); }
    constexpr auto end() const noexcept { return m_handler.begin() + m_handler_index; }

    // Response &execute(const Request &req) const noexcept { return run_step(req, 0); }

    constexpr std::uint8_t &get_size() noexcept { return m_handler_index; }

  private:
    std::array<HandlerFn<Request, Response>, MaxHandlerSize> m_handler;
    std::uint8_t m_handler_index;
};

} // namespace transport::server

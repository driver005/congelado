module;
#include <cstddef>
export module core_router:handler;

import std;
import interfaces;
import :consts;

export namespace core::router {

template <std::uint8_t MaxHandlerSize = 8>
class Handler {
  public:
    constexpr Handler() : m_handler{}, m_handler_mask{HANDLER_MASK}, m_handler_index{0} {}

    constexpr void add_handler(const interfaces::io::types::Method &method,
                               interfaces::HandlerFn handler) {
        if (m_handler_index >= MaxHandlerSize)
            throw std::runtime_error("HandlerSize cannot be greater than MaxHandlerSize due to "
                                     "offerflow limitations, please check "
                                     "your routes");

        const std::uint8_t shift = std::to_underlying(method) * 8;

        if (((m_handler_mask >> shift) & 0xFF) != 0xFF) {
            throw std::runtime_error("Handler for method already exists");
        }

        m_handler_mask &= ~(0xFFULL << shift);
        m_handler_mask |= (static_cast<std::uint64_t>(m_handler_index) << shift);
        m_handler[m_handler_index++] = handler;
    }

    constexpr interfaces::HandlerFn find(interfaces::io::types::Method method) const noexcept {
        const std::uint8_t idx = (m_handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return idx != 0xFF ? m_handler[idx] : nullptr;
    }

    constexpr const std::uint8_t &get_size() const noexcept { return m_handler_index; }
    constexpr const std::size_t &get_mask() const noexcept { return m_handler_mask; }
    constexpr const std::array<interfaces::HandlerFn, MaxHandlerSize> &
    get_handler() const noexcept {
        return m_handler;
    }

  private:
    std::array<interfaces::HandlerFn, MaxHandlerSize> m_handler;
    std::size_t m_handler_mask;
    std::uint8_t m_handler_index;
};

template <std::size_t MaxHandlerSize>
class HandlerPool {
  public:
    constexpr HandlerPool() : m_handler{}, m_handler_index{0} {}

    constexpr interfaces::HandlerFn find(std::size_t idx, std::size_t handler_mask,
                                         interfaces::io::types::Method method) const noexcept {
        const std::uint8_t offset = (handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return offset != 0xFF ? m_handler[idx + offset] : nullptr;
    }

    constexpr void add_handler(interfaces::HandlerFn handler) {
        if (m_handler_index >= MaxHandlerSize)
            throw std::runtime_error("HandlerSize cannot be greater than MaxHandlerSize due to "
                                     "offerflow limitations, please check "
                                     "your routes");

        m_handler[m_handler_index++] = handler;
    }

    constexpr auto begin() noexcept { return m_handler.begin(); }
    constexpr auto end() noexcept { return m_handler.begin() + m_handler_index; }

    constexpr auto begin() const noexcept { return m_handler.begin(); }
    constexpr auto end() const noexcept { return m_handler.begin() + m_handler_index; }

    constexpr std::uint8_t &get_size() noexcept { return m_handler_index; }

  private:
    std::array<interfaces::HandlerFn, MaxHandlerSize> m_handler;
    std::uint8_t m_handler_index;
};

} // namespace core::router

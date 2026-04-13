module;
#include <cstddef>
export module io_server:router;

import std;
import :types;
import :handler;
import :consts;
import :middleware;


namespace transport::server {

constexpr auto split_path(std::string_view path) noexcept {
    return std::views::concat(std::views::single(std::string_view{"/"}),
                              path | std::views::split('/') | std::views::transform([](auto &&rng) {
                                  return std::string_view(std::ranges::begin(rng), std::ranges::end(rng));
                              }) | std::views::filter([](std::string_view sv) { return !sv.empty(); }));
}

// constexpr std::uint32_t fnv1a(std::string_view s) noexcept {
//     std::uint32_t h = 2166136261u;
//     for (char c : s)
//         h = (h ^ static_cast<std::uint8_t>(c)) * 16777619u;
//     return h;
// }

constexpr std::uint32_t fnv1a(std::string_view s) noexcept {
    std::uint32_t h = 5381u;
    for (char c : s)
        h = h * 33u ^ static_cast<std::uint8_t>(c);
    return h;
}
} // namespace transport::server

export namespace transport::server {

template <typename Request, typename Response>
class RouterNode {
  public:
    RouterNode() = default;

    RouterNode(EdgeKind kind, std::string_view path, std::uint16_t children_offset, std::uint8_t children_length,
               std::uint16_t middleware_offset, std::uint8_t middleware_size, std::uint16_t handler_offset,
               std::size_t handler_mask)
        : m_kind{kind}, m_path{path}, m_children_offset{children_offset}, m_children_length{children_length},
          m_middleware_offset{middleware_offset}, m_middleware_length{middleware_size},
          m_handler_offset{handler_offset}, m_handler_mask{handler_mask} {}

    constexpr std::optional<RouterNode<Request, Response>> find_child(std::string_view seg,
                                                                      std::span<RouterNode<Request, Response>> table,
                                                                      std::uint8_t &current_index) const noexcept {
        if (m_children_length == 0)
            return std::nullopt;

        current_index += m_children_offset;

        std::optional<std::size_t> wild_index{std::nullopt};

        if (m_children_length > 1) {
            const std::uint32_t hash = fnv1a(seg) % m_children_length;

            for (std::uint32_t i = 0; i < m_children_length; ++i) {
                std::uint8_t idx = current_index + (hash + i) % m_children_length;

                auto &node = table[idx];
                std::println("Checking node at index {}: kind={}, literal={}", idx, std::to_underlying(node.get_kind()),
                             node.get_literal());
                std::println("wild_index: {}", wild_index.has_value() ? std::to_string(*wild_index) : "nullopt");

                switch (node.get_kind()) {
                case EdgeKind::Path:
                    if (node.get_literal() == seg) {
                        std::println("Found matching path node at index {}: {}", idx, node.get_literal());
                        return std::make_optional(node);
                    }
                    break;
                case EdgeKind::Param:
                    return std::make_optional(node);
                case EdgeKind::Wild:
                    wild_index = std::make_optional(idx);
                    continue;
                }
            }
        } else {
            auto &node = table[current_index];

            switch (node.get_kind()) {
            case EdgeKind::Path:
                if (node.get_literal() == seg) {
                    return std::make_optional(node);
                }
                break;
            case EdgeKind::Param:
                return std::make_optional(node);
            case EdgeKind::Wild:
                wild_index = std::make_optional(current_index);
            }
        }

        if (wild_index) {
            std::println("Returning wildcard node at index {}", *wild_index);
            return std::make_optional(table[*wild_index]);
        }

        return std::nullopt;
    }

    constexpr std::uint16_t &get_data_offset() noexcept { return m_children_offset; }
    constexpr std::uint8_t &get_children_length() noexcept { return m_children_length; }
    constexpr std::uint16_t &get_middleware_offset() noexcept { return m_middleware_offset; }
    constexpr std::uint8_t &get_middleware_length() noexcept { return m_middleware_length; }
    constexpr std::uint16_t &get_handler_offset() noexcept { return m_handler_offset; }
    constexpr std::size_t &get_handler_mask() noexcept { return m_handler_mask; }
    constexpr EdgeKind &get_kind() noexcept { return m_kind; }
    constexpr std::string_view get_literal() noexcept { return m_path; }

  private:
    EdgeKind m_kind;
    std::string_view m_path;
    std::uint16_t m_children_offset;
    std::uint8_t m_children_length;
    std::uint16_t m_middleware_offset;
    std::uint8_t m_middleware_length;
    std::uint16_t m_handler_offset;
    std::size_t m_handler_mask;
};

template <typename Request, typename Response, std::size_t RouterSize = 256, std::size_t HandlerSize = 64,
          std::size_t MiddlewareSize = 10>
class RouteHandler {
  public:
    constexpr explicit RouteHandler() : m_table{}, m_handler{}, m_middleware{}, m_table_index{0} {}

    constexpr RouteHandler(RouteHandler &&) noexcept = default;
    constexpr RouteHandler &operator=(RouteHandler &&) noexcept = default;

    RouteHandler(const RouteHandler &) = delete;
    RouteHandler &operator=(const RouteHandler &) = delete;

    ~RouteHandler() = default;

    constexpr void match(Method method, std::string_view path, Request req, Response res) {
        auto segments = split_path(path);
        auto it = segments.begin();
        auto end = segments.end();

        if (it != end) {
            ++it;

            RouterNode<Request, Response> current = m_table[0];
            std::uint8_t current_index = 0;

            for (; it != end; ++it) {
                std::println("Current node index: {}, looking for segment: {}", current_index, *it);
                auto node = current.find_child(*it, m_table, current_index);
                if (!node)
                    break;
                if (node->get_middleware_length() > 0) {
                    m_middleware.execute(req, res, node->get_middleware_offset(), node->get_middleware_length());
                }
                current = *node;
            }

            std::println("Edge kind: {}, literal: {}, children length: {}, middleware length: {}, handler offset: {}, "
                         "handler mask: {:016X}",
                         std::to_underlying(current.get_kind()), current.get_literal(), current.get_children_length(),
                         current.get_middleware_length(), current.get_handler_offset(), current.get_handler_mask());

            std::println("Handler mask: {:016X}, looking for method: {}", current.get_handler_mask(),
                         std::to_underlying(method));

            std::println("Gert handler offset: {}, middleware offset: {}, handler mask: {:016X}",
                         current.get_handler_offset(), current.get_middleware_offset(), current.get_handler_mask());

            if (current.get_children_length() == 0) {
                if (current.get_handler_mask() != HANDLER_MASK) {
                    const auto handler_fn =
                        m_handler.find(current.get_handler_offset(), current.get_handler_mask(), method);
                    if (handler_fn) {
                        handler_fn(req, res);
                        return;
                    } else {
                        throw std::runtime_error(std::format("Wrong method for route: {}", std::to_underlying(method)));
                    }
                }
            }
        }

        throw std::runtime_error("Route not found");
    }

    constexpr void add_route(EdgeKind kind, std::string_view path, std::size_t children_offset,
                             const Handler<Request, Response, 8> &handler,
                             const Middleware<Request, Response, MiddlewareSize> &middleware,
                             std::uint8_t children_size) noexcept {

        RouterNode<Request, Response> node{
            kind,
            path,
            static_cast<std::uint16_t>(children_offset),
            children_size,
            m_middleware.get_size(),
            middleware.get_size(),
            m_handler.get_size(),
            handler.get_mask(),
        };

        std::println("Adding route: {}, kind: {}, children offset: {}, children size: {}, middleware offset: {}, "
                     "middleware size: {}, handler offset: {}, handler mask: {:016X}",
                     path, std::to_underlying(kind), node.get_data_offset(), node.get_children_length(),
                     node.get_middleware_offset(), node.get_middleware_length(), node.get_handler_offset(),
                     node.get_handler_mask());

        for (std::uint8_t i = 0; i < middleware.get_size(); ++i) {
            m_middleware.push(middleware.get_middlewares()[i]);
        }

        for (std::uint8_t i = 0; i < handler.get_size(); ++i) {
            m_handler.push(handler.get_handler()[i]);
        }

        m_table[m_table_index++] = node;
    }

  private:
    std::array<RouterNode<Request, Response>, RouterSize> m_table;
    HandlerPool<Request, Response, HandlerSize> m_handler;
    Middleware<Request, Response, MiddlewareSize> m_middleware;
    std::uint8_t m_table_index;
};

} // namespace transport::server

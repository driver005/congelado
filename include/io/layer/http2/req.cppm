module;
#include <cassert>
#include <ranges>
export module io_layer_http2:request;

import std;
import hashmap;
import interfaces;
import io_layer_shared;
import io_codec_hpack;
import utils_buffering;
import utils_encode;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpRequest : public interfaces::io::IRequest {
  public:
    explicit HttpRequest(std::uint32_t stream_id)
        : interfaces::io::IRequest{stream_id}, m_static_headers{} {}

    // // --- Builder methods ---
    // HttpRequest &&with_method(shared::http::HttpMethod method) && {
    //     add_header(interfaces::io::types::Token::METHOD, method_str(method));
    //     return std::move(*this);
    // }
    // HttpRequest &&with_method(std::string_view method) && {
    //     add_header(interfaces::io::types::Token::METHOD, method);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_path(std::string_view path) && {
    //     add_header(interfaces::io::types::Token::PATH, path);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_scheme(std::string_view schema) && {
    //     add_header(interfaces::io::types::Token::SCHEME, schema);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_authority(std::string_view authority) && {
    //     add_header(interfaces::io::types::Token::AUTHORITY, authority);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_header(std::string_view name, std::string_view value) && {
    //     add_header(name, value);
    //     return std::move(*this);
    // }
    // HttpRequest &&with_header(interfaces::io::types::Token token, std::string_view value) && {
    //     add_header(token, value);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_query(std::string_view key, std::string_view value) && {
    //     auto &path_field =
    //     m_static_headers[std::to_underlying(interfaces::io::types::Token::PATH)]; std::string
    //     new_path; if (path_field) {
    //         new_path = path_field->get_value();
    //     }
    //     new_path += new_path.contains('?') ? '&' : '?';
    //     new_path += utils::encode::url_encode(key);
    //     new_path += '=';
    //     new_path += utils::encode::url_encode(value);
    //     add_header(interfaces::io::types::Token::PATH, new_path);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_bearer_auth(std::string_view token) && {
    //     add_header(interfaces::io::types::Token::AUTHORIZATION, "Bearer " + std::string(token));
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_basic_auth(std::string_view user, std::string_view password) && {
    //     add_header(interfaces::io::types::Token::AUTHORIZATION,
    //                "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
    //                                                        std::string(password)));
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_content_type(std::string_view mime) && {
    //     add_header(interfaces::io::types::Token::CONTENT_TYPE, mime);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_accept(std::string_view mime) && {
    //     add_header(interfaces::io::types::Token::ACCEPT, mime);
    //     return std::move(*this);
    // }
    //
    // HttpRequest &&with_user_agent(std::string_view user) && {
    //     add_header(interfaces::io::types::Token::USER_AGENT, user);
    //     return std::move(*this);
    // }
    //
    // [[nodiscard]] HttpRequest build() && { return std::move(*this); }

    HttpRequest(const HttpRequest &) = delete;
    HttpRequest &operator=(const HttpRequest &) = delete;
    constexpr HttpRequest(HttpRequest &&) noexcept = default;
    constexpr HttpRequest &operator=(HttpRequest &&) noexcept = default;

    void set_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token,
                    std::string_view value) &
        override {
        std::visit(
            [this, value](auto &&name) {
                using T = std::decay_t<decltype(name)>;

                if constexpr (std::is_same_v<T, std::string_view>) {
                    if (name.empty()) {
                        throw std::invalid_argument("Header name cannot be empty");
                    }

                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (!token_opt.has_value()) {
                        if (!value.empty()) {
                            if (auto existing_opt = m_headers.find(name);
                                existing_opt.has_value()) {
                                const auto &existing = *existing_opt;
                                existing->set_value(existing->get_value() +
                                                    interfaces::consts::VALUE_SEPARATOR +
                                                    std::string(value));
                            }
                            return;
                        }
                        m_headers.insert(name, std::make_shared<interfaces::io::HeaderField<false>>(
                                                   name, value));
                    }

                    auto token = token_opt.value();

                    if (token == interfaces::io::types::Token::COOKIE) {
                        const auto IDX = std::to_underlying(interfaces::io::types::Token::COOKIE);
                        if (m_static_headers[IDX] == nullptr) {
                            m_static_headers[IDX] =
                                std::make_shared<interfaces::io::HeaderField<true>>(
                                    interfaces::io::types::Token::COOKIE, std::string(value));
                        } else if (!value.empty()) {
                            m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() +
                                                             interfaces::consts::COOKIE_SEPARATOR +
                                                             std::string(value));
                        }
                    } else {
                        set_header(token, value);
                    }

                } else if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    if (name == interfaces::io::types::Token::NONE) {
                        throw std::invalid_argument("interfaces::io::types::Token cannot be None");
                    }

                    m_static_headers[std::to_underlying(name)] =
                        std::make_shared<interfaces::io::HeaderField<true>>(name,
                                                                            std::string(value));
                }
            },
            name_or_token);
    }

    void remove_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token) &
        override {
        std::visit(
            [&](const auto &name) {
                using T = std::decay_t<decltype(name)>;
                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    m_static_headers[std::to_underlying(name)] = nullptr;
                } else {
                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (token_opt.has_value()) {
                        m_static_headers[std::to_underlying(token_opt.value())] = nullptr;
                    } else {
                        m_headers.erase(name);
                    }
                }
            },
            name_or_token);
    }

    [[nodiscard]] std::size_t get_size(std::size_t max_frame_payload) const noexcept {
        std::size_t total = 0;

        std::size_t header_block = std::ranges::fold_left(
            m_static_headers |
                std::views::filter([](const auto &field) noexcept { return field != nullptr; }),
            std::size_t{0},
            [](std::size_t acc, const auto &field) noexcept { return acc + field->size(); });

        // TODO: add ranges support to my swiss hashmap
        //  header_block = std::ranges::fold_left(m_headers, header_block, [](std::size_t acc,
        //  const auto &entry) noexcept {
        //      return acc + entry.value()->size();
        //  });
        for (const auto &entry : m_headers) {
            header_block += entry.value()->size();
        }

        std::size_t num_header_frames =
            (header_block == 0) ? 1 : (header_block + max_frame_payload - 1) / max_frame_payload;

        total += (num_header_frames * 9) + header_block;

        // --- DATA frames ---
        const std::size_t BODY_SIZE = m_body.size();

        if (BODY_SIZE > 0) {
            std::size_t num_data_frames = (BODY_SIZE + max_frame_payload - 1) / max_frame_payload;
            total += (num_data_frames * 9) + BODY_SIZE;
        } else {
            total += 9; // empty DATA frame with END_STREAM
        }

        return total;
    }

    [[nodiscard]] std::string_view
    find_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token)
        const noexcept override {
        return std::visit(
            [&](const auto &name) -> std::string_view {
                using T = std::decay_t<decltype(name)>;

                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    const auto &field = m_static_headers[std::to_underlying(name)];
                    return field ? std::string_view{field->get_value()} : std::string_view{};
                } else {
                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (token_opt.has_value()) {
                        const auto &field = m_static_headers[std::to_underlying(token_opt.value())];
                        return field ? std::string_view{field->get_value()} : std::string_view{};
                    }

                    auto result = m_headers.find(name);
                    return result.has_value() ? std::string_view{(*result)->get_value()}
                                              : std::string_view{};
                }
            },
            name_or_token);
    }

    void clear_headers() & noexcept override {
        // TODO: add clear functions in qpack else this gets bad fix after compiles
    }

    [[nodiscard]] std::vector<interfaces::io::HeaderEntry> get_headers() const noexcept override {
        std::vector<interfaces::io::HeaderEntry> result;
        for (const auto &field : m_static_headers) {
            if (field != nullptr) {
                result.emplace_back(field);
            }
        }
        for (const auto &entry : m_headers) {
            result.emplace_back(entry.value());
        }
        return result;
    }

    [[nodiscard]] utils::buffering::BufferView &get_body() noexcept override { return m_body; }

    [[nodiscard]] const utils::buffering::BufferView &get_body() const noexcept override {
        return m_body;
    }


    //  TODO: set verion automaticly
    //  virtual void set_version(std::string_view version) & noexcept { std::abort(); }
    // TODO: think of a solution for this!!!
    // virtual void set_no_decompress(bool disable) & noexcept { std::abort(); }
    void set_addr(std::string_view addr) & noexcept override {
        set_header(interfaces::io::types::Token::AUTHORITY, addr);
    }

    [[nodiscard]] std::string_view get_method() const noexcept override {
        return find_header(interfaces::io::types::Token::METHOD);
    }
    [[nodiscard]] std::string_view get_path() const noexcept override {
        return find_header(interfaces::io::types::Token::PATH);
    }
    [[nodiscard]] std::string_view get_host() const noexcept override {
        return find_header(interfaces::io::types::Token::HOST);
    }
    [[nodiscard]] std::string_view get_scheme() const noexcept override {
        return find_header(interfaces::io::types::Token::SCHEME);
    }
    [[nodiscard]] std::string_view get_authority() const noexcept override {
        return find_header(interfaces::io::types::Token::AUTHORITY);
    }
    [[nodiscard]] std::string_view get_content_type() const noexcept override {
        return find_header(interfaces::io::types::Token::CONTENT_TYPE);
    }
    [[nodiscard]] std::string_view get_accept() const noexcept override {
        return find_header(interfaces::io::types::Token::ACCEPT);
    }
    [[nodiscard]] std::string_view get_user_agent() const noexcept override {
        return find_header(interfaces::io::types::Token::USER_AGENT);
    }
    [[nodiscard]] std::string_view get_authorization() const noexcept override {
        return find_header(interfaces::io::types::Token::AUTHORIZATION);
    }


  private:
    // void add_header(std::string_view name, std::string_view value) {
    //     if (name.empty()) {
    //         throw std::invalid_argument("Header name cannot be empty");
    //     }
    //
    //     auto token = interfaces::io::types::tokenize(name);
    //     if (token == interfaces::io::types::Token::COOKIE) {
    //         const auto IDX = std::to_underlying(interfaces::io::types::Token::COOKIE);
    //         if (m_static_headers[IDX] == nullptr) {
    //             m_static_headers[IDX] = std::make_shared<interfaces::io::HeaderField<true>>(
    //                 interfaces::io::types::Token::COOKIE, std::string(value));
    //         } else if (!value.empty()) {
    //             m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() +
    //                                              interfaces::consts::COOKIE_SEPARATOR +
    //                                              std::string(value));
    //         }
    //     } else if (token == interfaces::io::types::Token::CUSTOM) {
    //         if (!value.empty()) {
    //             if (auto existing_opt = m_headers.find(name); existing_opt.has_value()) {
    //                 const auto &existing = *existing_opt;
    //                 existing->set_value(existing->get_value() +
    //                 interfaces::consts::VALUE_SEPARATOR +
    //                                     std::string(value));
    //             }
    //             return;
    //         }
    //         m_headers.add_header(name, std::make_shared<interfaces::io::HeaderField<false>>(name,
    //         value));
    //     } else {
    //         add_header(token, value);
    //     }
    // }
    //
    // void add_header(interfaces::io::types::Token token, std::string_view value) {
    //     if (token == interfaces::io::types::Token::NONE) {
    //         throw std::invalid_argument("interfaces::io::types::Token cannot be None");
    //     }
    //     if (token == interfaces::io::types::Token::CUSTOM) {
    //         throw std::invalid_argument("interfaces::io::types::Token cannot be Custom");
    //     }
    //
    //     m_static_headers[std::to_underlying(token)] =
    //         std::make_shared<interfaces::io::HeaderField<true>>(token, std::string(value));
    // }


    struct FactoryTag {};

    HttpRequest(FactoryTag, std::uint32_t stream_id, interfaces::io::types::Method method,
                std::string_view path)
        : HttpRequest{stream_id} {
        set_header(interfaces::io::types::Token::METHOD, method_str(method));
        set_header(interfaces::io::types::Token::PATH, path);
    }

    std::shared_ptr<interfaces::io::HeaderField<true>>
    get_static(const interfaces::io::types::Token &token) {
        return m_static_headers[std::to_underlying(token)];
    }

    std::uint32_t m_stream_id;
    // TODO: make length a constant
    std::array<std::shared_ptr<interfaces::io::HeaderField<true>>,
               std::to_underlying(interfaces::io::types::Token::WWW_AUTHENTICATE) + 1>
        m_static_headers{};

    hashmap::swiss::SwissHashMap<std::string_view,
                                 std::shared_ptr<interfaces::io::HeaderField<false>>>
        m_headers;

    utils::buffering::BufferView m_body;
};

struct WriteHttpRequestAdaptor : std::ranges::range_adaptor_closure<WriteHttpRequestAdaptor> {
    explicit constexpr WriteHttpRequestAdaptor(HttpRequest &req, codec::hpack::HPackTable &table,
                                               std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_req{req}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&output) const {
        const auto STREAM_ID = m_req.get().get_stream_id();
        auto header_entries = m_req.get().get_headers();

        bool first_frame = true;

        codec::hpack::HpackEncoder<std::uint32_t>{
            m_table.get(), std::span<const interfaces::io::HeaderEntry>(header_entries),
            m_max_frame_size,
            [&](std::span<const std::byte> data, codec::hpack::HpackFlushReason reason) {
                const auto TYPE = first_frame ? shared_layer::FrameType::HEADERS
                                              : shared_layer::FrameType::CONTINUATION;
                const std::uint8_t FLAGS =
                    (reason == codec::hpack::HpackFlushReason::END)
                        ? static_cast<std::uint8_t>(shared_layer::Flags::END_HEADERS)
                        : std::uint8_t{0};
                output.append_range(
                    std::views::empty<std::byte> |
                    FrameHeaderClosureAdaptor{static_cast<std::uint32_t>(data.size()), TYPE, FLAGS,
                                              STREAM_ID});
                output.append_range(data);
                first_frame = false;
            }}();

        if (m_req.get().get_body().empty()) {
            std::uint8_t data_flags = m_flags | shared_layer::Flags::END_STREAM;

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::DATA)
                             .add_flags(data_flags)
                             .add_stream_id(STREAM_ID)
                             .build();

            output.append_range(std::views::empty<std::byte> |
                                WriteFrameBuilderAdaptor{std::move(frame), m_max_frame_size});
        } else {
            output.append_range(m_req.get().get_body() |
                                WriteFrameClosureAdapter{STREAM_ID, shared_layer::FrameType::DATA,
                                                         m_flags, m_max_frame_size});
        }
    }

    std::reference_wrapper<HttpRequest> m_req;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2

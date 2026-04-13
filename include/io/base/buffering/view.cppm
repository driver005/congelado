export module io_base_buffering:view;

import std;
import :deleter;

export namespace transport::base::buffering {

class BufferView {
  public:
    BufferView() : m_ptr{nullptr}, m_size{0}, m_deleter{} {}

    BufferView(std::byte *ptr, std::size_t size, Deleter deleter) noexcept
        : m_ptr{ptr}, m_size{size}, m_deleter{std::move(deleter)} {}

    ~BufferView() {
        delete m_ptr;
        m_deleter.~Deleter();
    };

    BufferView(const BufferView &) = delete;
    BufferView &operator=(const BufferView &) = delete;
    BufferView(BufferView &&) noexcept = default;
    BufferView &operator=(BufferView &&) noexcept = default;

    [[nodiscard]] BufferView share(std::size_t offset, std::size_t len) const {
        if (offset + len > m_size)
            throw std::out_of_range("BufferView::share out of bounds");
        return BufferView{m_ptr + offset, len, m_deleter};
    }

    [[nodiscard]] BufferView share() const { return share(0, m_size); }

    void trim_front(std::size_t n) noexcept {
        m_ptr += n;
        m_size -= n;
    }
    void trim(std::size_t n) noexcept { m_size = n; }

    [[nodiscard]] std::byte *get_data() noexcept { return m_ptr; }
    [[nodiscard]] const std::byte *get_data() const noexcept { return m_ptr; }
    [[nodiscard]] std::size_t get_size() const noexcept { return m_size; }
    [[nodiscard]] bool get_empty() const noexcept { return m_size == 0; }
    [[nodiscard]] std::span<std::byte> get_span() noexcept { return {m_ptr, m_size}; }
    [[nodiscard]] std::span<const std::byte> get_span() const noexcept { return {m_ptr, m_size}; }
    [[nodiscard]] int get_use_count() const noexcept { return m_deleter.use_count(); }

  private:
    std::byte *m_ptr;
    std::size_t m_size;
    Deleter m_deleter;
};

} // namespace transport::base::buffering

export module io_base_buffering:pool;

import std;
import :view;

struct iovec {
    void *iov_base;
    std::size_t iov_len;
};

export namespace transport::base::buffering {

class BufferPool {
  public:
    explicit BufferPool(std::size_t min_size = 8 * 1024, std::size_t max_size = 64 * 1024)
        : m_min_size(min_size), m_max_size(max_size), m_current_size(min_size) {}

    BufferPool(const BufferPool &) = delete;
    BufferPool &operator=(const BufferPool &) = delete;
    BufferPool(BufferPool &&) = delete;
    BufferPool &operator=(BufferPool &&) = delete;

    [[nodiscard]] std::optional<BufferView> acquire() noexcept {
        auto *ptr = new (std::nothrow) std::byte[m_current_size];
        if (!ptr)
            return std::nullopt;
        Deleter d{[ptr]() noexcept { delete[] ptr; }};
        return BufferView{ptr, m_current_size, std::move(d)};
    }

    void notify_read(std::size_t bytes_read) noexcept {
        if (bytes_read == m_current_size)
            m_current_size = std::clamp(m_current_size * 2, m_min_size, m_max_size);
        else
            m_current_size = std::clamp(bytes_read, m_min_size, m_max_size);
    }

    [[nodiscard]] std::size_t get_size() const noexcept { return m_current_size; }

  private:
    std::size_t m_min_size;
    std::size_t m_max_size;
    std::size_t m_current_size;
};


} // namespace transport::base::buffering

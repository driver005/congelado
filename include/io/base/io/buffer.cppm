export module io_io:buffer;

import std;
import :consts;

export namespace io::base::io {

class BufferOverflowException : public std::runtime_error {
  public:
    BufferOverflowException() : std::runtime_error("Writer attempt to overwrite reader: Buffer Full") {}
};

class RingBuffer {
  public:
    RingBuffer() : m_data(std::make_unique<std::array<std::uint8_t, BUFFER_SIZE>>()) {}

    // Enforce uniqueness per socket: Disable copying and moving
    // This prevents accidental sharing or invalidation of buffers across connections
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;
    RingBuffer(RingBuffer &&) = delete;
    RingBuffer &operator=(RingBuffer &&) = delete;

    // --- Metadata & State ---

    [[nodiscard]] std::size_t available_to_read() const noexcept {
        const std::size_t h = m_head.load(std::memory_order_relaxed);
        const std::size_t t = m_tail.load(std::memory_order_acquire);
        if (t >= h)
            return t - h;
        return BUFFER_SIZE - h + t;
    }

    [[nodiscard]] std::size_t available_to_write() const noexcept {
        const std::size_t t = m_tail.load(std::memory_order_relaxed);
        const std::size_t h = m_head.load(std::memory_order_acquire);

        std::size_t used;
        if (t >= h)
            used = t - h;
        else
            used = BUFFER_SIZE - h + t;

        return BUFFER_SIZE - used - 1;
    }

    // --- Reader Interface ---

    [[nodiscard]] std::span<std::uint8_t> get_readable_span() const noexcept {
        const std::size_t h = m_head.load(std::memory_order_relaxed);
        const std::size_t t = m_tail.load(std::memory_order_acquire);

        if (t >= h) {
            return {m_data->data() + h, t - h};
        } else {
            return {m_data->data() + h, BUFFER_SIZE - h};
        }
    }

    void advance_read(std::size_t bytes) noexcept {
        const std::size_t h = m_head.load(std::memory_order_relaxed);
        m_head.store((h + bytes) % BUFFER_SIZE, std::memory_order_release);
    }

    // --- Writer Interface ---

    [[nodiscard]] std::span<std::uint8_t> get_writable_span() const noexcept {
        const std::size_t t = m_tail.load(std::memory_order_relaxed);
        const std::size_t h = m_head.load(std::memory_order_acquire);

        if (t >= h) {
            std::size_t space = BUFFER_SIZE - t;
            if (h == 0)
                space -= 1;
            return {m_data->data() + t, space};
        } else {
            return {m_data->data() + t, h - t - 1};
        }
    }

    void commit_write(std::size_t bytes) {
        if (bytes > available_to_write()) {
            throw BufferOverflowException();
        }
        const std::size_t t = m_tail.load(std::memory_order_relaxed);
        m_tail.store((t + bytes) % BUFFER_SIZE, std::memory_order_release);
    }

  private:
    std::unique_ptr<std::array<std::uint8_t, BUFFER_SIZE>> m_data;
    std::atomic<std::size_t> m_head{0};
    std::atomic<std::size_t> m_tail{0};
};

} // namespace io::base::io

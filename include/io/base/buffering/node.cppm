export module io_base_buffering:node;

import std;

export namespace io::base::buffering {

class BufferNode {
  public:
    explicit BufferNode(std::size_t s)
        : m_data{new std::byte[s], std::default_delete<std::byte[]>()}, m_size{s}, m_written{0}, m_next{nullptr} {}

    explicit BufferNode(const std::byte *data, std::size_t size) // Added 'const'
        : m_data{const_cast<std::byte *>(data), [](std::byte *) { /* no-op */ }}, m_size{size}, m_written{size} {}

    BufferNode(const BufferNode &) = delete;
    BufferNode &operator=(const BufferNode &) = delete;
    BufferNode(BufferNode &&) = default;
    BufferNode &operator=(BufferNode &&) = default;

    [[nodiscard]] std::byte *begin() noexcept { return m_data.get(); }
    [[nodiscard]] std::byte *end() noexcept { return m_data.get() + m_size; }
    [[nodiscard]] const std::byte *begin() const noexcept { return m_data.get(); }
    [[nodiscard]] const std::byte *end() const noexcept { return m_data.get() + m_size; }

    [[nodiscard]] std::byte *get_data() const noexcept { return m_data.get(); }
    [[nodiscard]] BufferNode *get_next() const noexcept { return m_next; }
    [[nodiscard]] std::size_t get_size() const noexcept { return m_size; }
    [[nodiscard]] std::size_t get_written() const noexcept { return m_written; }
    [[nodiscard]] std::size_t get_remaining() const noexcept { return m_size - m_written; }

    void set_next(BufferNode *node) noexcept { m_next = node; }

  private:
    std::unique_ptr<std::byte[], std::function<void(std::byte *)>> m_data;
    std::size_t m_size;
    std::size_t m_written;
    BufferNode *m_next;
};

} // namespace io::base::buffering

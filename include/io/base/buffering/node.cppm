export module io_base_buffering:node;

import std;

export namespace io::base::buffering {

class BufferNode {
  public:
    explicit BufferNode(std::size_t s)
        : m_data{std::make_unique<std::byte[]>(s)}, m_size{s}, m_written{0}, m_next{nullptr} {}

    BufferNode(const BufferNode &) = delete;
    BufferNode &operator=(const BufferNode &) = delete;


    BufferNode(BufferNode &&) = default;
    BufferNode &operator=(BufferNode &&) = default;

    [[nodiscard]] std::byte *get_data() const noexcept { return m_data.get(); }
    [[nodiscard]] BufferNode *get_next() const noexcept { return m_next; }
    [[nodiscard]] std::size_t get_size() const noexcept { return m_size; }
    [[nodiscard]] std::size_t get_written() const noexcept { return m_written; }
    [[nodiscard]] std::size_t get_remaining() const noexcept { return m_size - m_written; }

    void set_next(BufferNode *node) noexcept { m_next = node; }

  private:
    std::unique_ptr<std::byte[]> m_data;
    std::size_t m_size;
    std::size_t m_written;
    BufferNode *m_next;
};

} // namespace io::base::buffering

export module io_base_buffering:helper;

import std;
import :view;

export namespace transport::base::buffering {

class StreamBuffers {
  public:
    StreamBuffers() : m_frames{}, m_total_bytes{0} {}

    void push(BufferView view) {
        m_total_bytes += view.get_size();
        m_frames.push_back(std::move(view));
    }

    void consume(std::invocable<std::span<const std::byte>> auto &&fn) const {
        for (const auto &v : m_frames)
            std::invoke(fn, v.get_span());
    }

    [[nodiscard]] std::size_t total_bytes() const noexcept { return m_total_bytes; }
    [[nodiscard]] bool empty() const noexcept { return m_frames.empty(); }
    [[nodiscard]] std::size_t frame_count() const noexcept { return m_frames.size(); }

    ~StreamBuffers() = default;

  private:
    std::vector<BufferView> m_frames;
    std::size_t m_total_bytes;
};

} // namespace transport::base::buffering

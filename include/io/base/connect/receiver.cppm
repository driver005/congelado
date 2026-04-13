export module io_base_register:receiver;

import std;
import shared;
import io_base_buffering;
import io_base_leverage;

export namespace transport::base::connect {

class Receiver : public shared::HandlerBase {
  public:
    using LeveragerType = leverage::Leverager<leverage::Context>;
    using ReadCallback = std::move_only_function<void(buffering::BufferView, std::size_t)>;
    using ErrorCallback = std::move_only_function<void(int)>;

    Receiver(int fd, LeveragerType &leverager, ReadCallback on_read, ErrorCallback on_error)
        : m_leverager{leverager}, m_pool{}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_fd{fd},
          m_stalled{false}, m_closed{true} {}

    Receiver(const Receiver &) = delete;
    Receiver &operator=(const Receiver &) = delete;
    Receiver(Receiver &&) = delete;
    Receiver &operator=(Receiver &&) = delete;

    shared::WorkerFunction on_execute() override {
        return [this]() {
            resume();
            shared::this_handler::shedule();
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_closed = true; };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                m_on_error(e.code().value());
            } catch (...) {
                m_on_error(-1);
            }
        };
    }

    void resume() {
        if (m_stalled) {
            m_stalled = true;
            arm_read();
        }
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }
    [[nodiscard]] int get_fd() const noexcept { return m_fd; }

  private:
    void arm_read() {
        auto slot = m_pool.acquire();
        if (!slot) {
            m_stalled = true;
            return;
        }

        m_leverager.get().async_read(
            m_fd, slot->get_data(), static_cast<unsigned>(slot->get_size()), 0,
            [this, s = std::move(*slot)](int result) mutable { on_read_complete(std::move(s), result); });
    }

    void on_read_complete(buffering::BufferView slot, int result) {
        if (m_closed)
            return;

        if (result <= 0) {
            m_closed = true;
            m_on_error(-result);
            return;
        }

        const auto bytes = static_cast<std::size_t>(result);
        slot.trim(bytes);
        m_pool.notify_read(bytes);
        m_on_read(std::move(slot), bytes);

        m_stalled = false;
    }

    std::reference_wrapper<LeveragerType> m_leverager;
    buffering::BufferPool m_pool;
    ReadCallback m_on_read;
    ErrorCallback m_on_error;
    int m_fd;
    bool m_stalled;
    bool m_closed;
};

} // namespace transport::base::connect

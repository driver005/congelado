export module io_base_register:sender;

import std;
import io_base_buffering;
import io_base_leverage;
import shared;

export namespace transport::base::connect {

class Sender : public shared::HandlerBase {
  public:
    using LeveragerType = leverage::Leverager<leverage::Context>;
    using ErrorCallback = std::move_only_function<void(int)>;

    Sender(int fd, LeveragerType &leverager, ErrorCallback on_error)
        : m_leverager{leverager}, m_pending{}, m_on_error{std::move(on_error)}, m_fd{fd}, m_stalled{false},
          m_closed{true} {}

    Sender(const Sender &) = delete;
    Sender &operator=(const Sender &) = delete;
    Sender(Sender &&) = delete;
    Sender &operator=(Sender &&) = delete;

    void send(buffering::BufferView &slot) { m_pending.push(std::move(slot)); }

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
            arm_write();
        }
    }

    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }
    [[nodiscard]] int get_fd() const noexcept { return m_fd; }

  private:
    void arm_write() {
        if (m_pending.empty()) {
            m_stalled = false;
            return;
        }

        auto slot = std::move(m_pending.front());
        m_pending.pop();

        m_leverager.get().async_write(
            m_fd, slot.get_data(), static_cast<unsigned>(slot.get_size()), 0,
            [this, s = std::move(slot)](int result) mutable { on_write_complete(std::move(s), result); });
    }

    void on_write_complete(buffering::BufferView slot, int result) {
        if (m_closed)
            return;

        if (result <= 0) {
            m_closed = true;
            m_on_error(-result);
            return;
        }

        slot.~BufferView();

        m_stalled = false;
    }

    std::reference_wrapper<LeveragerType> m_leverager;
    std::queue<buffering::BufferView> m_pending;
    ErrorCallback m_on_error;
    int m_fd;
    bool m_stalled;
    bool m_closed;
};

} // namespace transport::base::connect

module;

#include <errno.h>

export module io_flow_sender:async;

import std;
import core_logger;
import interfaces;
import utils_buffering;
import io_base_socket;
import shared;

export namespace io::base::flow::async {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::IoAsyncSend<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    Sender(Worker &worker) : m_worker{worker}, m_pool{}, m_on_error{nullptr}, m_fatal{false} {
    }

    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_error{std::move(on_error)}, m_fatal{false} {
        attach();
    }

    ~Sender() = default;

    Sender(const Sender &) = delete;
    Sender &operator=(const Sender &) = delete;
    Sender(Sender &&) = delete;
    Sender &operator=(Sender &&) = delete;

    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    void build() {
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
        attach();
    }

    void send(utils::buffering::BufferNode slot) {
        core::logger::debug("io/send", "fd {} enqueue {} bytes", m_worker.get().get_fd(), slot.get_written());
        m_pool.push(std::move(slot));
    }


    std::string_view get_name() const noexcept override { return "Sender - Async"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (resume()) {
                core::logger::debug("io/send", "fd {} rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            } else {
                core::logger::debug("io/send", "fd {} releasing", m_worker.get().get_fd());
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            detach();
            m_worker.get().async_close();
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/send", "fd {} sys error: {} ({})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/send", "fd {} exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/send", "fd {} unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    shared::SendCallback get_submitter() {
        return [this](utils::buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    bool resume() {
        if (m_fatal) {
            return false;
        }
        arm_write();
        return true;
    }

    void attach() { m_worker.get().attach(); }
    void detach() { m_worker.get().detach(); }

    [[nodiscard]] bool get_stalled() const noexcept { return m_fatal; }

    [[nodiscard]] bool has_on_error() const noexcept { return !m_on_error; }

  private:
    void arm_write() {
        const auto fd = m_worker.get().get_fd();
        auto [data, size] = m_pool.get_view().front();

        if (!data || size == 0) {
            return;
        }

        core::logger::debug("io/send", "fd {} tx attempt {} bytes", fd, size);
        m_worker.get().async_send(data, static_cast<unsigned>(size),
                                  [this](int result) mutable { on_write_complete(result); });
    }

    void on_write_complete(int result) {
        const auto fd = m_worker.get().get_fd();
        if (result == -EAGAIN || result == -EWOULDBLOCK) {
            core::logger::debug("io/send", "fd {} would block, reschedule", fd);
            return;
        }

        if (result < 0) {
            const auto error_code = -result;
            core::logger::warning("io/send", "fd {} send error: {}", fd, error_code);
            m_fatal = true;
            m_on_error(fd, error_code);
            return;
        }

        core::logger::debug("io/send", "fd {} tx {} bytes", fd, result);
        m_pool.get_view().consume(result);
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(interfaces::io::AsyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::async

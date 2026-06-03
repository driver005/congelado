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
        core::logger::debug("Sender", "Created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_error{std::move(on_error)}, m_fatal{false} {
        core::logger::debug("Sender", "Created for worker with FD `{}`", m_worker.get().get_fd());
        attach();
    }

    ~Sender() { core::logger::debug("Sender", "Destructor called for worker with FD `{}`", m_worker.get().get_fd()); }

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
        core::logger::debug("Sender", "FD `{}` adding node to send pool with size `{}`", m_worker.get().get_fd(),
                            slot.get_written());
        m_pool.push(std::move(slot));
    }


    std::string_view get_name() const noexcept override { return "Sender - Async"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            core::logger::debug("Sender", "FD `{}` on_execute is being executed", m_worker.get().get_fd());
            if (resume()) {
                core::logger::info("Sender", "FD `{}` rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            } else {
                core::logger::info("Sender", "FD `{}` to be released", m_worker.get().get_fd());
                shared::this_handler::release();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            core::logger::debug("Sender", "FD `{}` on_released is being executed", m_worker.get().get_fd());
            detach();
            m_worker.get().async_close();
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            core::logger::debug("Sender", "FD `{}` on_error is being executed", m_worker.get().get_fd());
            if (!eptr)
                return;
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("Sender", "FD `{}` system_error: {} (code: {})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("Sender", "FD `{}` exception: {}", m_worker.get().get_fd(), e.what());
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("Sender", "FD `{}` unknown exception", m_worker.get().get_fd());
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    shared::SendCallback get_submitter() {
        return [this](utils::buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    bool resume() {
        core::logger::debug("Sender", "FD `{}` is being executed in resume with fatal = `{}`", m_worker.get().get_fd(),
                            m_fatal);
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
        auto [data, size] = m_pool.get_view().front();

        if (!data || size == 0) {
            core::logger::info("Sender", "FD `{}` no data to send", m_worker.get().get_fd());
            return;
        }

        core::logger::info("Sender", "FD `{}` attempting to send {} bytes", m_worker.get().get_fd(), size);
        m_worker.get().async_send(data, static_cast<unsigned>(size),
                                  [this](int result) mutable { on_write_complete(result); });
    }

    void on_write_complete(int result) {
        if (result == -EAGAIN || result == -EWOULDBLOCK) {
            core::logger::warning("Sender", "FD `{}` send would have blocked, rescheduling", m_worker.get().get_fd());
            return;
        }

        if (result < 0) {
            core::logger::warning("Sender", "FD `{}` send failed with error code `{}`", m_worker.get().get_fd(),
                                  -result);
            m_fatal = true;
            m_on_error(m_worker.get().get_fd(), -result);
            return;
        }

        core::logger::info("Sender", "FD `{}` sent {} bytes", m_worker.get().get_fd(), result);
        m_pool.get_view().consume(result);
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(interfaces::io::AsyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::async

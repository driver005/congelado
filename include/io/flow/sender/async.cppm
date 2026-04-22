module;

#include <errno.h>

export module io_flow_sender:async;

import std;
import core_logger;
import interfaces;
import io_base_buffering;
import io_base_socket;
import shared;

export namespace io::base::flow::async {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::IoAsyncSend<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    Sender(Worker &worker) : m_worker{worker}, m_pool{}, m_on_error{nullptr}, m_fatal{false} {
        core::logger::debug("Sender", "created for worker with FD `{}`", m_worker.get().get_fd());
    }

    Sender &&add_on_error(shared::ErrorCallback on_error) && {
        m_on_error = std::move(on_error);
        return std::move(*this);
    }

    void add_on_error(shared::ErrorCallback on_error) & { m_on_error = std::move(on_error); }

    Sender &&build() && {
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
        attach();
        return std::move(*this);
    }

    void build() & {
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
        attach();
    }

    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_pool{}, m_on_error{std::move(on_error)}, m_fatal{false} {
        core::logger::debug("Sender", "created for worker with FD `{}`", m_worker.get().get_fd());
        attach();
    }

    ~Sender() { core::logger::debug("Sender", "destructor called for worker with FD `{}`", m_worker.get().get_fd()); }

    Sender(const Sender &) = delete;
    Sender &operator=(const Sender &) = delete;

    Sender(Sender &&other) noexcept
        : m_worker{other.m_worker}, m_pool{std::move(other.m_pool)}, m_on_error{std::move(other.m_on_error)},
          m_fatal{std::move(other.m_fatal)} {}

    Sender &operator=(Sender &&other) noexcept {
        if (this != &other) {
            m_worker = other.m_worker;
            m_pool = std::move(other.m_pool);
            m_on_error = std::move(other.m_on_error);
            m_fatal = std::move(other.m_fatal);
        }
        return *this;
    }

    void send(buffering::BufferNode slot) { m_pool.push(std::move(slot)); }


    std::string_view name() const noexcept override { return "Sender - Async"; }

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
        return [this](buffering::BufferNode &&node) { this->send(std::move(node)); };
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
        auto view = m_pool.get_view();

        auto slot_opt = view.peek();

        if (auto slot = slot_opt.value(); slot_opt) {
            core::logger::info("Sender", "FD `{}` attempting to send {} bytes", m_worker.get().get_fd(),
                               slot->get_size());
            m_worker.get().async_send(slot->get_data(), static_cast<unsigned>(slot->get_size()),
                                      [this](int result) mutable { on_write_complete(result); });
        } else {
            core::logger::info("Sender", "FD `{}` no data to send", m_worker.get().get_fd());
        }
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
        m_pool.get_view().pop_front();
    }

    std::reference_wrapper<Worker> m_worker;
    buffering::BufferPool m_pool;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(interfaces::io::AsyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::async

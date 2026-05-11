export module shared:flow;

import std;
import io_base_buffering;
import :handler;

export namespace shared {

using ReadCallback = std::move_only_function<void(io::base::buffering::BufferView &)>;
using SendCallback = std::move_only_function<void(io::base::buffering::BufferNode &&)>;
using CloseCallback = std::move_only_function<void()>;
using ErrorCallback = std::move_only_function<void(int, int)>;
using CompletionCallback = std::move_only_function<void(int)>;

template <typename T>
concept FlowLayer = requires(SendCallback send, CloseCallback close) {
    { T(send, close) };
} && requires(T t) {
    { t.on_read() } -> std::convertible_to<ReadCallback>;
};


template <typename T, typename Controller, typename Leverager>
concept FlowBase =
    HandlerController<Controller> && requires(ReadCallback &&on_read, Leverager &leverager, Controller controller) {
        T{std::move(on_read), leverager, controller};
    } && requires(T t, int fd) {
        { t.on_send(fd) } -> std::convertible_to<SendCallback>;
    };

} // namespace shared

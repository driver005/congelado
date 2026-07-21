export module shared:flow;

import std;
import utils_buffering;
import :handler;

export namespace shared {

using ReadCallback = std::move_only_function<void(utils::buffering::BufferReader &)>;
using SendCallback = std::move_only_function<void(utils::buffering::BufferNode &&)>;
using CloseCallback = std::move_only_function<void()>;
using ErrorCallback = std::move_only_function<void(int, int)>;
using CompletionCallback = std::move_only_function<void(int)>;
using QueryReadFn = std::move_only_function<void(std::string_view)>;

template <typename T>
concept FlowLayer = requires(SendCallback send, CloseCallback close) {
    { T(send, close) };
} && requires(T instance) {
    { instance.on_read() } -> std::convertible_to<ReadCallback>;
};


template <typename T, typename Controller, typename Leverager>
concept FlowBase = HandlerController<Controller> &&
                   requires(ReadCallback &&on_read, Leverager &leverager, Controller controller) {
                       T{std::move(on_read), leverager, controller};
                   } && requires(T instance, int descriptor) {
                       { instance.on_send(descriptor) } -> std::convertible_to<SendCallback>;
                   };

} // namespace shared

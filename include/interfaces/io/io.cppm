export module interfaces:io;

export import :io_header;
export import :io_types;
export import :io_request;
export import :io_response;

import std;
import io_shared;

export namespace interfaces::io {

using ReceiveDispatchFn = std::function<void(IRequest &, IResponse &)>;

using SendDispatchFn = std::function<void(IRequest &)>;

template <typename Status>
using IoCallback = std::move_only_function<void(std::size_t, Status)>;

template <typename T>
concept ReceiveOutput = std::indirectly_writable<T, std::byte>;


// SYNC

template <typename T, typename Status, typename... Args>
concept SyncSendable = requires(const T SOCK, const std::byte *buf, std::size_t len, Args... args) {
    { SOCK.sync_send(buf, len, args...) } -> std::same_as<std::pair<std::size_t, Status>>;
};

template <typename T, typename Status, typename Out, typename... Args>
concept SyncReceivable = ReceiveOutput<Out> && requires(T sock, Out out, std::size_t len,
                                                        const std::size_t OFFSET, Args... args) {
    {
        sock.sync_receive(out, len, OFFSET, args...)
    } -> std::same_as<std::pair<std::size_t, Status>>;
};

// template <typename T, typename Status, typename Out, typename... Args>
// concept SyncReceivableWait = ReceiveOutput<Out> && requires(T sock, Out out, std::size_t len,
// bool wait, Args... args) {
//     { sock.sync_receive(out, len, wait, args...) } -> std::same_as<std::pair<std::size_t,
//     Status>>;
// };

template <typename T>
concept SyncClose = requires(T sock) {
    { sock.sync_close() } -> std::same_as<void>;
};

template <typename T>
concept SyncGetter = requires(T sock) {
    { sock.get_fd() } noexcept -> std::same_as<void>;
};

template <typename T, typename Status, typename... Args>
concept IoSyncSend = SyncSendable<T, Status, Args...> && SyncGetter<T> && SyncClose<T>;

template <typename T, typename Status, typename Out, typename... Args>
concept IoSyncReceive = SyncReceivable<T, Status, Out, Args...> && SyncGetter<T> && SyncClose<T>;

template <typename T, typename Status, typename Out, typename... Args>
concept IoSyncOps = SyncSendable<T, Status, Args...> && SyncReceivable<T, Status, Out, Args...> &&
                    SyncGetter<T> && SyncClose<T>;

// ASYNC

template <typename T, typename Status, typename... Args>
concept AsyncSendable =
    requires(T sock, const std::byte *buf, std::size_t len, IoCallback<Status> callback, Args... args) {
        { sock.async_send(buf, len, std::move(callback), args...) } noexcept -> std::same_as<void>;
    };

template <typename T, typename Status, typename Out, typename... Args>
concept AsyncReceivable = ReceiveOutput<Out> && requires(T sock, Out out, std::size_t len,
                                                         IoCallback<Status> callback, Args... args) {
    { sock.async_receive(out, len, std::move(callback), args...) } noexcept -> std::same_as<void>;
};

template <typename T>
concept AsyncClose = requires(T sock) {
    { sock.async_close() } noexcept -> std::same_as<void>;
};

template <typename T>
concept AsyncGetter = requires(T sock) {
    { sock.get_fd() } noexcept -> std::same_as<void>;
    { sock.attach() } noexcept -> std::same_as<void>;
    { sock.detach() } noexcept -> std::same_as<void>;
};

template <typename T, typename Status, typename... Args>
concept IoAsyncSend = AsyncSendable<T, Status, Args...> && AsyncGetter<T> && AsyncClose<T>;

template <typename T, typename Status, typename Out, typename... Args>
concept IoAsyncReceive =
    AsyncReceivable<T, Status, Out, Args...> && AsyncGetter<T> && AsyncClose<T>;

template <typename T, typename Status, typename Out, typename... Args>
concept IoAsyncOps = AsyncSendable<T, Status, Args...> &&
                     AsyncReceivable<T, Status, Out, Args...> && AsyncGetter<T> && AsyncClose<T>;

// Buf  – must satisfy SendBuffer  (e.g. std::array<std::byte, N>)
// Out  – must satisfy ReceiveOutput (e.g. std::byte*, std::back_insert_iterator)
// Args – trailing extras forwarded to every sub-concept (e.g. AddressInfo*, uint8_t)
template <typename T, typename Status, typename Buf, typename Out, typename... Args>
concept IoOps = AsyncSendable<T, Status, Args...> && SyncSendable<T, Status, Args...> &&
                SyncReceivable<T, Status, Out, Args...> &&
                // SyncReceivableWait<T, Status, Out, Args...> &&
                AsyncReceivable<T, Status, Out, Args...>;

} // namespace interfaces::io

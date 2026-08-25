export module shared:flow;

import std;
import utils_buffering;
import :handler;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace shared {

using ReadCallback = std::move_only_function<void(utils::buffering::BufferReader &)>;
using SendCallback = std::move_only_function<void(utils::buffering::BufferNode &&)>;
using CloseCallback = std::move_only_function<void()>;
using ErrorCallback = std::move_only_function<void(int, int)>;
using CompletionCallback = std::move_only_function<void(int)>;
using QueryReadFn = std::move_only_function<void(std::string_view)>;

template <typename T>
concept FlowLayer = requires(SendCallback send, CloseCallback close) {
    { T(std::move(send), std::move(close)) };
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

#ifdef CONGELADO_TEST
namespace shared::flow_tests {

// Satisfies FlowLayer — constructible from (SendCallback, CloseCallback), on_read() -> ReadCallback.
class MockFlowLayer {
  public:
    MockFlowLayer(SendCallback, CloseCallback) {}
    ReadCallback on_read() {
        return [](utils::buffering::BufferReader &) {};
    }
};

// Missing on_read() — should not satisfy FlowLayer.
class NotAFlowLayer {
  public:
    NotAFlowLayer(SendCallback, CloseCallback) {}
};

// Satisfies HandlerController for the FlowBase concept check.
class MockFlowController {
  public:
    void schedule(std::uint32_t) {}
    void deschedule(std::uint32_t) {}
    void release(std::uint32_t) {}

    struct Scheduled {
        void schedule() {}
        void deschedule() {}
        void release() {}
    };

    Scheduled create(std::string_view, WorkerFunction, ReleaseFunction, ErrorHandler) { return {}; }
};

class MockLeverager {};

// Satisfies FlowBase — constructible from (ReadCallback&&, Leverager&, Controller), on_send(int) -> SendCallback.
class MockFlowBase {
  public:
    MockFlowBase(ReadCallback &&, MockLeverager &, MockFlowController) {}
    SendCallback on_send(int) {
        return [](utils::buffering::BufferNode &&) {};
    }
};

using namespace boost::ut;

suite<"FlowLayer/FlowBase concepts"> flow_concepts_suite = [] {
    "MockFlowLayer satisfies FlowLayer"_test = [] {
        expect(FlowLayer<MockFlowLayer>);
    };

    "a type missing on_read() does not satisfy FlowLayer"_test = [] {
        expect(!FlowLayer<NotAFlowLayer>);
    };

    "MockFlowBase satisfies FlowBase over MockFlowController/MockLeverager"_test = [] {
        expect((FlowBase<MockFlowBase, MockFlowController, MockLeverager>));
    };
};

} // namespace shared::flow_tests
#endif

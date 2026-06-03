import std;
import congelado_worker;
#include <congelado/worker.h>

class EchoTask : public congelado::ITask {
  public:
    [[nodiscard]] std::string_view get_type() const noexcept override {
        return "echo_worker";
    }

    [[nodiscard]] congelado::TaskOutput run(congelado::TaskInput const &input) override {
        congelado::TaskOutput out{};
        if (auto msg = input.get<std::string>("message")) {
            out.set("echo", *msg);
        }
        return out;
    }
};

CONGELADO_TASK(EchoTask)

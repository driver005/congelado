import std;
import worker;
#include "worker/task_worker.h"

class EchoTask : public worker::ITaskWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override {
        return "echo_worker";
    }

    [[nodiscard]] worker::TaskOutput execute(worker::TaskInput const &input) override {
        worker::TaskOutput out{};
        if (auto msg = input.get<std::string>("message")) {
            out.set("echo", *msg);
        }
        return out;
    }
};

CONGELADO_TASK(EchoTask)

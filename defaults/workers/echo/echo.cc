#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>
import std;

namespace {

class EchoWorker final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "EchoWorker"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::string_view get_worker_type() const noexcept override { return "echo"; }

    [[nodiscard]] CongeladoConfigView execute_worker(
        const CongeladoConfigView *input) override {
        m_output = {};
        congelado::config_for_each(*input, [&](std::string_view key, std::string_view value) {
            m_output.add(std::string{key}, std::string{value});
        });
        return m_output.view();
    }

  private:
    congelado::ConfigViewBuilder m_output;
};

} // namespace

CONGELADO_PLUGIN(EchoWorker)

#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>
import std;

namespace {

class TransformWorker final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "TransformWorker"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::string_view get_worker_type() const noexcept override { return "transform"; }

    [[nodiscard]] CongeladoConfigView execute_worker(
        const CongeladoConfigView *input) override {
        m_output = {};

        auto transform = congelado::config_get(*input, "transform").value_or("identity");
        auto value = congelado::config_get(*input, "value").value_or("");

        std::string result;
        if (transform == "to_upper") {
            result = value;
            for (auto &c : result)
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else if (transform == "to_lower") {
            result = value;
            for (auto &c : result)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (transform == "negate") {
            auto val = std::strtod(value.c_str(), nullptr);
            result = std::format("{}", -val);
        } else {
            result = value;
        }

        m_output.add("result", std::move(result));
        return m_output.view();
    }

  private:
    congelado::ConfigViewBuilder m_output;
};

} // namespace

CONGELADO_PLUGIN(TransformWorker)

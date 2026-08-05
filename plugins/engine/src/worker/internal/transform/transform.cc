#include <congelado/worker.h>
#include <cctype>
#include <cstdlib>
#include <format>
#include <string>
#include <string_view>
#include <vector>

class TransformWorker {
  public:
    /**
     * @brief Task type this worker registers under, per `CONGELADO_TASK`'s duck-typed contract.
     * @return `"transform"`.
     */
    [[nodiscard]] static std::string_view get_worker_type() noexcept { return "transform"; }

    /**
     * @brief Applies a named string transform (`to_upper`, `to_lower`, `negate`, or an identity
     * passthrough for anything else) to the `value` field and hands the result back under
     * `result`.
     * @warning `negate` runs `value` through `std::strtod` — a non-numeric `value` doesn't error,
     * it just quietly becomes `0.0` (strtod's failure mode) and gets formatted out as `-0`. No
     * L raised, just a silently wrong answer if the caller sends garbage.
     * @param input the task's config input view; reads `transform` (defaults to `"identity"`)
     * and `value`.
     * @return a config view holding a single `result` entry with the transformed string.
     */
    CongeladoConfigView execute_worker(const CongeladoConfigView *input) {
        // Wipe whatever's left from a previous call before building this one's output fresh.
        m_output_keys.clear();
        m_output_values.clear();
        m_output_key_ptrs.clear();
        m_output_val_ptrs.clear();
        // Same reservation-before-push discipline as echo.cc — a mid-loop reallocation would
        // dangle any already-captured .c_str() pointers on SSO strings. Only one entry is
        // pushed today, but reserving keeps this safe if more output fields are added later.
        m_output_keys.reserve(1);
        m_output_values.reserve(1);
        m_output_key_ptrs.reserve(1);
        m_output_val_ptrs.reserve(1);

        std::string_view transform_type = "identity";
        std::string_view value;

        // Pull `transform` and `value` out of the input pairs — everything else gets ignored.
        for (std::size_t i = 0; i < input->count; ++i) {
            std::string_view key{input->keys[i]};
            if (key == "transform") {
                transform_type = input->values[i];
            } else if (key == "value") {
                value = input->values[i];
            }
        }

        // Dispatch on the requested transform — to_upper/to_lower mutate a copy in place,
        // negate parses+flips the sign numerically, anything else just passes value through.
        std::string result;
        if (transform_type == "to_upper") {
            result = value;
            for (auto &character : result) {
                character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
            }
        } else if (transform_type == "to_lower") {
            result = value;
            for (auto &character : result) {
                character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
        } else if (transform_type == "negate") {
            // value is a view into a null-terminated C string from CongeladoConfigView's const
            // char* array, so .data() is genuinely null-terminated here despite string_view not
            // guaranteeing it in general.
            // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
            auto val = std::strtod(value.data(), nullptr);
            result = std::format("{}", -val);
        } else {
            result = value;
        }

        // Only one output field today — stash it and hand back a view into this instance's
        // own storage, same pattern as echo.cc.
        m_output_keys.emplace_back("result");
        m_output_values.emplace_back(std::move(result));
        m_output_key_ptrs.push_back(m_output_keys.back().c_str());
        m_output_val_ptrs.push_back(m_output_values.back().c_str());

        return {.keys = m_output_key_ptrs.data(),
                .values = m_output_val_ptrs.data(),
                .count = m_output_key_ptrs.size()};
    }

  private:
    std::vector<std::string> m_output_keys;
    std::vector<std::string> m_output_values;
    std::vector<const char *> m_output_key_ptrs;
    std::vector<const char *> m_output_val_ptrs;
};

CONGELADO_TASK(TransformWorker)

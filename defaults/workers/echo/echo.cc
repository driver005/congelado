#include <congelado/worker.h>
#include <string>
#include <string_view>
#include <vector>

class EchoWorker {
  public:
    /**
     * @brief Task type this worker registers under, per `CONGELADO_TASK`'s duck-typed contract
     * (no virtuals here, just the shape the macro expects).
     * @return `"echo"`.
     */
    [[nodiscard]] static std::string_view get_worker_type() noexcept { return "echo"; }

    /**
     * @brief Copies every key/value pair from `input` straight back out unchanged — the
     * simplest possible worker, mostly good for smoke-testing the task pipeline end to end.
     * @note See the reserve-before-push comment below for why capacity gets reserved up front —
     * skipping that step would be a real L waiting to happen.
     * @param input the task's config key/value input view.
     * @return a config view mirroring `input` 1:1, backed by this instance's own storage.
     */
    CongeladoConfigView execute_worker(const CongeladoConfigView *input) {
        // Wipe whatever's left from a previous call — this instance's storage gets reused
        // across executions, not reallocated fresh each time.
        m_keys.clear();
        m_values.clear();
        m_key_ptrs.clear();
        m_val_ptrs.clear();
        // Reserve capacity for the whole batch up front — growing m_keys/m_values mid-loop
        // would relocate already-inserted (SSO) strings, dangling the .c_str() pointers
        // already pushed into m_key_ptrs/m_val_ptrs for earlier entries.
        m_keys.reserve(input->count);
        m_values.reserve(input->count);
        m_key_ptrs.reserve(input->count);
        m_val_ptrs.reserve(input->count);

        // Copy every key/value pair 1:1 into this instance's owned storage, then capture a
        // raw pointer into each copy for the returned view — straight mirror.
        for (std::size_t i = 0; i < input->count; ++i) {
            m_keys.emplace_back(input->keys[i]);
            m_values.emplace_back(input->values[i]);
            m_key_ptrs.push_back(m_keys.back().c_str());
            m_val_ptrs.push_back(m_values.back().c_str());
        }

        return {.keys = m_key_ptrs.data(), .values = m_val_ptrs.data(), .count = m_key_ptrs.size()};
    }

  private:
    std::vector<std::string> m_keys;
    std::vector<std::string> m_values;
    std::vector<const char *> m_key_ptrs;
    std::vector<const char *> m_val_ptrs;
};

CONGELADO_TASK(EchoWorker)

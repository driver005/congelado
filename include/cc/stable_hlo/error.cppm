export module cc_stable_hlo:error;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace cc::stable_hlo {

// A builder-op failure — shape mismatches, malformed attributes, anything that stops a
// StableHloBuilder op call from producing a value. Carried through std::expected instead
// of throwing, matching interfaces::worker.cppm's WorkerResult idiom.
class StableHloError {
  public:
    explicit StableHloError(std::string message) : m_message{std::move(message)} {}

    [[nodiscard]] const std::string &get_message() const noexcept { return m_message; }

  private:
    std::string m_message;
};

} // namespace cc::stable_hlo

#ifdef CONGELADO_TEST
namespace cc::stable_hlo::tests {
using namespace boost::ut;

suite<"StableHloError"> stable_hlo_error_suite = [] {
    "get_message returns exactly what the ctor stored"_test = [] {
        StableHloError error{"shape mismatch"};
        expect(error.get_message() == "shape mismatch");
    };
};

} // namespace cc::stable_hlo::tests
#endif

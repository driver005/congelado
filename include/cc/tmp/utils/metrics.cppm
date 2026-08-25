module;

#include <string>

export module cc_tmp:utils_metrics;

import std;
import cc_abi;

export {

namespace tensorflow {

class Metrics {
public:
    static void RecordCounter(const std::string& name, int64_t count) {
        // Dispatches through ice::Counter / ice::Meter
    }
};

} // namespace tensorflow

} // export

export module cc_abi_gen_writer:diff_result;

import std;

export namespace cc_abi_gen {

// Outcome of one DiffReporter::compare() call.
class DiffResult
{
public:
    bool m_identical = false;
    std::string m_unified_diff;
};

} // namespace cc_abi_gen

module;

export module model:audit;

import std;

export namespace model {

struct AuditRecord {
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::uint32_t version{0};
};

} // namespace model

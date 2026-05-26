module;

export module model:timestamps;

import std;

export namespace model {

struct ExecutionTimings {
    std::optional<std::chrono::system_clock::time_point> scheduled_at;
    std::optional<std::chrono::system_clock::time_point> started_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;
};

} // namespace model

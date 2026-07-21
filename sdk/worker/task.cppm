module;
#include <congelado/abi.h>
#include <cstdio>

export module congelado_worker:task;

import std;
import core_plugin;

export namespace congelado::worker {

class TaskInput {
  public:
    /**
     * @brief Wraps an existing string-keyed/string-valued map as read-only task input — no
     * copy, straight reference, zero extra allocs on construction.
     * @warning `data` is stored by reference (`m_data`), not copied. The caller's map must
     * outlive this `TaskInput`, otherwise every accessor is a dangling-reference read — that's
     * a straight UB footgun for anyone building one of these off a short-lived temporary.
     * @param data the backing key/value map this input reads from.
     */
    explicit TaskInput(std::unordered_map<std::string, std::string> const &data) noexcept
        : m_data(data) {}

    /// @brief Checks whether a key is present. @param key the key to look for. @return true if
    /// `key` exists in the input map.
    [[nodiscard]] bool has(std::string_view key) const noexcept {
        return m_data.contains(std::string(key));
    }

    /**
     * @brief Fetches and parses a single input value as `T` — the workhorse accessor every task
     * calls to pull typed config out of what's otherwise a flat string map.
     * @warning For `T = std::string_view`, the returned view aliases the backing map's string
     * storage. Since `m_data` is itself just a reference to the caller's map (see the ctor
     * warning), that view is only good for as long as the *original* map passed at construction
     * stays alive — outlive that and it's a dangling view, no cap.
     * @tparam T the type to parse into — one of `std::string`, `std::string_view`, `int`,
     * `std::int64_t`, `double`, or `bool`; anything else fails to compile via the `requires`
     * clause.
     * @param key the key to look up.
     * @return the parsed value, or `std::nullopt` if the key is missing or fails to parse as
     * `T` (numeric types use `std::from_chars`; bool only accepts the literal strings `"true"`
     * or `"false"`).
     */
    template <typename T>
        requires(std::same_as<T, std::string> || std::same_as<T, std::string_view> ||
                 std::same_as<T, int> || std::same_as<T, std::int64_t> || std::same_as<T, double> ||
                 std::same_as<T, bool>)
    [[nodiscard]] std::optional<T> get(std::string_view key) const {
        // Missing key is a plain nullopt, not an error — callers are expected to handle it.
        auto it = m_data.find(std::string(key));
        if (it == m_data.end()) {
            return std::nullopt;
        }
        auto const &string = it->second;

        // Dispatch on T at compile time — strings pass through as-is, numerics go through
        // from_chars, bool only accepts the exact literal strings.
        if constexpr (std::same_as<T, std::string>) {
            return string;
        } else if constexpr (std::same_as<T, std::string_view>) {
            // Lifetime: returned view is valid only while the source map passed to TaskInput lives.
            return std::string_view{string};
        } else if constexpr (std::same_as<T, int>) {
            int val{};
            auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, std::int64_t>) {
            std::int64_t val{};
            auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, double>) {
            double val{};
            auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else {
            // bool
            if (string == "true") {
                return true;
            }
            if (string == "false") {
                return false;
            }
            return std::nullopt;
        }
    }

    /// @brief Gets the whole backing map at once — the escape hatch for callers (like
    /// `detail::FfiWorker::execute`) that need to forward every key/value pair rather than pull
    /// them one at a time. @return a reference to the raw input map.
    [[nodiscard]] std::unordered_map<std::string, std::string> const &
    get_data_map() const noexcept {
        return m_data;
    }

  private:
    std::unordered_map<std::string, std::string> const &m_data;  // FIXME(clang-tidy): cppcoreguidelines-avoid-const-or-ref-data-members — deliberate: TaskInput is a zero-copy view over the caller's map (see ctor warning above), switching to a copy or pointer would change the documented lifetime contract
};

class TaskOutput {
  public:
    /// @brief Gets the whole output map built up by `set()` calls. @return a reference to the
    /// raw output map.
    [[nodiscard]] std::unordered_map<std::string, std::string> const &get_data() const noexcept {
        return m_data;
    }

    /**
     * @brief Stringifies `val` and stashes it under `key` — this is how task results get built
     * up before crossing back over the FFI boundary as a flat string map.
     * @tparam T the value's type — `std::string`/`std::string_view` are stored verbatim,
     * `bool` becomes the literal `"true"`/`"false"`, and everything else goes through
     * `std::format("{}", val)`.
     * @param key the output key to write under; overwrites any existing value at that key.
     * @param val the value being stringified and stored.
     */
    template <typename T>
    void set(std::string const &key, T const &val) {
        // Compile-time dispatch on T — strings stored verbatim, bool becomes a literal
        // "true"/"false", everything else falls back to std::format.
        if constexpr (std::same_as<T, std::string>) {
            m_data[key] = val;
        } else if constexpr (std::same_as<T, std::string_view>) {
            m_data[key] = std::string(val);
        } else if constexpr (std::same_as<T, bool>) {
            m_data[key] = val ? "true" : "false";
        } else {
            m_data[key] = std::format("{}", val);
        }
    }

  private:
    std::unordered_map<std::string, std::string> m_data;
};

class ITaskWorker {
  public:
    /// @brief Virtual dtor, default's fine — implementations don't own anything this base
    /// class needs to clean up.
    virtual ~ITaskWorker() = default;
    ITaskWorker() = default;
    ITaskWorker(const ITaskWorker &) = default;
    ITaskWorker &operator=(const ITaskWorker &) = default;
    ITaskWorker(ITaskWorker &&) = default;
    ITaskWorker &operator=(ITaskWorker &&) = default;
    /// @brief Tells the runner which task type this worker handles — used to route
    /// `TaskRunner::execute` calls to the right worker. @return the task type string.
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;
    /**
     * @brief Runs the task against the given input — the actual work, pure virtual so every
     * concrete worker has to implement it.
     * @param input the task's typed input accessor.
     * @return the task's output, built up via `TaskOutput::set`.
     */
    [[nodiscard]] virtual TaskOutput execute(TaskInput const &input) = 0;
    // Optional hooks — callers test truthiness (`if (on_released()) ...`) before invoking,
    // so "not overridden" must mean "no hook", not an exception.
    /**
     * @brief Optional cleanup hook fired after `execute` completes (success or failure).
     * @note Default returns an empty `std::function` — callers test its truthiness before
     * invoking, so leaving this unoverridden cleanly means "no hook", never a null-call crash.
     * @return a callable to run post-execution, or an empty (falsy) `std::function` if this
     * worker has no cleanup to do.
     */
    [[nodiscard]] virtual std::function<void()> on_released() { return {}; }
    /**
     * @brief Optional error hook fired when `execute` throws.
     * @note Same falsy-by-default deal as `on_released` — an unoverridden worker just means
     * "nothing to run on error", checked before the call, not assumed.
     * @return a callable taking the caught `std::exception_ptr`, or an empty (falsy)
     * `std::function` if this worker has no error handling to do.
     */
    [[nodiscard]] virtual std::function<void(std::exception_ptr)> on_error() { return {}; }
};

} // namespace congelado::worker

namespace congelado::worker::detail {

using WorkerExecuteFn = CongeladoConfigView (*)(const CongeladoConfigView *);

// Adapts a CONGELADO_TASK-ABI shared library symbol pair into an ITaskWorker.
class FfiWorker final : public ITaskWorker {
  public:
    /**
     * @brief Wraps a `CONGELADO_TASK`-ABI plugin's exported type name and execute symbol as an
     * `ITaskWorker` — the bridge from raw dlsym'd C function pointers to the normal C++ worker
     * interface.
     * @param type the task type this worker handles, dlsym'd from `congelado_worker_type`.
     * @param exec the raw C execute function, dlsym'd from `congelado_worker_execute`.
     */
    FfiWorker(std::string type, WorkerExecuteFn exec) : m_type{std::move(type)}, m_exec{exec} {}

    /// @brief Gets the task type this worker handles. @return the task type string.
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_type; }

    /**
     * @brief Flattens `input` into a `CongeladoConfigView`, calls across the FFI boundary into
     * the plugin's C execute symbol, then rebuilds the raw C result back into a `TaskOutput` —
     * this is the whole marshal/unmarshal dance for every FFI-loaded task, no cap.
     * @param input the task's typed input, converted to a flat key/value view for the C call.
     * @return the task's output, rebuilt from the plugin's raw `CongeladoConfigView` result.
     */
    [[nodiscard]] TaskOutput execute(TaskInput const &input) override {
        const auto &data = input.get_data_map();

        // Flatten the input map into parallel key/value arrays of raw C string pointers —
        // this is the shape CongeladoConfigView needs to cross the FFI boundary.
        std::vector<std::string> keys;
        std::vector<std::string> values;
        std::vector<const char *> key_ptrs;
        std::vector<const char *> val_ptrs;
        keys.reserve(data.size());
        values.reserve(data.size());
        key_ptrs.reserve(data.size());
        val_ptrs.reserve(data.size());

        for (const auto &[key, value] : data) {
            keys.push_back(key);
            values.push_back(value);
            key_ptrs.push_back(keys.back().c_str());
            val_ptrs.push_back(values.back().c_str());
        }

        // Cross into the plugin's C execute symbol.
        CongeladoConfigView in{.keys = key_ptrs.data(), .values = val_ptrs.data(), .count = keys.size()};
        CongeladoConfigView out = m_exec(&in);

        // Rebuild the raw C result back into a normal TaskOutput for the rest of the host.
        TaskOutput result;
        for (std::size_t i = 0; i < out.count; ++i) {
            result.set(std::string{out.keys[i]}, std::string{out.values[i]});
        }

        return result;
    }

  private:
    std::string m_type;
    WorkerExecuteFn m_exec;
};

} // namespace congelado::worker::detail

export namespace congelado::worker {

// Host-side loading and execution of task plugins (the CONGELADO_TASK ABI, e.g.
// defaults/workers/echo, defaults/workers/transform) — fully self-contained, no HTTP
// surface. Owns both the dlopen/FFI-symbol-resolution machinery and the task-type
// registry directly (no wrapped/external "worker" module dependency).
class TaskRunner {
  public:
    /**
     * @brief Builds an empty TaskRunner with no workers registered yet.
     * @param workerId this runner's identifier, empty by default.
     */
    explicit TaskRunner(std::string_view workerId = {}) : m_worker_id{workerId} {}

    /**
     * @brief Fires every registered worker's `on_released()` cleanup hook (if it has one) on
     * teardown — a last sweep so no worker gets skipped just because it was never executed.
     */
    ~TaskRunner() {
        // Sweep every registered worker and fire its cleanup hook, if it has one — this
        // catches workers that were registered but never actually executed.
        for (auto *entry : m_workers) {
            if (auto release = entry->on_released()) {
                release();
            }
        }
    }

    /// @brief Deleted — a TaskRunner owns raw worker pointers and dlopen'd shared libraries, no
    /// copying that motion.
    TaskRunner(TaskRunner const &) = delete;
    /// @brief Deleted — same reason as the copy ctor.
    TaskRunner &operator=(TaskRunner const &) = delete;
    /// @brief Deleted — same reason as the copy ctor, moving out from under live worker
    /// pointers isn't safe either.
    TaskRunner(TaskRunner &&) = delete;
    /// @brief Deleted — same reason as the move ctor.
    TaskRunner &operator=(TaskRunner &&) = delete;

    /// @brief Sets this runner's identifier. @param workerId the new worker ID.
    void setWorkerId(std::string_view workerId) { m_worker_id = workerId; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after the prefix), not snake_case

    /**
     * @brief Scans `directory` for `CONGELADO_TASK`-ABI shared libraries, opens every one it
     * finds, and registers a `detail::FfiWorker` for each that actually exports a non-empty
     * worker type and execute symbol — this is the real bulk-loading entrypoint most worker
     * hosts call once at startup.
     * @warning Aborts the process (`std::abort()`) if any discovered shared library fails to
     * open — this is a boot-time hard-fail, not a per-file skip.
     * @param directory filesystem directory to scan for worker `.so`s.
     */
    void load_workers(const std::filesystem::path &directory) {
        // Scan the directory and open every discovered shared library — a hard boot-time
        // fail if any of them can't be opened, no partial-load recovery here.
        auto directory_str = directory.string();
        m_store.scan(directory_str);
        auto res = m_store.open_all();
        if (!res) {
            std::println(stderr, "[congelado_worker] failed to load workers from '{}': {}",
                         directory_str, res.error().get_message());
            std::abort();
        }

        // For every opened .so, only register it as a worker if it actually looks like a
        // CONGELADO_TASK plugin — has a non-empty worker type and an execute symbol. Anything
        // missing either just gets skipped, no abort.
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto type_it = plugin->m_data.find("congelado_worker_type");
            if (type_it == plugin->m_data.end()) {
                return;
            }
            const auto &type = std::any_cast<const std::string &>(type_it->second);
            if (type.empty()) {
                return;
            }

            auto exec_it = plugin->m_data.find("congelado_worker_execute");
            if (exec_it == plugin->m_data.end()) {
                return;
            }
            auto *raw = std::any_cast<void *>(exec_it->second);
            auto exec_fn = reinterpret_cast<detail::WorkerExecuteFn>(raw);  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type, no smart-pointer/GSL equivalent applies

            // Both pieces resolved — build the FfiWorker, register it, and keep it alive in
            // m_loaded_workers since addTaskWorker only stores a non-owning pointer.
            auto worker = std::make_unique<detail::FfiWorker>(type, exec_fn);
            addTaskWorker(worker.get());
            m_loaded_workers.push_back(std::move(worker));
        });
    }

    /**
     * @brief Registers a worker instance, replacing any existing worker already registered for
     * the same task type — last one in wins, no duplicate-type entries stick around.
     * @warning `worker` is stored as a raw non-owning pointer — the caller (or whoever owns it,
     * e.g. `load_workers`'s `m_loaded_workers`) must keep it alive for as long as this
     * `TaskRunner` might call `execute` against it. Register a worker that goes out of scope
     * and it's dangling-pointer UB the next time it's looked up, straight L.
     * @param worker the worker instance to register.
     */
    void addTaskWorker(ITaskWorker *worker) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention
        // Same task type already registered — swap it in place rather than stacking dupes.
        for (auto &entry : m_workers) {
            if (entry->get_task_type() == worker->get_task_type()) {
                entry = worker;
                return;
            }
        }
        // No existing entry for this type — bet, it's a new one, append it.
        m_workers.push_back(worker);
    }

    /**
     * @brief Looks up the worker for `taskType` and runs it against `input`, firing
     * `on_error`/`on_released` hooks around the call as appropriate — the main runtime
     * entrypoint for actually dispatching a task.
     * @note Catches every exception `execute` throws: fires the worker's `on_error()` hook (if
     * it has one) with the caught `std::exception_ptr`, still fires `on_released()` either way,
     * and returns `std::nullopt` instead of letting the exception propagate.
     * @param taskType the task type to look up and dispatch to.
     * @param input the task's typed input.
     * @return the task's output on success, or `std::nullopt` if no worker is registered for
     * `taskType` or if the worker's `execute` threw.
     */
    [[nodiscard]] std::optional<TaskOutput> execute(std::string_view taskType,
                                                     const TaskInput &input) const {
        // No worker registered for this type — nothing to dispatch to.
        auto *worker = getTaskWorker(taskType);
        if (worker == nullptr) {
            return std::nullopt;
        }
        auto release = worker->on_released();
        auto on_error = worker->on_error();
        try {
            // Happy path — run the task, fire cleanup, hand back the output.
            auto output = worker->execute(input);
            if (release) {
                release();
            }
            return output;
        } catch (...) {
            // execute() threw — fire the error hook first, still fire cleanup either way,
            // then swallow the exception and report failure via nullopt instead.
            if (on_error) {
                on_error(std::current_exception());
            }
            if (release) {
                release();
            }
            return std::nullopt;
        }
    }

    /// @brief Finds the registered worker for a task type, linear scan over `m_workers`.
    /// @param taskType the task type to look for.
    /// @return a pointer to the matching worker, or `nullptr` if none is registered.
    [[nodiscard]] ITaskWorker *getTaskWorker(std::string_view taskType) const noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention
        for (auto *entry : m_workers) {
            if (entry->get_task_type() == taskType) {
                return entry;
            }
        }
        return nullptr;
    }

    /// @brief Checks whether a worker is registered for the given task type. @param taskType
    /// the task type to check. @return true if a matching worker is registered.
    [[nodiscard]] bool has_task_type(std::string_view taskType) const noexcept {
        return getTaskWorker(taskType) != nullptr;
    }

    /// @brief Lists every registered worker's task type. @return the task types, in
    /// registration order.
    [[nodiscard]] std::vector<std::string_view> getTaskTypes() const noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention
        std::vector<std::string_view> types;
        types.reserve(m_workers.size());
        for (auto *entry : m_workers) {
            types.push_back(entry->get_task_type());
        }
        return types;
    }

    /// @brief Gets this runner's identifier. @return the worker ID string.
    [[nodiscard]] std::string_view getWorkerId() const noexcept { return m_worker_id; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention

  private:
    std::string m_worker_id;
    std::vector<ITaskWorker *> m_workers;
    std::vector<std::unique_ptr<ITaskWorker>> m_loaded_workers;
    core::plugin::SharedLibrary m_store{"worker"};
};

} // namespace congelado::worker

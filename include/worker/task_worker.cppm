export module worker:task_worker;

import std;

export namespace worker {

using CongeladoTaskFactory = void *(*)();

class TaskInput {
  public:
    explicit TaskInput(std::unordered_map<std::string, std::string> const &data) noexcept
        : m_data(data) {}

    [[nodiscard]] bool has(std::string_view key) const noexcept {
        return m_data.contains(std::string(key));
    }

    template <typename T>
        requires(std::same_as<T, std::string> || std::same_as<T, std::string_view> ||
                 std::same_as<T, int> || std::same_as<T, std::int64_t> ||
                 std::same_as<T, double> || std::same_as<T, bool>)
    [[nodiscard]] std::optional<T> get(std::string_view key) const {
        auto it = m_data.find(std::string(key));
        if (it == m_data.end()) return std::nullopt;
        auto const &s = it->second;

        if constexpr (std::same_as<T, std::string>) {
            return s;
        } else if constexpr (std::same_as<T, std::string_view>) {
            // Lifetime: returned view is valid only while the source map passed to TaskInput lives.
            return std::string_view{s};
        } else if constexpr (std::same_as<T, int>) {
            int val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, std::int64_t>) {
            std::int64_t val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, double>) {
            double val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else {
            // bool
            if (s == "true") return true;
            if (s == "false") return false;
            return std::nullopt;
        }
    }

    [[nodiscard]] std::unordered_map<std::string, std::string> const &data_map() const noexcept {
        return m_data;
    }

  private:
    std::unordered_map<std::string, std::string> const &m_data;
};

class TaskOutput {
  public:
    [[nodiscard]] std::unordered_map<std::string, std::string> const &data() const noexcept {
        return m_data;
    }

    template <typename T>
    void set(std::string const &key, T const &val) {
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
    virtual ~ITaskWorker() = default;
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;
    [[nodiscard]] virtual TaskOutput execute(TaskInput const &input) = 0;
};

} // namespace worker

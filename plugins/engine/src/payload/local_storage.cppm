export module engine:local_payload_storage;

import std;
import interfaces;
import shared;
import core_events;
import core_logger;

export namespace engine {

/// @brief Default `IExternalPayloadStorage` — writes each payload to its own file under a
/// configured directory, named by a fresh UUID; the file path itself is the returned reference.
/// Always available (no capability-plugin resolution, unlike ISearchProvider/IDatabase) since
/// it's plain local disk I/O, not an external service connection — an optional S3-backed
/// alternative would need that machinery, this one doesn't.
class LocalPayloadStorage : public interfaces::IExternalPayloadStorage {
  public:
    /**
     * @brief Builds a storage instance rooted at `directory`, creating it if it doesn't exist.
     * @param directory the directory every payload file gets written under.
     */
    explicit LocalPayloadStorage(std::filesystem::path directory) : m_dir{std::move(directory)} {
        std::error_code error_code;
        std::filesystem::create_directories(m_dir, error_code);
        if (error_code) {
            core::logger::warning("engine", "payload storage: could not create '{}': {}",
                                  m_dir.string(), error_code.message());
            core::events::publish("engine.payload_storage.create_directory_failed",
                                  {{"directory", m_dir.string()}, {"error", error_code.message()}});
        }
    }

    /// @brief Out-of-line (not `= default` in-class) so this class has a key function — pins the
    /// vtable's emission to exactly this TU instead of leaving it to vague-linkage dedup, which
    /// clang's C++20-modules handling got inconsistent about across module-partition boundaries
    /// (weakly emitted the destructor at the one construction site in engine.cc, but not the
    /// vtable or constructor — undefined symbol at dlopen time).
    ~LocalPayloadStorage() override;

    /**
     * @brief Writes `data` to a fresh file under this storage's directory.
     * @param type unused — every payload type lands in the same flat directory; a real
     * multi-backend implementation might partition by type, this one doesn't need to.
     * @param data the raw payload bytes to write.
     * @param callback gets the written file's path on success, `""` on failure.
     */
    void write(interfaces::PayloadType /*type*/, std::string_view data,
               shared::QueryReadFn &&callback) noexcept override {
        auto path = m_dir / std::format("{}.payload", generate_uuid());
        std::error_code error_code;
        std::ofstream out{path, std::ios::binary};
        if (!out) {
            core::logger::warning("engine", "payload storage: could not open '{}' for write",
                                  path.string());
            core::events::publish("engine.payload_storage.open_write_failed",
                                  {{"path", path.string()}});
            callback("");
            return;
        }
        out.write(data.data(), static_cast<std::streamsize>(data.size()));
        out.close();
        callback(path.string());
    }

    /**
     * @brief Reads back a payload previously written by write().
     * @param reference the file path write() returned.
     * @param callback gets the file's contents on success, `""` if it can't be opened.
     */
    void read(std::string_view reference, shared::QueryReadFn &&callback) noexcept override {
        std::ifstream in{std::filesystem::path{reference}, std::ios::binary};
        if (!in) {
            core::logger::warning("engine", "payload storage: could not open '{}' for read",
                                  reference);
            core::events::publish("engine.payload_storage.open_read_failed",
                                  {{"reference", std::string{reference}}});
            callback("");
            return;
        }
        std::string contents{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
        callback(contents);
    }

  private:
    std::filesystem::path m_dir;

    /// @brief A random-enough filename component — not a real UUID library, just a fast,
    /// dependency-free way to avoid filename collisions for this local, single-process use.
    [[nodiscard]] static std::string generate_uuid() {
        static std::atomic<std::uint64_t> counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return std::format("{:x}-{:x}", now, counter.fetch_add(1, std::memory_order_relaxed));
    }
};

LocalPayloadStorage::~LocalPayloadStorage() = default;

} // namespace engine

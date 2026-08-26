export module payload_local;

import std;
import interfaces;
import shared;
import core_events;
import core_logger;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace payload_local {

/// @brief Local-disk `IExternalPayloadStorage` — writes each payload to its own file under a
/// configured directory, named by a fresh id; the file path itself is the returned reference.
/// Plain local disk I/O, no external service connection — the default payload backend, resolved
/// as the `payload_storage` capability. The directory is set from the plugin's config in
/// on_load (defaults to "payloads"); the storage instance itself exists at plugin construction
/// so the host can resolve its pointer before build() (the resolve-before-build contract every
/// capability follows).
class LocalPayloadStorage : public interfaces::IExternalPayloadStorage
{
public:
    /// @brief Default ctor — roots at "payloads" until set_directory() re-points it.
    LocalPayloadStorage()
    {
        set_directory("payloads");
    }

    /// @brief Out-of-line dtor (a key function) so the vtable is emitted in this module TU
    /// rather than left to vague-linkage dedup — clang's C++20-modules handling is inconsistent
    /// about emitting it across a construction site in the plugin .cc, causing undefined-symbol
    /// at dlopen.
    ~LocalPayloadStorage() override;

    /// @brief Re-roots the storage at `directory`, creating it if needed. Called from on_load.
    /// @param directory the directory every payload file gets written under.
    void set_directory(std::filesystem::path directory)
    {
        m_dir = std::move(directory);
        std::error_code error_code;
        std::filesystem::create_directories(m_dir, error_code);
        if (error_code) {
            core::logger::warning(
                "payload.local", "could not create '{}': {}", m_dir.string(), error_code.message()
            );
            core::events::publish(
                "payload.local.create_directory_failed",
                {{"directory", m_dir.string()}, {"error", error_code.message()}}
            );
        }
    }

    /**
     * @brief Writes `data` to a fresh file under this storage's directory.
     * @param type unused — every payload type lands in the same flat directory.
     * @param data the raw payload bytes to write.
     * @param callback gets the written file's path on success, `""` on failure.
     */
    void write(
        interfaces::PayloadType /*type*/, std::string_view data, shared::QueryReadFn&& callback
    ) noexcept override
    {
        auto path = m_dir / std::format("{}.payload", generate_id());
        std::error_code error_code;
        std::ofstream out{path, std::ios::binary};
        if (!out) {
            core::logger::warning("payload.local", "could not open '{}' for write", path.string());
            core::events::publish("payload.local.open_write_failed", {{"path", path.string()}});
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
    // SECURITY: path traversal / arbitrary file read. `reference` is opened as-is with zero
    // validation — it isn't confined to `m_dir`, isn't checked for `..` segments, and an
    // absolute path is honored just as readily as a relative one. write() only ever hands back
    // references it generated itself (m_dir / "<id>.payload"), so this is safe today only
    // because nothing yet feeds `read()` anything else — per the IExternalPayloadStorage class
    // doc, this interface "is NOT yet wired into any actual input/output write path." The
    // moment a `reference` is derived from anything caller/attacker-influenced (a payload key
    // echoed back through an API, a row a caller can edit, etc.) this becomes a straight
    // arbitrary-file read off whatever this process can access. Needs the resolved path
    // canonicalized and checked to still be lexically inside `m_dir` before opening it.
    void read(std::string_view reference, shared::QueryReadFn&& callback) noexcept override
    {
        std::ifstream in{std::filesystem::path{reference}, std::ios::binary};
        if (!in) {
            core::logger::warning("payload.local", "could not open '{}' for read", reference);
            core::events::publish(
                "payload.local.open_read_failed", {{"reference", std::string{reference}}}
            );
            callback("");
            return;
        }
        std::string contents{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
        callback(contents);
    }

private:
    std::filesystem::path m_dir;

    /// @brief A random-enough filename component — dependency-free collision avoidance for this
    /// local, single-process use.
    [[nodiscard]] static std::string generate_id()
    {
        static std::atomic<std::uint64_t> counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return std::format("{:x}-{:x}", now, counter.fetch_add(1, std::memory_order_relaxed));
    }
};

LocalPayloadStorage::~LocalPayloadStorage() = default;

} // namespace payload_local

#ifdef CONGELADO_TEST
namespace payload_local_tests {
using namespace boost::ut;
using payload_local::LocalPayloadStorage;

/// @brief A fresh, uniquely-named sandbox directory under the system temp dir — every test
/// below roots its `LocalPayloadStorage` here (never at the class's own "payloads" default),
/// and everything created under it is torn down when the guard goes out of scope. Keeps every
/// test, including the path-traversal ones, entirely inside a test-controlled temp dir instead
/// of ever touching a real filesystem path outside it.
class TempSandbox
{
public:
    TempSandbox()
    {
        static std::atomic<std::uint64_t> counter{0};
        auto unique = std::format(
            "congelado_payload_local_test_{:x}_{:x}",
            std::chrono::steady_clock::now().time_since_epoch().count(),
            counter.fetch_add(1, std::memory_order_relaxed)
        );
        m_root = std::filesystem::temp_directory_path() / unique;
        std::filesystem::create_directories(m_root);
    }

    ~TempSandbox()
    {
        std::error_code error_code;
        std::filesystem::remove_all(m_root, error_code);
    }

    TempSandbox(const TempSandbox&) = delete;
    TempSandbox& operator=(const TempSandbox&) = delete;

    [[nodiscard]] std::filesystem::path get_root() const
    {
        return m_root;
    }

private:
    std::filesystem::path m_root;
};

/// @brief Reads the whole contents of `path` via the same idiom write() below uses to build
/// out-of-sandbox fixture files.
void write_file(const std::filesystem::path& path, std::string_view contents)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out{path, std::ios::binary};
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

suite<"LocalPayloadStorage::set_directory"> local_payload_set_directory_suite = [] {
    "creates the configured directory if it doesn't exist yet"_test = [] {
        TempSandbox sandbox;
        auto target = sandbox.get_root() / "fresh" / "nested";
        LocalPayloadStorage storage;

        storage.set_directory(target);

        expect(std::filesystem::exists(target));
        expect(std::filesystem::is_directory(target));
    };
};

suite<"LocalPayloadStorage::write/read"> local_payload_write_read_suite = [] {
    "write() then read() round-trips the exact bytes"_test = [] {
        TempSandbox sandbox;
        LocalPayloadStorage storage;
        storage.set_directory(sandbox.get_root());

        std::string written_reference;
        storage.write(
            interfaces::PayloadType::TASK_OUTPUT, "hello payload", [&](std::string_view reference) {
                written_reference = std::string{reference};
            }
        );

        expect(!written_reference.empty()) << fatal;
        expect(written_reference.starts_with(sandbox.get_root().string()));
        expect(written_reference.ends_with(".payload"));

        std::string read_back;
        storage.read(written_reference, [&](std::string_view data) {
            read_back = std::string{data};
        });

        expect(read_back == "hello payload");
    };

    "write() with empty data still succeeds and round-trips an empty payload"_test = [] {
        TempSandbox sandbox;
        LocalPayloadStorage storage;
        storage.set_directory(sandbox.get_root());

        std::string written_reference = "unset";
        storage.write(interfaces::PayloadType::WORKFLOW_INPUT, "", [&](std::string_view reference) {
            written_reference = std::string{reference};
        });

        expect(written_reference != "unset") << fatal;
        expect(!written_reference.empty());

        std::string read_back = "unset";
        storage.read(written_reference, [&](std::string_view data) {
            read_back = std::string{data};
        });

        expect(read_back.empty());
    };

    "two consecutive write() calls land in two distinct files, never colliding"_test = [] {
        TempSandbox sandbox;
        LocalPayloadStorage storage;
        storage.set_directory(sandbox.get_root());

        std::string first_reference;
        std::string second_reference;
        storage.write(
            interfaces::PayloadType::TASK_INPUT, "first", [&](std::string_view reference) {
                first_reference = std::string{reference};
            }
        );
        storage.write(
            interfaces::PayloadType::TASK_INPUT, "second", [&](std::string_view reference) {
                second_reference = std::string{reference};
            }
        );

        expect(!first_reference.empty()) << fatal;
        expect(!second_reference.empty()) << fatal;
        expect(first_reference != second_reference);

        std::string first_read;
        std::string second_read;
        storage.read(first_reference, [&](std::string_view data) {
            first_read = std::string{data};
        });
        storage.read(second_reference, [&](std::string_view data) {
            second_read = std::string{data};
        });

        expect(first_read == "first");
        expect(second_read == "second");
    };

    "read() on a reference that was never written reports failure via an empty callback string"_test =
        [] {
            TempSandbox sandbox;
            LocalPayloadStorage storage;
            storage.set_directory(sandbox.get_root());

            std::string read_back = "unset";
            storage.read(
                (sandbox.get_root() / "never-written.payload").string(),
                [&](std::string_view data) {
                    read_back = std::string{data};
                }
            );

            expect(read_back.empty());
        };

    "write() into a directory that couldn't be created (blocked by a same-named file) fails via an empty callback string"_test =
        [] {
            TempSandbox sandbox;
            auto blocked_path = sandbox.get_root() / "blocked";
            write_file(blocked_path, "this is a file, not a directory");

            LocalPayloadStorage storage;
            storage.set_directory(blocked_path);

            std::string written_reference = "unset";
            storage.write(
                interfaces::PayloadType::TASK_OUTPUT, "data", [&](std::string_view reference) {
                    written_reference = std::string{reference};
                }
            );

            expect(written_reference.empty());
        };
};

// SECURITY: pins the finding in the SECURITY comment above read() — the reference string is
// opened as a literal filesystem path with no confinement to the configured storage directory
// whatsoever. Both cases below stay entirely inside a TempSandbox (never a real out-of-sandbox
// path like /etc/passwd) but demonstrate the exact same escape a hostile reference would
// achieve against a real deployment's payload directory.
suite<"LocalPayloadStorage::read path confinement (SECURITY)"> local_payload_traversal_suite = [] {
    "an absolute-path reference is honored outright, reading a file completely outside the configured storage directory"_test =
        [] {
            TempSandbox sandbox;
            auto storage_dir = sandbox.get_root() / "storage";
            auto outside_file = sandbox.get_root() / "elsewhere" / "secret.txt";
            write_file(outside_file, "TOP-SECRET-OUTSIDE-STORAGE-DIR");

            LocalPayloadStorage storage;
            storage.set_directory(storage_dir);

            // A confined implementation would refuse a reference that resolves outside
            // storage_dir (empty result); this one happily hands back the file's contents.
            std::string read_back;
            storage.read(outside_file.string(), [&](std::string_view data) {
                read_back = std::string{data};
            });

            expect(read_back == "TOP-SECRET-OUTSIDE-STORAGE-DIR");
        };

    "a relative '../' reference resolves against the process's current directory, not the configured storage directory — read() never joins the reference with m_dir at all"_test =
        [] {
            TempSandbox sandbox;
            auto storage_dir = sandbox.get_root() / "storage";
            auto cwd_dir = sandbox.get_root() / "cwd";
            std::filesystem::create_directories(cwd_dir);
            auto outside_file = sandbox.get_root() / "outside" / "secret.txt";
            write_file(outside_file, "TOP-SECRET-VIA-RELATIVE-ESCAPE");

            LocalPayloadStorage storage;
            storage.set_directory(storage_dir);

            // Scoped, restored immediately after the single synchronous read() call below —
            // boost::ut runs suites sequentially on one thread, so no other test observes the
            // changed directory.
            auto previous_cwd = std::filesystem::current_path();
            std::filesystem::current_path(cwd_dir);
            std::string read_back;
            storage.read("../outside/secret.txt", [&](std::string_view data) {
                read_back = std::string{data};
            });
            std::filesystem::current_path(previous_cwd);

            expect(read_back == "TOP-SECRET-VIA-RELATIVE-ESCAPE");
        };
};

} // namespace payload_local_tests
#endif

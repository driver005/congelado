module;
#ifdef CONGELADO_TEST
// Test-only: Document::at() walks a serde::Value (== rfl::Generic) tree, and no JSON format
// plugin is registered in this isolated test target, so Document::parse()/load() can never
// produce one to navigate. Building fixtures directly via rfl::Generic (same pattern
// plugins/serde/json/bin/json_plugin.cc's own tests already use) is the only way to exercise
// at() here; guarded so production builds never see this include (this file otherwise
// deliberately never touches rfl headers directly, see the note below).
#include <rfl/Generic.hpp>
#endif

export module openapi_generator_plugin:document;

import std;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// Navigates a parsed OpenAPI document as a dynamic value tree. The actual JSON bytes-to-
// serde::Value step goes through serde::Ser::decode_generic (the same JSON format plugin every
// other consumer dispatches through — build.cc bootstraps it before this ever runs), so this
// class no longer duplicates plugins/json/json_plugin.cc's own rfl::json::read call; it just
// adds the file-read + chained-key-lookup motions the codegen tool actually needs on top. Only
// `serde` itself (plus the two format plugins, which implement the ABI contract) touches rfl
// headers directly — this file reaches the reflected-value type through serde::Value instead.

export namespace congelado::client {

class Document {
  public:
    /**
     * @brief Parses a raw JSON string into a dynamic value tree via the registered JSON format
     * plugin — no target type needed, this is for navigating JSON whose shape you don't know
     * ahead of time.
     * @param data the raw JSON text to parse.
     * @return the parsed value tree, or an error message if `data` isn't valid JSON or no JSON
     * format plugin is loaded.
     */
    [[nodiscard]] static std::expected<serde::Value, std::string> parse(std::string_view data) {
        return serde::Ser::decode_generic("application/json", data);
    }

    /**
     * @brief Loads and parses a JSON file from disk into a dynamic value tree.
     * @param path the filesystem path to read.
     * @return the parsed value tree, or an error message (path + underlying parse failure) if
     * the file's missing, unreadable, or isn't valid JSON.
     */
    [[nodiscard]] static std::expected<serde::Value, std::string>
    load(const std::filesystem::path &path) {
        std::ifstream file{path};
        if (!file) {
            return std::unexpected{std::format("failed to open '{}'", path.string())};
        }
        std::string contents{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        auto result = parse(contents);
        if (!result) {
            return std::unexpected{std::format("failed to parse '{}': {}", path.string(), result.error())};
        }
        return *result;
    }

    /**
     * @brief Chained object-key lookup — no operator[] chaining of its own, so this is the
     * motion for reaching into nested objects without a null check per hop.
     * @param value the root value to start the walk from.
     * @param keys the sequence of object keys to descend through, in order.
     * @return the value found at the end of the key chain, or `nullopt` the moment any hop
     * isn't an object or is missing the next key — bails clean, no exception.
     */
    [[nodiscard]] static std::optional<serde::Value>
    at(const serde::Value &value, std::initializer_list<std::string_view> keys) {
        serde::Value current = value;
        for (auto key : keys) {
            auto object = current.to_object();
            if (!object) {
                return std::nullopt;
            }
            auto found = object->get(std::string{key});
            if (!found) {
                return std::nullopt;
            }
            current = *found;
        }
        return current;
    }
};

} // namespace congelado::client

#ifdef CONGELADO_TEST
namespace openapi_gen_document_tests {
using namespace boost::ut;
using congelado::client::Document;

suite<"Document"> document_suite = [] {
    "parse() always errors in this isolated test target — no JSON format plugin is linked"_test = [] {
        // See the module-preamble note above: this test target never links a format plugin
        // (json_plugin is a whole separate runtime-loadable target), so
        // serde::Ser::decode_generic()'s registry lookup is guaranteed empty here — this is a
        // deterministic property of the test environment, not something parse() itself does.
        auto result = Document::parse(R"({"a": 1})");

        expect(not result.has_value()) << fatal;
        expect(result.error() == "no format plugin loaded for 'application/json'");
    };

    "load() on a nonexistent path fails to open"_test = [] {
        auto result = Document::load("/nonexistent/path/does/not/exist.json");

        expect(not result.has_value()) << fatal;
        expect(result.error().contains("failed to open"));
    };

    "load() on an existing file still fails, at the parse step (no format plugin)"_test = [] {
        auto path = std::filesystem::temp_directory_path() / "congelado_document_test_load.json";
        {
            std::ofstream out{path};
            out << R"({"a": 1})";
        }

        auto result = Document::load(path);

        expect(not result.has_value()) << fatal;
        expect(result.error().contains("failed to parse"));
        expect(result.error().contains("no format plugin loaded for 'application/json'"));

        std::filesystem::remove(path);
    };

    "at() walks a chained key lookup down to a leaf value"_test = [] {
        serde::Value::Object leaf_holder;
        leaf_holder.insert(std::string{"schema"}, serde::Value{std::string{"leaf-value"}});
        serde::Value::Object json_holder;
        json_holder.insert(std::string{"application/json"}, serde::Value{leaf_holder});
        serde::Value::Object content_holder;
        content_holder.insert(std::string{"content"}, serde::Value{json_holder});

        auto found = Document::at(serde::Value{content_holder},
                                  {"content", "application/json", "schema"});

        expect(found.has_value()) << fatal;
        auto leaf = found->to_string();
        expect(leaf.has_value()) << fatal;
        expect(*leaf == "leaf-value");
    };

    "at() returns nullopt when a key is missing partway through"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"a"}, serde::Value{std::string{"x"}});

        auto found = Document::at(serde::Value{object}, {"b"});

        expect(not found.has_value());
    };

    "at() returns nullopt when a hop isn't an object"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"a"}, serde::Value{std::string{"not an object"}});

        auto found = Document::at(serde::Value{object}, {"a", "b"});

        expect(not found.has_value());
    };

    "at() with no keys returns the root value unchanged"_test = [] {
        serde::Value root{std::string{"root-value"}};

        auto found = Document::at(root, {});

        expect(found.has_value()) << fatal;
        auto leaf = found->to_string();
        expect(leaf.has_value()) << fatal;
        expect(*leaf == "root-value");
    };
};

} // namespace openapi_gen_document_tests
#endif

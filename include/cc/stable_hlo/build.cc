// Build-time code generator: emits ONLY schema/table.cppm — the one piece of cc_stable_hlo
// that actually varies with generator/ops.json's content (the compiled-in
// stable_hlo_op_schema_table() function, one StableHloOpSchema literal per manifest op).
// Everything else in this directory is fixed content that doesn't depend on the manifest, so
// it's a normal hand-written, checked-in file — one class per file, subdirectories (op/,
// schema/, builder/) wherever a concern needed more than one.
//
// Nothing inside the generated schema/table.cppm ever parses JSON or depends on @reflect_cpp
// — this tool reads generator/ops.json once, at build time, and emits the manifest as plain
// C++ initializer syntax. This tool itself is a plain cc_binary (not a C++-modules target):
// this toolchain's deps-scanner can't resolve @reflect_cpp's headers from inside a scanned
// module partition, so build.cc uses only traditional headers, never `import std;`.
//
// Usage: stable_hlo_build <ops.json path> <output dir>
// Invoked at build time by the stable_hlo_generated genrule in include/cc/stable_hlo/BUILD.

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <rfl/json.hpp>

namespace {

// --- JSON manifest structures (must match include/cc/stable_hlo/generator/ops.json) ---

struct JsonParameter {
    std::string name;
    bool variadic{false};
};

struct JsonAttr {
    std::string name;
    std::string cpp_type;
    bool optional{false};
    bool list{false};
};

struct JsonOpDefinition {
    std::string name;
    std::string summary;
    std::string category;
    std::vector<JsonParameter> inputs;
    std::vector<JsonAttr> attrs;
    int output_count{1};
};

// Escapes a string for embedding in a C++ string literal ("..." form).
std::string escape_cpp_string(const std::string &text) {

    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            default: out += c;
        }
    }
    return out;

}

void log_line(std::FILE *stream, std::string_view message) {

    std::fwrite(message.data(), 1, message.size(), stream);
    std::fputc('\n', stream);

}

std::string render_param_schema_literal(const JsonParameter &param) {

    return std::format("{{\"{}\", {}}}", escape_cpp_string(param.name), param.variadic ? "true" : "false");

}

std::string render_attr_schema_literal(const JsonAttr &attr) {

    return std::format("{{\"{}\", \"{}\", {}, {}}}", escape_cpp_string(attr.name), escape_cpp_string(attr.cpp_type),
                       attr.optional ? "true" : "false", attr.list ? "true" : "false");

}

std::string render_op_schema_literal(const JsonOpDefinition &op) {

    std::string inputs = "{";
    for (std::size_t i = 0; i < op.inputs.size(); ++i) {
        if (i > 0) inputs += ", ";
        inputs += render_param_schema_literal(op.inputs[i]);
    }
    inputs += "}";

    std::string attrs = "{";
    for (std::size_t i = 0; i < op.attrs.size(); ++i) {
        if (i > 0) attrs += ", ";
        attrs += render_attr_schema_literal(op.attrs[i]);
    }
    attrs += "}";

    return std::format("        {{\"{}\", \"{}\", \"{}\", {}, {}, {}}},", escape_cpp_string(op.name),
                       escape_cpp_string(op.summary), escape_cpp_string(op.category), inputs, attrs,
                       op.output_count);

}

// The one generated file: the compiled-in stable_hlo_op_schema_table() function, one
// StableHloOpSchema literal per manifest op. Just data — the StableHloOpSchema/
// StableHloParamSchema/StableHloAttrSchema types it uses are hand-written (schema/types.cppm).
std::vector<std::string> render_table_file(const std::vector<JsonOpDefinition> &ops) {

    std::vector<std::string> lines;
    std::size_t start = 0;
    auto push_block = [&lines, &start](const std::string &block) {

        start = 0;
        while (start <= block.size()) {
            std::size_t end = block.find('\n', start);
            if (end == std::string::npos) {
                lines.push_back(block.substr(start));
                break;
            }
            lines.push_back(block.substr(start, end - start));
            start = end + 1;
        }

    };

    push_block(R"GENCODE(// GENERATED FILE — produced at build time by include/cc/stable_hlo/build.cc, do not hand-edit.
// One compiled-in table entry per include/cc/stable_hlo/generator/ops.json op — no runtime
// JSON parsing anywhere in this module; build.cc read the manifest once, at build time.

module;

export module cc_stable_hlo:table;

import std;
import :types;

export namespace cc::stable_hlo {

const std::vector<StableHloOpSchema> &stable_hlo_op_schema_table() {

    static const std::vector<StableHloOpSchema> table = {
)GENCODE");

    for (const auto &op : ops) {
        lines.push_back(render_op_schema_literal(op));
    }

    push_block(R"GENCODE(    };
    return table;

}

} // namespace cc::stable_hlo
)GENCODE");
    return lines;

}

} // namespace

int main(int argument_count, char **arguments) {

    if (argument_count != 3) {
        log_line(stderr, "usage: stable_hlo_build <ops.json path> <output dir>");
        return 1;
    }
    std::string source_path = arguments[1];
    std::string output_dir = arguments[2];

    std::ifstream file{source_path};
    if (!file) {
        log_line(stderr, std::format("stable_hlo_build: could not open '{}'", source_path));
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();

    auto parsed = rfl::json::read<std::vector<JsonOpDefinition>>(buffer.str());
    if (!parsed) {
        log_line(stderr, std::format("stable_hlo_build: failed to parse manifest: {}", parsed.error().what()));
        return 1;
    }
    const std::vector<JsonOpDefinition> &ops = *parsed;

    std::filesystem::path full_path = std::filesystem::path(output_dir) / "schema" / "table.cppm";
    std::error_code ec;
    std::filesystem::create_directories(full_path.parent_path(), ec);
    std::ofstream out(full_path);
    if (!out) {
        log_line(stderr, std::format("stable_hlo_build: failed to open '{}'", full_path.string()));
        return 1;
    }
    for (const auto &line : render_table_file(ops)) {
        out << line << "\n";
    }

    log_line(stdout, std::format("stable_hlo_build: wrote {} op schema entries into schema/table.cppm", ops.size()));
    return 0;

}

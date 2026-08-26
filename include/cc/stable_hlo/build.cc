// Build-time code generator: emits ONLY table.cppm — the one piece of cc_stable_hlo that
// actually varies with generator/ops.json's content (the compiled-in
// stable_hlo_op_schema_table() function, one Operation entry per manifest op, each built via the same
// append_parameter/append_attr/append_result/finalize path a normal caller uses — see
// op/op.cppm).
// Everything else in this directory is fixed content that doesn't depend on the manifest, so
// it's a normal hand-written, checked-in file — one class per file, op/ subfolder for the IR
// tree, everything else flat at the top level.
//
// Nothing inside the generated table.cppm ever parses JSON or depends on @reflect_cpp
// — this tool reads generator/ops.json once, at build time, and emits the manifest as plain
// C++ statements. This tool itself is a plain cc_binary (not a C++-modules target):
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

// A placeholder Shape for a schema-side input/output slot — build.cc has no real bound tensor
// shape to give it (the schema table describes op KINDs, not bound values). Only rank matters:
// it's the only part of a placeholder Shape anything reads (Parameter::get_type()'s is_list()
// answer) — variadic gets rank 1, everything else is a scalar. Same "placeholder value, only
// one field is real" reasoning as DType's context-free defaults.
std::string render_placeholder_shape(bool variadic) {

    return variadic ? "Shape{{1}, DType::f32()}" : "Shape::scalar(DType::f32())";

}

// Same output_count normalization the old schema view always applied: negative (variadic
// outputs) or zero both mean "exactly one, rank-1-placeholder" result slot for schema purposes;
// otherwise the manifest's count, each a scalar placeholder.
std::size_t normalized_output_count(int output_count) {

    return output_count <= 0 ? 1 : static_cast<std::size_t>(output_count);

}

// One `{ Operation op{...}; op.append_parameter(...); ...; result.push_back(std::move(op)); }`
// statement block — construct + append_parameter per input + (construct an Attribute, real
// cpp_type/optional/list metadata, no append_value: a schema entry's attr is never bound) +
// append_attr per attr + one append_result per `normalized_output_count(op.output_count)`
// placeholder result, exactly the same path a real bound caller drives (build, then Function::
// add_op finalizes it — schema entries stop one step short: built the same way, just never
// handed to a Function). `result_counter` is a generation-time id counter (there's no more
// finalize_explicit taking a runtime next_id callback to hide a `static` counter in) — threaded
// in from render_table_file's loop, incremented once per emitted result across the whole table.
std::string render_op_block(const JsonOpDefinition &op, std::size_t &result_counter) {

    std::string block = std::format(
        "    {{\n        Operation op{{\"{}\", \"{}\", \"{}\"}};\n", escape_cpp_string(op.name),
        escape_cpp_string(op.category), escape_cpp_string(op.summary));

    for (std::size_t i = 0; i < op.inputs.size(); ++i) {
        const JsonParameter &input = op.inputs[i];
        block += std::format(
            "        op.append_parameter(Parameter{{\"{}\", {}, {}}});\n",
            escape_cpp_string(input.name), render_placeholder_shape(input.variadic), i);
    }

    for (const JsonAttr &attr : op.attrs) {
        // Own nested block per attr (not a single shared local) so each can declare its own
        // `attribute` without colliding with the others in this same Operation's statement block.
        // No append_value() call — a schema entry's attr is never bound, Attribute::m_value
        // defaults to nullopt already.
        block += std::format(
            "        {{\n"
            "            Attribute attribute{{\"{}\", \"{}\", {}, {}}};\n"
            "            op.append_attr(attribute);\n"
            "        }}\n",
            escape_cpp_string(attr.name), escape_cpp_string(attr.cpp_type),
            attr.optional ? "true" : "false", attr.list ? "true" : "false");
    }

    bool variadic_output = op.output_count < 0;
    std::size_t output_count = normalized_output_count(op.output_count);
    for (std::size_t i = 0; i < output_count; ++i) {
        block += std::format(
            "        op.append_result(Parameter{{\"%schema{}\", {}}});\n", result_counter++,
            render_placeholder_shape(variadic_output));
    }

    block += "        result.push_back(std::move(op));\n    }\n";

    return block;

}

// The one generated file: the compiled-in stable_hlo_op_schema_table() function, one Operation per
// manifest op, each built via the append_parameter/append_attr/append_result path (statement
// blocks, not literal expressions — Operation has no aggregate-friendly shape once its attrs carry
// real type metadata alongside optional bound values). The types it constructs (Parameter,
// Attribute, Operation) are hand-written (base/parameter.cppm, base/attribute.cppm, skipper/operation.cppm).
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
import :dtype;
import :shape;
import :operation;
import :parameter_view;
import :schema_attribute_view;

export namespace cc::stable_hlo {

const std::vector<Operation> &stable_hlo_op_schema_table() {

)GENCODE");

    push_block(std::format(
        "    static const std::vector<Operation> table = [] {{\n"
        "        std::vector<Operation> result;\n"
        "        result.reserve({});\n",
        ops.size()));

    std::size_t result_counter = 0;
    for (const auto &op : ops) {
        push_block(render_op_block(op, result_counter));
    }

    push_block(R"GENCODE(        return result;
    }();
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

    std::filesystem::path full_path = std::filesystem::path(output_dir) / "table.cppm";
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

    log_line(stdout, std::format("stable_hlo_build: wrote {} op schema entries into table.cppm", ops.size()));
    return 0;

}

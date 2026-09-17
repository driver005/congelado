module;

#include <cstdio>
#include <string>

export module cc_abi_gen_generator:runtime;

import std;
import cc_abi_gen_parser;
import :emitter;
import cc_abi_gen_writer;

export namespace cc_abi_gen::generator::runtime {


class Runtime
{
public:
    Runtime() = default;

    Runtime(std::filesystem::path&& path) :
        m_repo_root{std::move(path)}
    {
    }

    ~Runtime() = default;
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = default;
    Runtime& operator=(Runtime&&) = default;

    Runtime& add_registry(parser::Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
        return *this;
    }

    Runtime& add_parser(parser::Parser&& parser) noexcept
    {
        m_parser = std::move(parser);
        return *this;
    }

    Runtime& add_emitter(emitter::Emitter&& emitter) noexcept
    {
        m_emitter = std::move(emitter);
        return *this;
    }

    Runtime& add_repo_root(std::filesystem::path&& repo_root) noexcept
    {
        m_repo_root = std::move(repo_root);
        return *this;
    }

    Runtime& add_output_dir(std::string&& out_dir) noexcept
    {
        m_output_dir = std::move(out_dir);
        return *this;
    }

    std::expected<void, std::string> parse_file(const std::filesystem::path& header_path)
    {
        if (m_repo_root.empty()) {
            return std::unexpected{"repo_root not set"};
        }
        return m_parser.parse_file(header_path, m_repo_root);
    }

    std::expected<void, std::string> parse_directory(const std::filesystem::path& root_path)
    {
        if (m_repo_root.empty()) {
            return std::unexpected{"repo_root not set"};
        }

        std::array<std::string_view, 2> extensions = {".h", ".hpp"};

        return m_parser.parse_directory(m_repo_root, root_path.string(), extensions);
    }

    std::expected<void, std::string>
    generate_single(const parser::vtable::Model& model, emitter::Mode mode)
    {
        return m_emitter.generate(m_repo_root, model, mode);
    }

    std::expected<void, std::string> generate()
    {
        const auto& registry = m_parser.get_registry();
        if (registry.begin() == registry.end()) {
            return std::unexpected{"no models parsed"};
        }

        for (const auto& [struct_name, model]: registry) {
            auto result = m_emitter.generate(m_repo_root, model, emitter::Mode::Both);
            if (!result) {
                return std::unexpected{std::move(result.error())};
            }
        }
        return {};
    }

    std::expected<bool, std::string>
    check_single(const parser::vtable::Model& model, emitter::Mode mode)
    {
        auto rendered = m_emitter.check(m_repo_root, model, mode);
        if (!rendered) {
            return std::unexpected{std::move(rendered.error())};
        }

        return *rendered;
    }

    std::expected<bool, std::string> check()
    {
        const auto& registry = m_parser.get_registry();
        if (registry.begin() == registry.end()) {
            return std::unexpected{"no models parsed"};
        }

        for (const auto& [struct_name, model]: registry) {
            for (auto mode: {emitter::Mode::Builder, emitter::Mode::Sonic}) {
                auto rendered = m_emitter.check(m_repo_root, model, mode);
                if (!rendered) {
                    return std::unexpected{std::move(rendered.error())};
                }

                if (!*rendered) {
                    return false;
                }
            }
        }

        return true;
    }

    void set_registry(parser::Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
    }

    void set_parser(parser::Parser&& parser) noexcept
    {
        m_parser = std::move(parser);
    }

    void set_emitter(emitter::Emitter&& emitter) noexcept
    {
        m_emitter = std::move(emitter);
    }

    void set_repo_root(std::filesystem::path&& repo_root) noexcept
    {
        m_repo_root = std::move(repo_root);
    }

    void set_output_dir(std::string&& out_dir) noexcept
    {
        m_output_dir = std::move(out_dir);
    }

    const parser::Registry& get_registry() const noexcept
    {
        return m_registry;
    }

    const parser::Parser& get_parser() const noexcept
    {
        return m_parser;
    }

    const emitter::Emitter& get_emitter() const noexcept
    {
        return m_emitter;
    }

    const std::filesystem::path& get_repo_root() const noexcept
    {
        return m_repo_root;
    }

    std::string_view get_output_dir() const noexcept
    {
        return m_output_dir;
    }


private:
    parser::Registry m_registry;
    parser::Parser m_parser{"clang++", m_registry};
    emitter::Emitter m_emitter{m_registry, "ice"};
    std::filesystem::path m_repo_root{};
    std::string m_output_dir;
};

} // namespace cc_abi_gen::generator::runtime

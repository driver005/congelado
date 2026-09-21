export module cc_abi_gen_generator:helper_module_naming;

import std;

export namespace cc_abi_gen::helper {

class ModuleNaming
{
public:
    // include/cc/ice/extern/filesystem/sonic -> cc_ice_extern_filesystem_sonic
    static std::expected<std::string, std::string>
    module_name(const std::filesystem::path& directory)
    {

        return join(directory, "_", false);

    }

    // include/cc/ice/extern/filesystem/sonic -> //include/cc/ice/extern/filesystem/sonic
    static std::expected<std::string, std::string>
    bazel_package(const std::filesystem::path& directory)
    {

        auto package = join(directory, "/", true);
        if (!package) {
            return package;
        }

        return "//" + *package;

    }

    // Full label of a directory's module target.
    static std::expected<std::string, std::string>
    bazel_label(const std::filesystem::path& directory)
    {

        auto package = bazel_package(directory);
        if (!package) {
            return package;
        }

        auto name = module_name(directory);
        if (!name) {
            return name;
        }

        return std::format("{}:{}", *package, *name);

    }

private:
    static std::expected<std::string, std::string>
    join(const std::filesystem::path& directory, std::string_view separator, bool keep_marker)
    {

        auto marker = std::ranges::find_if(
            directory,
            [](const std::filesystem::path& part) { return part.string() == MARKER; }
        );
        if (marker == directory.end()) {
            return std::unexpected(
                std::format("path has no `{}` component: {}", MARKER, directory.string())
            );
        }

        if (!keep_marker) {
            ++marker;
        }

        std::string joined;
        for (const auto& part: std::ranges::subrange(marker, directory.end())) {
            if (part.empty()) {
                continue;
            }

            if (!joined.empty()) {
                joined += separator;
            }
            joined += part.string();
        }

        return joined;

    }

    static constexpr std::string_view MARKER = "include";
};

} // namespace cc_abi_gen::helper

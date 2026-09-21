export module cc_abi_gen_generator:helper_types;

import std;

export namespace cc_abi_gen::generator::emitter {

enum class Mode
{
    Builder,
    Sonic,
    Both
};

constexpr inline std::optional<Mode> from_string(std::string_view str) noexcept
{
    if (str == "builder") {
        return Mode::Builder;
    }
    if (str == "sonic") {
        return Mode::Sonic;
    }
    if (str == "both") {
        return Mode::Both;
    }
    return std::nullopt;
}

using PathCallback = std::function<void(std::filesystem::path& model)>;

struct PathComponents
{
    std::string tier;
    std::string domain;
    std::string mode;
    std::string file;
};


} // namespace cc_abi_gen::generator::emitter

template<>
struct std::formatter<cc_abi_gen::generator::emitter::Mode> : std::formatter<std::string_view>
{
    auto format(cc_abi_gen::generator::emitter::Mode mode, std::format_context& ctx) const
    {
        // C++20: brings the enum values into the local scope
        using enum cc_abi_gen::generator::emitter::Mode;

        std::string_view name;
        switch (mode) {
            case Builder:
                name = "Builder";
                break;
            case Sonic:
                name = "Sonic";
                break;
            case Both:
                name = "Both";
                break;
            default:
                name = "Unknown";
                break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};

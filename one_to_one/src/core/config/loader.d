module core.config.loader;
@nogc nothrow:

import core.config.types : Config, PluginConfig;
import util.result : Result;

// PORT-NOTE: TOML and JSON parsing (toml++, simdjson) are C++ library dependencies
// with no direct @nogc D equivalents.  The parse_toml / parse_json helpers are
// retained as stubs that always return an empty Config.  Full parsing can be wired
// in the Run-3 improvement pass when a D TOML/JSON library is integrated.
//
// PORT-NOTE: std::filesystem::path → const(char)[] (path string slice)
// PORT-NOTE: std::expected<Config, std::string> → Result!(Config, const(char)[])

private Result!(Config, const(char)[]) parse_toml(const(char)[] path) {
    // TODO: wire toml++ or a D TOML library
    import util.result : ok;
    return ok!(Config, const(char)[])(new Config());
}

private Result!(Config, const(char)[]) parse_json(const(char)[] path) {
    // TODO: wire simdjson or a D JSON library
    import util.result : ok;
    return ok!(Config, const(char)[])(new Config());
}

// Public entry point — mirrors core::config::load().
Result!(Config, const(char)[]) load(const(char)[] path) {
    if (path.length == 0) {
        import util.result : ok;
        return ok!(Config, const(char)[])(new Config());
    }

    // Detect extension by scanning from the end
    size_t dot_pos = path.length;
    for (size_t i = path.length; i > 0; --i) {
        if (path[i - 1] == '.') { dot_pos = i - 1; break; }
    }

    const(char)[] ext = dot_pos < path.length ? path[dot_pos .. $] : "";

    if (ext == ".toml")
        return parse_toml(path);
    if (ext == ".json")
        return parse_json(path);

    import util.result : err;
    return err!(Config, const(char)[])(
        "unknown config extension: use .toml or .json");
}

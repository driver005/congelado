module core.config.types;
@nogc nothrow:

import util.hashmap.swiss : SwissHashMap;

// Config for one plugin section. All TOML keys are stored verbatim in m_fields
// and forwarded to the plugin's on_load() via CongeladoConfigView.
// The plugin itself owns the interpretation of every key — no host-side schema.
//
// PORT-NOTE: std::unordered_map<std::string, std::string> → SwissHashMap!(string, string)
//   with @nogc semantics.  std::string members → const(char)[] (borrowed views) for
//   name/type, and a SwissHashMap for fields.  Full heap strings would require a
//   separate owning array; for the one-to-one pass we store const(char)[] slices.
class PluginConfig {
  public:
    this() {}

    void set_name(const(char)[] name) { m_name = name; }
    void set_type(const(char)[] type) { m_type = type; }
    void add_field(const(char)[] key, const(char)[] value) { m_fields[key] = value; }

    const(char)[] get_name() const { return m_name; }
    const(char)[] get_type() const { return m_type; }
    // PORT-NOTE: returns by ref in C++; here we return a const ref to the internal map
    ref const(SwissHashMap!(const(char)[], const(char)[])) get_fields() const { return m_fields; }
    ref SwissHashMap!(const(char)[], const(char)[]) get_fields_mut() { return m_fields; }

  private:
    const(char)[] m_name;
    const(char)[] m_type;
    SwissHashMap!(const(char)[], const(char)[]) m_fields;
}

// Top-level config. Only [plugins.*] sections exist — logger is just another plugin.
class Config {
  public:
    this() {}

    void add_plugin(PluginConfig plugin) { m_plugins[plugin.get_name()] = plugin; }

    ref const(SwissHashMap!(const(char)[], PluginConfig)) get_plugins() const { return m_plugins; }
    ref SwissHashMap!(const(char)[], PluginConfig) get_plugins_mut() { return m_plugins; }

  private:
    SwissHashMap!(const(char)[], PluginConfig) m_plugins;
}

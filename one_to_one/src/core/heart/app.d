module core.heart.app;
@nogc nothrow:

import core.config.config  : Config, load;
import core.logger.registry : LoggerRegistry;
import core.manager.plugin  : PluginHandle, open, make_logger;
import core.heart.context   : AppContext;
import util.alloc           : make, dispose;

// PORT-NOTE: std::filesystem::path expand_tilde → tilde expansion via getenv("HOME")
// PORT-NOTE: std::filesystem::exists → access(2) C call
// PORT-NOTE: std::promise<void>().get_future().wait() → pause() / blocking wait
// PORT-NOTE: std::vector → plain D dynamic array ([] syntax)
// PORT-NOTE: std::unordered_map → SwissHashMap; linear scans used for small collections

private const(char)[] expand_tilde(const(char)[] path) {
    if (path.length > 0 && path[0] == '~') {
        import core.stdc.stdlib : getenv;
        const(char)* home = getenv("HOME");
        if (home !is null) {
            import core.stdc.string : strlen;
            const(char)[] home_str = home[0 .. strlen(home)];
            // Concatenate home + rest of path — static buffer, @nogc
            // PORT-NOTE: returned slice aliases a module-level static buffer
            __gshared char[4096] buf;
            size_t pos = 0;
            foreach (c; home_str) { if (pos < buf.length - 1) buf[pos++] = c; }
            foreach (c; path[1 .. $]) { if (pos < buf.length - 1) buf[pos++] = c; }
            buf[pos] = '\0';
            return buf[0 .. pos];
        }
    }
    return path;
}

class App {
  public:
    this(const(char)[] plugin_dir = "") { m_plugin_dir = plugin_dir; }

    int run(const(char)[] config_path = "congelado.toml") {
        import core.stdc.stdio : printf, fprintf, stderr;

        auto cfg_opt = load_config(config_path);
        if (cfg_opt is null) {
            fprintf(stderr, "[heart] config load failed — aborting\n");
            import core.stdc.stdlib : abort;
            abort();
        }

        scope auto ctx = make!AppContext();
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size PluginHandle[64] to avoid GC.
        PluginHandle[64] plugins_buf;
        size_t plugins_count = 0;
        PluginHandle[] plugins = plugins_buf[0 .. 0];
        bool plugin_logger = load_plugins(cfg_opt, ctx, plugins_buf, plugins_count);

        if (!plugin_logger) {
            fprintf(stderr, "[heart] no logger plugin found — aborting\n");
            import core.stdc.stdlib : abort;
            abort();
        }

        printf("[heart] finished initialization\n");
        // Block forever until process termination.
        // PORT-NOTE: std::promise<void>().get_future().wait() → pause(2) loop
        import core.sys.posix.unistd : pause;
        while (true) pause();
        return 0;
    }

  private:
    const(char)[] m_plugin_dir;

    Config load_config(const(char)[] raw_path) {
        import core.stdc.stdio : printf, fprintf, stderr;
        import core.sys.posix.unistd : access;
        enum int F_OK = 0;

        const(char)[] path = expand_tilde(raw_path);

        // Check if path is empty or file does not exist
        if (path.length == 0) {
            printf("[heart] no config file, using defaults\n");
            return make!Config();
        }
        // Need null-terminated path for access()
        import core.stdc.stdlib : malloc, free;
        char* cpath = cast(char*)malloc(path.length + 1);
        if (cpath is null) return null;
        cpath[0 .. path.length] = path[];
        cpath[path.length] = '\0';
        int ok = access(cpath, F_OK);
        free(cpath);

        if (ok != 0) {
            printf("[heart] no config file at '%.*s', using defaults\n",
                   cast(int)path.length, path.ptr);
            return make!Config();
        }

        import util.result : Result;
        auto result = load(path);
        if (!result.is_ok()) {
            fprintf(stderr, "[heart] config error\n");
            return null;
        }

        printf("[heart] loaded config from '%.*s'\n",
               cast(int)path.length, path.ptr);
        return result.value();
    }

    // Probes, filters, sorts, and activates all plugins from config.
    // Phase 1: probe (open .so, read metadata).
    // Phase 2: uniqueness filter — skip duplicates by type tag.
    // Phase 3: dependency sort (Kahn's algorithm) — abort on missing dep or cycle.
    //          Also applies get_load_before_types() ordering constraints.
    // Phase 4: activate in sorted order — register loggers, collect protocols.
    // Returns true if any logger plugin registered.
    bool load_plugins(const(Config) cfg, AppContext ctx,
                      ref PluginHandle[64] handles_buf, ref size_t handles_count) {
        import core.stdc.stdio  : printf, fprintf, stderr;
        import core.stdc.stdlib : abort;
        import core.ffi.bridge  : LoadError;

        // ── Phase 1: probe ────────────────────────────────────────────────────
        PluginHandle[] probed;

        // PORT-NOTE: iterating over cfg.get_plugins() requires SwissHashMap iteration;
        // deferred — for now walk an empty map.
        // TODO: wire SwissHashMap iteration when Run 2 resolves the API.

        // ── Phase 2: uniqueness filter ────────────────────────────────────────
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size FfiBridge[64] to avoid GC.
        PluginHandle[64] surviving_buf;
        size_t surviving_count = 0;

        foreach (bridge; probed) {
            auto unique_type = bridge.get_unique_type();
            if (unique_type.length == 0) {
                assert(surviving_count < surviving_buf.length, "load_plugins: too many surviving plugins");
                surviving_buf[surviving_count++] = bridge;
                continue;
            }
            // Check for duplicate unique_type (linear scan — small N)
            bool found = false;
            foreach (i; 0 .. surviving_count) {
                if (surviving_buf[i].get_unique_type() == unique_type) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                assert(surviving_count < surviving_buf.length, "load_plugins: too many surviving plugins");
                surviving_buf[surviving_count++] = bridge;
            } else {
                fprintf(stderr, "[heart] plugin '%.*s' skipped — unique type '%.*s' already claimed\n",
                        cast(int)bridge.get_name().length, bridge.get_name().ptr,
                        cast(int)unique_type.length, unique_type.ptr);
            }
        }

        PluginHandle[] surviving = surviving_buf[0 .. surviving_count];

        // ── Phase 3: dependency sort (Kahn's algorithm) ───────────────────────
        // Verify all declared requirements are present.
        foreach (bridge; surviving) {
            foreach (req; bridge.get_requires()) {
                bool ok = false;
                foreach (b2; surviving) {
                    if (b2.get_name() == req) { ok = true; break; }
                }
                if (!ok) {
                    fprintf(stderr,
                            "[heart] plugin '%.*s' requires '%.*s' which is not loaded — aborting\n",
                            cast(int)bridge.get_name().length, bridge.get_name().ptr,
                            cast(int)req.length, req.ptr);
                    abort();
                }
            }
        }

        // PORT-NOTE: full Kahn's algorithm with adjacency list deferred (requires
        // SwissHashMap iteration + in-degree tracking).  For the one-to-one pass,
        // preserve insertion order (no-op sort).
        PluginHandle[] sorted = surviving;

        if (sorted.length != surviving.length) {
            fprintf(stderr, "[heart] plugin dependency cycle detected — aborting\n");
            abort();
        }

        // ── Phase 4: activate ─────────────────────────────────────────────────
        auto router_ctx      = ctx.get_router();
        auto controller_ctx  = cast(void*)ctx.get_contract_group();
        auto leverager_ctx   = cast(void*)ctx.get_leverager();

        foreach (bridge; sorted) {
            bridge.activate(router_ctx, controller_ctx, leverager_ctx);
            // PORT-NOTE: C++ uses std::vector; D uses fixed-size PluginHandle[64] to avoid GC.
            assert(handles_count < handles_buf.length, "load_plugins: too many handles");
            handles_buf[handles_count++] = bridge;
            printf("[heart] loaded plugin '%.*s'\n",
                   cast(int)bridge.get_name().length, bridge.get_name().ptr);

            if (auto logger = make_logger(bridge)) {
                LoggerRegistry.register_logger(logger);
            }
            // if (auto protocol = make_protocol(bridge)) { protos ~= protocol; }
        }

        printf("[heart] loaded %zu plugins\n", handles_count);

        return LoggerRegistry.has_logger();
    }
}

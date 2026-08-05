-- Orchestrator: spawns the merged "plugins" project (plugins/xmake.lua — json, engine, http2,
-- file_logger, toml, postgres, sql_postgres, python_bridge, lua_bridge, otel_otlp), then builds
-- the root project (congelado/congelado_worker/congelado_cli) — the latter is what actually needs
-- engine's generated client SDK to already exist on disk. The CONGELADO_TASK worker bundles
-- (plugins/engine/worker/internal/<name>/) are engine's own worker subsystem, not separate
-- projects — engine's own xmake.lua discovers and builds them directly as part of its own
-- includes()'d child script (see that file), so they don't need an entry here at all.
--
-- engine shares the merged "plugins" project despite its build.cc codegen step — safe because
-- the constraint that used to force it into its own process was specifically "the ROOT project
-- must not scan for an import of engine's generated output before build.cc has written it";
-- none of the other plugins here import anything engine generates, so their scan doesn't care.
-- xmake's C++ module dependency scan is one global, upfront pass across every target in a SINGLE
-- invocation, before any target's build actions run — so a target's build.cc (a build-time code
-- generator) can never finish writing a new module in time for a *different* target's own scan
-- to resolve an import from it, no matter what hook is used (confirmed this session, including
-- an isolated minimal repro). The root project stays its own separate `xmake` invocation, built
-- last, after this whole merged project (including engine) has fully finished.
--
-- Order: the merged "plugins" project before the root project's own build (congelado_worker
-- imports engine's generated output, which is part of this project). plugin_order is a
-- single-entry table (not just a plain variable) so another standalone plugin project can be
-- added back here later without restructuring this loop.
task("build-all")
on_run(function()
	import("core.base.option")

	-- Explicit mode, passed to every "f -c" config call below (plugins project + root project)
	-- instead of letting each project fall back to its own default: the root project hardcodes
	-- set_defaultmode("debug") (xmake.lua) while the merged plugins project has no explicit
	-- default and silently falls back to xmake's own built-in "release" — meaning a bare
	-- "xmake build-all" used to build the same logical build in two different modes at once with
	-- no way to override either half. Defaulting this option to "debug" keeps that historical
	-- root-project behavior for a bare local "xmake build-all" (no flags) while making both
	-- halves agree, and "xmake build-all --mode=release" (or docker/Dockerfile.builder's
	-- BUILD_MODE build arg, via `make build MODE=release`) now actually controls both.
	local mode = option.get("mode")

	local xmake_bin = os.programfile()
	local root_dir = os.projectdir()

	local plugin_order = {
		-- Merged project (plugins/xmake.lua): json, engine, http2, file_logger, toml, postgres,
		-- sql_postgres, python_bridge, lua_bridge, otel_otlp all share one project now —
		-- congelado_include/congelado_sdk only need to be declared (and compiled) once for all
		-- ten, instead of once per plugin.
		"plugins",
	}

	for _, rel_dir in ipairs(plugin_order) do
		local plugin_dir = path.join(root_dir, rel_dir)
		cprint("${color.build.target}[build-all]:${clear} building standalone plugin project %s", rel_dir)
		-- curdir, not just -P: without it, xmake's own workdir (build/ output, .xmake config)
		-- follows this process's actual cwd (the root project dir, since that's where `xmake
		-- build-all` itself was invoked from) rather than the "-P" project dir — harmless
		-- (no target-name collisions today) but leaves each plugin's build output sitting in
		-- the root project's own build/ instead of its own, which won't stay harmless once more
		-- plugins move out.
		-- Two separate invocations (config, then build), not one combined `xmake -y -P dir` —
		-- confirmed via repeated local testing that the combined form is genuinely flaky for a
		-- project's first-ever (never-configured) run: package installation (conan add_requires
		-- resolution) and the module dependency scan can race when both are triggered by a
		-- single command, intermittently failing with "<some conan package>'s header not found"
		-- (a different package each time) — splitting config and build into two commands, the
		-- same way the root project's own two Dockerfile RUN steps (`make install` then
		-- `make build`) already do, was reliable every time.
		-- Only the "plugins" project (first in plugin_order) is EVER a genuinely fresh-cache case
		-- — checked here, before the config step below, not by unconditionally special-casing
		-- plugin_order[1] forever: `xmake f -c -y` on the next line can itself touch/create parts
		-- of build/.gens/, so this freshness signal has to be captured before that call runs, not
		-- after. os.isdir(...) here reflects reality (was congelado_include's own generated-output
		-- tree ever created in this build/ dir by ANY prior build — a warm local checkout, or a
		-- Docker/Podman build/ cache restored by docker/Dockerfile.builder) rather than assuming
		-- "first plugin processed in this run" always means "first time this machine has ever
		-- built it".
		local congelado_include_gens_dir = path.join(root_dir, "build", ".gens", "congelado_include")
		local first_time_build = (rel_dir == plugin_order[1]) and not os.isdir(congelado_include_gens_dir)

		os.execv(xmake_bin, { "f", "-c", "-y", "-m", mode, "-P", plugin_dir }, { curdir = plugin_dir })
		-- "plugins" (first in plugin_order) is whichever project builds congelado_include/
		-- congelado_sdk from scratch on a genuinely fresh build/ dir — the only time their std
		-- module BMI cache dir (build/.gens/congelado_include/.../rules/bmi/cache/interfaces/) gets
		-- CREATED rather than reused. Under default -j(nproc) parallelism, many congelado_include
		-- .cppm files race to build/consume that not-yet-existing dir concurrently; one job's
		-- "output missing, recompile" cleanup can remove the dir mid-write for a sibling job,
		-- producing "unable to open output file .../std.pcm: No such file or directory". -j1 for
		-- just this one first-time build removes the race; any later "build-all" run (or another
		-- standalone plugin project added to plugin_order in the future) reuses the now-populated
		-- cache and keeps full parallelism.
		--
		-- first_time_build (computed above, before the config step) gates this correctly instead
		-- of applying unconditionally on every "build-all" run forever: once build/ has been built
		-- at least once — locally, or via docker/Dockerfile.builder's build/ cache-mount restore —
		-- the race is impossible and this would otherwise serialize the biggest compile unit in the
		-- whole project (congelado_include/congelado_sdk, see xmake/core_layers.lua) for no reason,
		-- on every single container build.
		local build_args = { "-P", plugin_dir }
		if first_time_build then
			table.insert(build_args, "-j1")
		end
		os.execv(xmake_bin, build_args, { curdir = plugin_dir })
	end

	cprint("${color.build.target}[build-all]:${clear} building root project")
	os.execv(xmake_bin, { "f", "-c", "-y", "-m", mode, "-P", root_dir })
	os.execv(xmake_bin, { "-P", root_dir })
end)

set_menu({
	usage = "xmake build-all [options]",
	description = "Build core + every standalone plugin project (in dependency order), then the root project's remaining targets — the single-invocation entry point (replaces the old 'run xmake build twice by hand' workaround).",
	options = {
		{"m", "mode", "kv", "debug", "Build mode for every project (plugins + root): debug or release."},
	},
})
task_end()

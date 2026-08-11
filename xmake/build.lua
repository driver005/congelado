-- Generic xmake "build-time generator" idiom: declare_build_tool() opens a set_default(false)
-- tool target (excluded from the normal build set) for a tool that must be force-built and run
-- before its dependent target compiles (e.g. build.cc). Actually running it belongs in
-- xmake/modules/build_tool.lua instead, as a real importable module — before_build()'s own
-- sandboxed script scope can't see plain functions defined here at description-parse time.
--
-- Real usage: plugins/engine/xmake.lua's "engine"/"engine_build" targets, for build.cc.

-- Opens (declares) the tool target: binary, given source file(s), excluded from the default
-- build set. Caller adds anything else it needs (add_deps, set_policy, set_languages, ...) and
-- its own target_end().
function declare_build_tool(name, files)
	target(name)
	set_kind("binary")
	add_files(files)
	set_default(false)
end

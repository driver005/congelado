import("core.project.config")
import("core.project.project")

-- name must be a real add_deps() dependency of the calling target (see plugins/engine/xmake.lua's
-- engine_lib) — xmake's own scheduler then guarantees it's already built, once, before this
-- before_build hook runs. This used to shell out to a second `xmake build -y <name>` subprocess
-- here instead, which raced the outer process's own scheduled build of the same target: two
-- clang invocations writing the same partition .pcm output path at once, corrupting the BMI.
--
-- project.lock()/unlock() around running the generator: it rewrites checked-in generated/**.cppm
-- sources in the project tree, and another concurrently-scheduled target could be mid dep-scan
-- over those same files. s_active makes it reentrant across the two sequential calls in
-- engine_lib's before_build (engine_api then worker_api) without double-locking.
local s_active = 0

function main(target, name, srcdir)
	-- Skip codegen on `xmake run`. run autobuilds via a nested task.run("build") (see xmake's
	-- actions/run/main.lua), which still fires this before_build hook — but xmake.argv()[1] is the
	-- ORIGINAL top-level command, unchanged by that nested task, so it reads "run" here while a
	-- real build reads "build" (or nil for a bare `xmake`). The generated/**.cppm sources are
	-- checked in and only need regenerating on an explicit build, not on every run.
	if xmake.argv()[1] == "run" then
		cprint("${blue}[build_tool] skipping %s codegen (xmake run)${clear}", name)
		return
	end

	srcdir = srcdir or "src"
	local buildir = config.builddir({ absolute = true })
	local toolfile = path.join(buildir, target:plat(), target:arch(), config.mode(), name)
	cprint("${blue}[build_tool] running %s${clear}", toolfile)
	if s_active == 0 then
		project.lock()
	end
	s_active = s_active + 1
	os.execv(toolfile, {}, { curdir = path.join(target:scriptdir(), srcdir) })
	s_active = s_active - 1
	if s_active == 0 then
		project.unlock()
	end
	cprint("${blue}[build_tool] finish: %s${clear}", name)
end

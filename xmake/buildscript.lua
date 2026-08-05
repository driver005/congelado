-- Generic build-script feature: any "build.cc" found anywhere under a given folder — at any
-- depth, not just directly inside it — gets compiled as its own standalone binary (never
-- merged into the normal binary/lib targets) and wired to run itself automatically — via its
-- own after_build hook — before the folder's actual target compiles. Ordinary add_deps()
-- ordering is what enforces "before": the caller still has to add_deps() every tool name
-- returned here from within the target it protects.
--
-- Convention: each build.cc takes no arguments — its own output paths (and whether they land
-- inside or outside any existing glob) are entirely up to that build.cc; this feature only
-- discovers and runs it, relative to the project root, which is where xmake always runs it
-- from.
function wire_build_scripts(target_name, dir, deps)
	local tool_names = {}
	-- "**/build.cc" alone doesn't match a build.cc sitting directly in dir (xmake's glob
	-- requires at least one subdirectory level for "**/x") — union it with a direct,
	-- depth-0 check so a build.cc right inside the plugin's own folder is never missed.
	local found = {}
	for _, f in ipairs(os.files(path.join(dir, "build.cc"))) do
		found[f] = true
	end
	for _, f in ipairs(os.files(path.join(dir, "**/build.cc"))) do
		found[f] = true
	end
	local build_files = {}
	for f, _ in pairs(found) do
		table.insert(build_files, f)
	end
	table.sort(build_files)

	for _, build_file in ipairs(build_files) do
		-- Unique name per file found: target_name plus the build.cc's own relative sub-path
		-- (slashes/dots collapsed to underscores) so two build.cc files under the same dir
		-- tree never collide on target name.
		local rel = path.relative(path.directory(build_file), dir)
		local tool_name = target_name
		if rel ~= "." then
			tool_name = tool_name .. "_" .. rel:gsub("[/\\%.]", "_")
		end
		tool_name = tool_name .. "_build"

		target(tool_name)
		set_kind("binary")
		set_languages("c++26")
		set_policy("build.c++.modules", true)
		-- Same cross-target BMI race as congelado_include/congelado_sdk/congelado_lib (see
		-- xmake.lua) — this tool target add_deps() one of `deps` (typically the target whose
		-- folder this build.cc lives in), and needs that target's own modules fully built before
		-- it can compile a `build.cc` that imports them; xmake's default cross-target
		-- parallelism doesn't reliably wait for that on its own.
		set_policy("build.fence", true)
		add_files(build_file)
		for _, dep in ipairs(deps) do
			add_deps(dep)
		end
		add_rpathdirs("$ORIGIN")
		if is_plat("linux", "macosx") then
			add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
		end
		-- Run via a subshell that cd's before exec'ing, rather than os.execv's own `curdir`
		-- option: `curdir` does a process-wide chdir() in the CALLING xmake process — fine in
		-- isolation, but this after_build hook can fire while OTHER compile jobs are running
		-- concurrently in other coroutines during a real parallel build (confirmed empirically:
		-- works every time run standalone/serially, fails every time inside a live -j build,
		-- exact symptom of a shared-process-cwd race). A subshell's `cd` only affects that child
		-- process's own state, so it's safe regardless of what else the parent process is doing.
		-- build.cc conventionally uses paths relative to its own folder (e.g.
		-- "generated/openapi.json", "../../build/plugins/libx.so") — that only resolves
		-- correctly with cwd pinned to *this* folder, which stopped being implicitly true once
		-- this folder's targets could share a project (and therefore a process cwd) with sibling
		-- folders that aren't this one.
		local build_dir = path.directory(build_file)
		after_build(function(target)
			-- absolute, not target:targetfile() as-is: that's relative to the invoking process's
			-- cwd (e.g. relative to plugins/, not plugins/engine/), which would resolve wrong
			-- once the subshell below has already cd'd into build_dir.
			local targetfile = path.absolute(target:targetfile())
			os.execv("/bin/sh", { "-c", "cd \"$1\" && exec \"$2\"", "sh", build_dir, targetfile })
		end)
		target_end()

		table.insert(tool_names, tool_name)
	end
	return tool_names
end

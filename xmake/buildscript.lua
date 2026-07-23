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
		add_files(build_file)
		for _, dep in ipairs(deps) do
			add_deps(dep)
		end
		add_rpathdirs("$ORIGIN")
		if is_plat("linux", "macosx") then
			add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
		end
		after_build(function(target)
			os.execv(target:targetfile())
		end)
		target_end()

		table.insert(tool_names, tool_name)
	end
	return tool_names
end

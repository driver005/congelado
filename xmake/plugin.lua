-- Generic dlopen'd shared-lib target discovery, shared by both plugins (protocol/infra
-- extensions the server host loads) and workers (CONGELADO_TASK FFI plugins the worker
-- daemon loads) — they're both "scan a directory tree of .cc files, make each a dlopen'd
-- shared-lib target," differing only in how files group into targets and where the
-- output lands.
--
-- Plugin mode (one .cc file = one target, named after the file):
--   plugins/engine/engine.cc         -> target "engine"
--   plugins/http2/http2.cc           -> target "http2"
--
-- Worker bundle mode (every .cc under one folder = one target, named after the folder):
--   plugins/engine/worker/internal/echo/echo.cc           -> target "echo"
--   plugins/engine/worker/internal/payments/{a,b}.cc      -> target "payments" (both files)
--
-- Either way, a build.cc found anywhere under that target's own folder is excluded from its
-- sources and instead compiled as its own standalone tool, run automatically before the
-- target's real sources compile (see xmake/buildscript.lua).
local function define_dlopen_target(name, files, opts)
	local build_tools = wire_build_scripts(name, path.directory(files[1]), { "congelado_lib" })

	target(name)
	set_kind("shared")
	set_languages("c++26")
	set_policy("build.c++.modules", true)
	add_files(table.unpack(files))
	add_includedirs("$(projectdir)/include")
	for _, dir in ipairs(opts.extra_includedirs or {}) do
		add_includedirs(dir)
	end
	add_deps("congelado_lib")
	for _, build_tool in ipairs(build_tools) do
		add_deps(build_tool)
	end
	if opts.packages then
		add_packages(table.unpack(opts.packages))
	end
	add_rpathdirs("$ORIGIN")
	set_targetdir(opts.targetdir)
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	if is_plat("windows", "mingw") then
		add_cxflags("--target=x86_64-w64-mingw32")
	end
	target_end()
end

local function discover_dlopen_targets(root_dir, opts)
	if opts.group_by_folder then
		local groups = {}
		for _, f in ipairs(os.files(path.join(root_dir, "**/*.cc"))) do
			if path.basename(f) ~= "build" then
				local name = path.basename(path.directory(f))
				groups[name] = groups[name] or {}
				table.insert(groups[name], f)
			end
		end
		for name, files in pairs(groups) do
			define_dlopen_target(name, files, opts)
		end
	else
		for _, f in ipairs(os.files(path.join(root_dir, "**/*.cc"))) do
			-- exclude_dir keeps this per-file scan of plugins/** out of the worker bundle's
			-- own subtree (plugins/engine/worker/internal/), which gets its own separate
			-- group_by_folder discovery call below — without this, e.g. echo.cc would get
			-- claimed by both calls and double-defined.
			local excluded = opts.exclude_dir and path.directory(f):startswith(opts.exclude_dir)
			if path.basename(f) ~= "build" and not excluded then
				define_dlopen_target(path.basename(f), { f }, opts)
			end
		end
	end
end

-- Plugins: entry points live directly under plugins/<name>/<name>.cc, alongside that
-- plugin's own .cppm implementation and any build.cc it has.
discover_dlopen_targets(path.join(os.projectdir(), "plugins"), {
	targetdir = "$(builddir)/plugins",
	packages = { "sqlgen" },
	exclude_dir = path.join(os.projectdir(), "plugins/engine/worker/internal"),
})

-- Workers: CONGELADO_TASK FFI plugins the worker daemon dlopens, bundled by folder.
discover_dlopen_targets(path.join(os.projectdir(), "plugins/engine/worker/internal"), {
	targetdir = "$(builddir)/workers",
	extra_includedirs = { "$(projectdir)/sdk/worker/include" },
	group_by_folder = true,
})

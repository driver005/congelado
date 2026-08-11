-- Shared setup for the three layered shared libs in xmake.lua (congelado_include/sdk/lib), and
-- for every plugin's own target (plugins/*/xmake.lua): platform link flags and the common package
-- set. Each caller still does its own set_kind/add_deps/add_files/target_end, and (since not
-- every target needs the same include dirs) its own
-- add_includedirs()/add_defines() calls — this only covers what's identical across all of them.
--
-- opts.core_packages is exactly what gets add_packages()'d — nothing implicit. Not every target
-- needs every package in the core_packages list above, so callers pass only what they actually
-- use (e.g. the base list, a subset, or extras like { "hiredis" }); passing nothing adds no
-- packages.
--
-- opts.layer identifies which target this is — every caller passes its own target name (root
-- layers drop the "congelado_" prefix: "include", "sdk"; plugins/workers pass their target name
-- as-is). Only "include" currently changes behavior: it gates uring/pthread/dl (ws2_32/mswsock
-- on Windows). Raw io_uring, pthread and dlopen calls only
-- happen inside congelado_include's own io layer — everything above it (congelado_sdk, plugins,
-- binaries) only ever touches that through congelado_include's module interface, never the
-- syscalls directly, so only the "include" layer needs these on its link line.
-- Some conan packages don't auto-link their .so/.a (or a target linking their headers-only usage
-- still needs the raw syslink) — explicit add_syslinks() per package, gated on that package
-- actually being in opts.core_packages. Ungated, a target without the package still gets the
-- -lXXX flag but never the package's -L, so the linker either fails outright ("cannot find
-- -lpython3.12") or silently falls back to whatever same-named lib is on the system default
-- path (e.g. system libssl instead of conan's QUIC-enabled build) — wrong either way.
local PACKAGE_SYSLINKS = {
	openssl = { "ssl", "crypto" },
	cpython = { "python3.12" },
}

function apply_common_layer_settings(opts)
	opts = opts or {}
	add_cxflags("-fPIC")

	if is_plat("windows", "mingw") then
		add_syslinks("stdc++", "gcc_s")
		add_defines("_WIN32_WINNT=0x0A00")
		add_cxflags("--target=x86_64-w64-mingw32")
		add_ldflags("--target=x86_64-w64-mingw32", "-lstdc++exp")
		if opts.layer == "include" then
			add_syslinks("ws2_32", "mswsock")
		end
	else
		add_links("c++")
		if opts.layer == "include" then
			add_links("uring", "pthread", "dl")
		end
	end

	if opts.core_packages and #opts.core_packages > 0 then
		add_packages(table.unpack(table.join(opts.core_packages, { { public = true } })))
		if not is_plat("windows", "mingw") then
			for _, pkg in ipairs(opts.core_packages) do
				if PACKAGE_SYSLINKS[pkg] then
					add_syslinks(table.unpack(PACKAGE_SYSLINKS[pkg]))
				end
			end
		end
	end

	if opts.targetdir then
		add_rpathdirs("$ORIGIN")
		set_targetdir(opts.targetdir)
	end
	if opts.fence then
		set_policy("build.fence", true)
	end
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
end

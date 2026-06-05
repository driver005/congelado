-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- SDK entry point: import congelado_plugin; + #include <congelado/plugin.h>
-- C++ plugins inherit congelado::Plugin and use CONGELADO_PLUGIN(T).
for _, file in ipairs(os.files(path.join(os.projectdir(), "defaults/plugins/**/*.cc"))) do
	local name = path.basename(file)
	target(name)
	set_kind("shared")
	set_languages("c++26")
	set_policy("build.c++.modules", true)
	add_files(file)
	add_includedirs("$(projectdir)/include")
	add_deps("congelado_lib")
	add_packages("sqlgen")
	add_rpathdirs("$ORIGIN")
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	if is_plat("windows", "mingw") then
		add_cxflags("--target=x86_64-w64-mingw32")
	end
	target_end()
end

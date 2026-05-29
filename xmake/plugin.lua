-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- Each exposes congelado_get_plugin/congelado_destroy_plugin (see include/core/ffi/plugin_api.h).
-- C++ plugins inherit congelado::PluginBase and use CONGELADO_PLUGIN(T) (plugin_api.hpp).
for _, file in ipairs(os.files(path.join(os.projectdir(), "plugins/**/*.cc"))) do
	local name = path.basename(file)
	target(name)
	set_kind("shared")
	set_languages("c++26")
	set_policy("build.c++.modules", true)
	add_files(file)
	add_includedirs("$(projectdir)/include")
	add_deps("congelado_lib")
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	if is_plat("windows", "mingw") then
		add_cxflags("--target=x86_64-w64-mingw32")
	end
	target_end()
end

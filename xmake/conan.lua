-- Shared conan settings table (platform-dependent) — factored out so any project needing to
-- add_requires() a conan package (xmake.lua itself, plugins/xmake.lua's own separate project)
-- doesn't have to duplicate this table by hand.
function build_conan_settings()
	local conan = {
		settings_build = {
			"os=Linux",
			"compiler=clang",
			"compiler.version=20",
			"compiler.cppstd=gnu26",
			"compiler.libcxx=libstdc++11",
		},
		options = {
			"openssl/*:enable_quic=True",
			-- Only the LLVMSupport component (SmallVector/SmallString/StringRef's
			-- out-of-line malloc-based growth helpers live here) — the full "all" default
			-- builds every LLVM tool/target backend, which nothing in this repo needs.
			"llvm-core/*:components=Support",
		},
		build = "missing",
	}

	if is_plat("windows", "mingw") then
		conan.settings = {
			"os=Windows",
			"compiler=clang",
			"compiler.version=20",
			"compiler.cppstd=gnu26",
			"compiler.libcxx=libc++",
		}

		conan.conf = {
			"tools.build:compiler_executables={'c':'/opt/llvm-mingw/bin/clang','cpp':'/opt/llvm-mingw/bin/clang++','rc':'/opt/llvm-mingw/bin/x86_64-w64-mingw32-windres'}",
			"tools.build:cflags=['--target=x86_64-w64-mingw32']",
			"tools.build:cxxflags=['--target=x86_64-w64-mingw32']",
			"tools.build:exelinkflags=['--target=x86_64-w64-mingw32','-fuse-ld=lld']",
			"tools.cmake.cmaketoolchain:system_name=Windows",
			"tools.cmake.cmaketoolchain:generator=Unix Makefiles",
		}
	else
		conan.settings = {
			"os=Linux",
			"compiler=clang",
			"compiler.version=20",
			"compiler.cppstd=gnu26",
			"compiler.libcxx=libstdc++11",
		}

		conan.conf = {
			"tools.build:compiler_executables={'c':'clang','cpp':'clang++'}",
		}
	end
	return conan
end
